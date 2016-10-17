// Copyright (c) 2016 Baidu.com, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// 
// Author: shichengyi@baidu.com (Shi Chengyi)
// Data:2016-10-14 17:08

#ifndef BAIDU_LOGBOOK_CONDITION_H
#define BAIDU_LOGBOOK_CONDITION_H

#include <boost/noncopyable.hpp>
#include <common/mutex.h>

namespace logbook
{
namespace common
{

// TODO make a lockfree condition
class Condition : boost::noncopyable 
{
public:
    explicit Condition(Mutex& mu) : _mu(mu)
    {
        pthread_cond_init(&_cond, NULL);
    }

    ~Condition()
    {
        pthread_cond_destroy(&_cond);
    }

    void wait()
    {
        MutexLockGuard _(_mu);
        pthread_cond_wait(&_cond, _mu.get_pthread_mutex());
    }

    template <class Expression>
    void wait(Expression expr)
    {
        if (expr()) return;
        while (true)
        {
            if (expr())
            {
                break;
            }
            else
            {
                wait();
            }
        }
    }

    void notify()
    {
        pthread_cond_signal(&_cond);
    }

    void notify_all()
    {
        pthread_cond_broadcast(&_cond);
    }

private:
    Mutex& _mu;
    pthread_cond_t _cond;
};

} // end namespace common
} // end namespace logbook

#endif
