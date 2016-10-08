// Copyright (c) 2016 Baidu.com, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// 
// Author: shichengyi@baidu.com (Shi Chengyi)
// Data:2016-10-05 17:52

#include <common/worker_thread.h>
#include <common/work_stealing_queue.h>

namespace logbook
{
namespace common
{
namespace detail
{

__thread 
common::WorkStealingQueue<common::WorkerThread::ThreadFunc>* tls_queue = NULL;

struct ThreadData
{
    ThreadData(const common::WorkerThread::ThreadFunc& func, 
               pthread_t* tid) 
        : _func(func)
        , _tid(tid)
    { }
    common::WorkerThread::ThreadFunc _func;
    pthread_t* _tid;

};

void* run_in_thread(void* args)
{
    ThreadData* data = static_cast<ThreadData *>(args);
    data->_func();
    delete data;
    return NULL;
}

} // end namespace detail

WorkerThread::~WorkerThread()
{
    if (_started && !_joined)
    {
        pthread_detach(_tid);
    }
}

bool WorkerThread::start()
{
    if (_started)
    {
        return false;
    }

    detail::ThreadData* data = 
        new detail::ThreadData(_thread_func, &_tid);
    if (pthread_create(&_tid, NULL, 
                       &detail::run_in_thread, (void *)data) != 0)
    {
        delete data;
        return false;
    }
    _started = true;
    return true;
}

bool WorkerThread::join()
{
    if (!_started || _joined)
    {
        return false;
    }
    _joined = true;
    return (pthread_join(_tid, NULL) == 0);
}

} // end namespace common
} // end namespace logbook
