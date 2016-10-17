// Copyright (c) 2016 Baidu.com, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// 
// Author: shichengyi@baidu.com (Shi Chengyi)
// Data:2016-10-17 09:24

#ifndef BAIDU_LOGBOOK_MUTEX_H
#define BAIDU_LOGBOOK_MUTEX_H

#include <pthread.h>
#include <boost/noncopyable.hpp>

namespace logbook
{
namespace common
{

class Mutex : public boost::noncopyable
{
public:
    Mutex() 
    { 
        pthread_mutex_init(&_mu, NULL);
    }

    ~Mutex() 
    { 
        pthread_mutex_destroy(&_mu);
    }

    void lock()
    {  
        pthread_mutex_lock(&_mu);
    }

    void unlock()
    {
        pthread_mutex_unlock(&_mu);
    }

private:
    pthread_mutex_t _mu;
};

template <class T>
class LockGuard : public boost::noncopyable
{
public:
    explicit LockGuard(T& mu) : _mu(mu)
    {
        _mu.lock();
    }

    ~LockGuard()
    {
        _mu.unlock();
    }

private:
    T& _mu;
};

#define MutexLockGuard logbook::common::LockGuard<Mutex>

} // end namespace common
} // end namespace logbook

#endif
