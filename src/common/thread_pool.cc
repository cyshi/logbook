// Copyright (c) 2016 Baidu.com, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// 
// Author: shichengyi@baidu.com (Shi Chengyi)
// Data:2016-10-05 23:18

#include <errno.h>
#include <boost/bind.hpp>
#include <common/thread_pool.h>

namespace logbook
{
namespace common
{
namespace detail
{
extern __thread
common::WorkStealingQueue<common::WorkerThread::ThreadFunc>* tls_queue;

static pthread_once_t thread_pool_create_once = PTHREAD_ONCE_INIT;
static common::ThreadPool* g_thread_pool = NULL;


class ThreadExitHelper 
{
public:
    typedef boost::function<void ()> Fn;

    ~ThreadExitHelper() 
    {
        while (!_fns.empty()) 
        {
            Fn back = _fns.back();
            _fns.pop_back();
            back();
        }
    }

    int add(const Fn& fn) 
    {
        if (_fns.capacity() < 16) 
        {
            _fns.reserve(16);
        }
        _fns.push_back(fn);
        return 0;
    }

private:
    std::vector<Fn> _fns;
};

static pthread_key_t thread_atexit_key;
static pthread_once_t thread_atexit_once = PTHREAD_ONCE_INIT;

static void delete_thread_exit_helper(void* arg) 
{
    delete static_cast<ThreadExitHelper*>(arg);
}

static void helper_exit_global() 
{
    detail::ThreadExitHelper* h =
        (detail::ThreadExitHelper*)pthread_getspecific(detail::thread_atexit_key);
    if (h) 
    {
        pthread_setspecific(detail::thread_atexit_key, NULL);
        delete h;
    }
}

static void make_thread_atexit_key() 
{
    pthread_key_create(&thread_atexit_key, delete_thread_exit_helper);
    // If caller is not pthread, delete_thread_exit_helper will not be called.
    // We have to rely on atexit().
    atexit(helper_exit_global);
}

detail::ThreadExitHelper* get_thread_exit_helper() 
{
    pthread_once(&detail::thread_atexit_once, detail::make_thread_atexit_key);
    return (detail::ThreadExitHelper*)pthread_getspecific(detail::thread_atexit_key);
}

detail::ThreadExitHelper* get_or_new_thread_exit_helper() 
{
    detail::ThreadExitHelper* h = get_thread_exit_helper();
    if (NULL == h) 
    {
        h = new (std::nothrow) detail::ThreadExitHelper;
        if (NULL != h) 
        {
            pthread_setspecific(detail::thread_atexit_key, h);
        }
    }
    return h;
}

} // end namespace detail

ThreadPool::ThreadPool()
    : _threads()
    , _started(false)
    , _mu()
    , _queues(NULL)
    , _nqueue(0)
{ 
    // TODO 1024 add to function parameter
    _queues = (WorkerQueue **) calloc (1024, sizeof(WorkerQueue));
}

ThreadPool::~ThreadPool()
{
    stop();
    free(_queues);
}

bool ThreadPool::start(size_t thread_num)
{
    if (_started.exchange(true))
    {
        return false;
    }
    _threads.resize(thread_num);
    for (size_t i = 0; i < thread_num; ++i)
    {
        _threads[i].reset(
            new WorkerThread(boost::bind(&ThreadPool::main_task, this)));
        _threads[i]->start();
    }
    return true;
}

bool ThreadPool::add_task(const WorkerThread::ThreadFunc& func)
{
    if (!_started)
    {
        return false;
    }
    WorkerQueue* q = get_or_create_queue();
    while (!q->push(func))
    {
        usleep(1000);
    }
    return true;
}

ThreadPool::WorkerQueuePtr ThreadPool::get_or_create_queue()
{
    if (detail::tls_queue != NULL)
    {
        return detail::tls_queue;
    }

    detail::tls_queue = new WorkerQueue();
    // TODO add config
    detail::tls_queue->init(1024 * 512);

    // at exit
    detail::ThreadExitHelper* h = detail::get_or_new_thread_exit_helper();
    if (h)
    {
        h->add(boost::bind(&ThreadPool::destroy_queue, this));
    }

    {
        MutexLock l(&_mu);
        const size_t nqueue = _nqueue.load(boost::memory_order_relaxed);
        _queues[nqueue] = detail::tls_queue;
        _nqueue.store(nqueue + 1, boost::memory_order_relaxed);
    }

    return detail::tls_queue;
}

void ThreadPool::stop()
{
    if (!_started.exchange(false))
    {
        return;
    }
    for (size_t i = 0; i < _threads.size(); ++i)
    {
        _threads[i]->join();
    }
}

// run in worker thread
void ThreadPool::main_task()
{
    WorkerQueue* q = get_or_create_queue();
    WorkerThread::ThreadFunc func;
    while (_started)
    {
        if (q->pop(&func))
        {
            func();
            // TODO is this best choice?
            continue;
        }
        size_t nqueue = _nqueue;
        if (nqueue == 0)
        {
            continue;
        }
        // TODO random using xorshift
        // TODO implement a better scheduler
        WorkerQueuePtr victim = _queues[rand() % nqueue];
        if (victim->steal(&func))
        {
            func();
        }
    }
    
    while (q && q->size() != 0)
    {
        if (q->pop(&func))
        {
            func();
        }
    }
}

void ThreadPool::destroy_queue()
{
    while (detail::tls_queue && detail::tls_queue->size() != 0)
    {
        // just sleep a while, wait for stole by other threads
        usleep(1000);
    }

    {
    MutexLock l(&_mu);
    size_t nqueue = _nqueue.load(boost::memory_order_relaxed);
    for (size_t i = 0; i < nqueue; ++i) 
    {
        if (_queues[i] == detail::tls_queue)
        {
            _queues[i] = _queues[nqueue - 1];
            break;
        }
    }
    _nqueue.store(nqueue - 1, boost::memory_order_seq_cst);
    }

    // avoid other threads is stealing
    usleep(1000000L);
    delete detail::tls_queue;
    detail::tls_queue = NULL;
}

ThreadPool* ThreadPool::get_instance()
{
    pthread_once(&detail::thread_pool_create_once,
                 &ThreadPool::create_thread_pool);
    return detail::g_thread_pool;
}

void ThreadPool::destroy_thread_pool()
{
    delete detail::g_thread_pool;
    detail::g_thread_pool = NULL;
}
    
void ThreadPool::create_thread_pool()
{
    detail::g_thread_pool = new ThreadPool();
    ::atexit(&ThreadPool::destroy_thread_pool);
}

} // end namespace common
} // end namespace logbook
