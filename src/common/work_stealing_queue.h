// Copyright (c) 2016 Baidu.com, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// 
// Author: shichengyi@baidu.com (Shi Chengyi)
// Data:2016-10-04 23:48

#ifndef BAIDU_LOGBOOK_WORK_STEALING_QUEUE_H
#define BAIDU_LOGBOOK_WORK_STEALING_QUEUE_H

#include <new>
#include <boost/atomic.hpp>
#include <boost/noncopyable.hpp>

namespace logbook
{
namespace common
{

template <class T>
class WorkStealingQueue : public boost::noncopyable 
{
public:
    WorkStealingQueue()
        : _top(1)
        , _bottom(1)
        , _size(0) 
        , _data(NULL)
    { }

    ~WorkStealingQueue()
    {
        delete[] _data;
        _data = NULL;
    }

    bool init(size_t size)
    {
        if (size == 0)
        {
            return false;
        }
        _data = new(std::nothrow) T[size];
        if (_data == NULL)
        {
            return false;
        }
        _size = size;
        return true;
    }

    bool push(const T& val)
    {
        const size_t b = _bottom.load(boost::memory_order_relaxed);
        const size_t t = _top.load(boost::memory_order_acquire);
        if (b - t > _size - 1)
        {
            // Full Queue
            return false;
        }
        _data[b % _size] = val;
        boost::atomic_thread_fence(boost::memory_order_release);
        _bottom.store(b + 1, boost::memory_order_relaxed);
        return true;
    }

    bool pop(T* val)
    {
        const size_t b = _bottom.load(boost::memory_order_relaxed) - 1;
        _bottom.store(b, boost::memory_order_relaxed);
        boost::atomic_thread_fence(boost::memory_order_seq_cst);
        size_t t = _top.load(boost::memory_order_relaxed);

        bool result = false;
        if (t <= b)
        {
            // non-empty queue
            *val = _data[b % _size];
            result = true;
            if (t == b) 
            {
                if (!_top.compare_exchange_strong(t, t + 1, 
                                                  boost::memory_order_seq_cst, 
                                                  boost::memory_order_relaxed))
                {
                    // failed race
                    result = false;
                }
                _bottom.store(b + 1, boost::memory_order_relaxed);
            }
        }
        else
        {
            // empty queue
            _bottom.store(b + 1, boost::memory_order_relaxed);
            result = false;
        }
        return result;
    }

    bool steal(T* val)
    {
        size_t t = _top.load(boost::memory_order_acquire);
        boost::atomic_thread_fence(boost::memory_order_seq_cst);
        const size_t b = _bottom.load(boost::memory_order_acquire);
        if (t >= b)
        {
            // empty queue
            return false;
        }

        // non-empty queue
        *val = _data[t % _size];
        if (!_top.compare_exchange_strong(t, t + 1, 
                                          boost::memory_order_seq_cst, 
                                          boost::memory_order_relaxed))
        {
            // failed race
            return false;
        }
        return true;
    }

    size_t size()
    {
        const size_t t = _top.load();
        const size_t b = _bottom.load();
        return b - t > 0 ? b - t : 0;
    }

private:
    boost::atomic<size_t> _top;
    boost::atomic<size_t> _bottom;
    size_t _size;
    T* _data;
};

} // end namespace common
} // end namespace logbook

#endif
