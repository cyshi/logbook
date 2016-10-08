// Copyright (c) 2016 Baidu.com, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// 
// Author: shichengyi@baidu.com (Shi Chengyi)
// Data:2016-10-04 16:59

#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <common/thread_pool.h>
#include <common/work_stealing_queue.h>
#include <common/worker_thread.h>

using namespace logbook::common;

/*
boost::atomic<int> sum;

void* StealFunc(void* args)
{
    WorkStealingQueue<int>* q = (WorkStealingQueue<int>*)args;
    while (true)
    {
        int val;
        bool ret = q->steal(&val);
        if (ret)
        {
            sum += val;
        }
    }
}
*/

boost::atomic<int> count;

void task()
{
    count++;
}

void* test(void* p)
{
    ThreadPool* tp = (ThreadPool*)p;
    while (tp->add_task(boost::bind(task)))
    { }
    return NULL;
}

int main(int /*argc*/, const char** /*argv[]*/)
{
    ThreadPool tp;
    tp.start(1);
    std::vector<pthread_t> pids;
    for (int i = 0; i < 30; ++i)
    {
        pthread_t pid;
        pthread_create(&pid, NULL, test, (void *)&tp);
        pids.push_back(pid);
    }

    while (true)
    {
        fprintf(stdout, "count = %d\n", 
                count.load(boost::memory_order_relaxed));
        count = 0;
        sleep(1);
    }

    for (size_t i = 0; i < pids.size(); ++i)
    {
        pthread_join(pids[i], NULL);
    }

    tp.stop();

    return 0;
}
