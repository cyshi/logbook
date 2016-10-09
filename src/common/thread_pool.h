// Copyright (c) 2016 Baidu.com, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// 
// Author: shichengyi@baidu.com (Shi Chengyi)
// Data:2016-10-04 17:00

#ifndef BAIDU_LOGBOOK_THREAD_POOL_H
#define BAIDU_LOGBOOK_THREAD_POOL_H

#include <vector>
#include <boost/atomic.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <common/mutex.h>
#include <common/work_stealing_queue.h>
#include <common/worker_thread.h>

namespace logbook
{
namespace common
{

class ThreadPool : public boost::noncopyable
{
public:
    /// @brief start 
    /// before add_task must call start first
    /// this function will create threads, and thread runtime env
    ///
    /// @param thread_num
    /// the count of thread you would like managed by thread pool
    ///
    /// @return 
    bool start(size_t thread_num);

    /// @brief stop 
    /// stop thread pool, this operation will join per thread in it
    void stop();

    /// @brief add_task 
    /// add a task to thread pool, maybe block awhile when stealing 
    /// queue is full
    ///
    /// @param task
    ///
    /// @return return false when thread pool not start
    bool add_task(const WorkerThread::ThreadFunc& task);

    /// @brief get_instance 
    ///
    /// @return the instance of thread_pool
    /// thread pool must be a global variable, because we use work stealing
    /// queue per thread, including user thread
    static ThreadPool* get_instance();

private:
    ThreadPool();

    ~ThreadPool();

    static void destroy_thread_pool();
    
    static void create_thread_pool();


    // threads main task
    void main_task();

    typedef common::WorkStealingQueue<WorkerThread::ThreadFunc> WorkerQueue;
    typedef WorkerQueue* WorkerQueuePtr;

    // create work stealing queue which belongs to thread
    // one work stealing queue to one thread
    // and this function will add the queue to _queues
    WorkerQueuePtr get_or_create_queue();

    // destroy work stealing queue
    // and remove the queue from _queues
    void destroy_queue();

private:
    std::vector<boost::shared_ptr<WorkerThread> > _threads;
    boost::atomic<bool> _started;

    // protect _queue
    Mutex _mu;

    // manage work stealing queue
    // the queue added to _queues, then it can be stolen
    WorkerQueuePtr* _queues;
    // the size of _queues
    boost::atomic<size_t> _nqueue;
};

} // end namespace common
} // end namespace logbook

#endif
