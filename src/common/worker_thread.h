// Copyright (c) 2016 Baidu.com, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// 
// Author: shichengyi@baidu.com (Shi Chengyi)
// Data:2016-10-05 12:54

#ifndef BAIDU_LOGBOOK_WORKER_THREAD_H
#define BAIDU_LOGBOOK_WORKER_THREAD_H

#include <pthread.h>
#include <boost/atomic.hpp>
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>

namespace logbook
{
namespace common
{

class WorkerThread : boost::noncopyable
{
public:
    typedef boost::function<void ()> ThreadFunc;
    explicit WorkerThread(const ThreadFunc& func)
        : _started(false)
        , _joined(false)
        , _tid(0)
        , _thread_func(func)
    { }

    ~WorkerThread();

    bool start();

    bool join();

private:
    boost::atomic<bool> _started;
    boost::atomic<bool> _joined;
    pthread_t _tid;
    ThreadFunc _thread_func;
};

} // end namespace common
} // end namespace logbook
#endif
