// Copyright (c) 2016 Baidu.com, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// 
// Author: shichengyi@baidu.com (Shi Chengyi)
// Data:2016-10-13 15:53

#ifndef BAIDU_LOGBOOK_RAFT_SERVER_IMPL_H
#define BAIDU_LOGBOOK_RAFT_SERVER_IMPL_H

#include <boost/noncopyable.hpp>
#include <core/raft_server.h>

namespace logbook
{
namespace core
{

class RaftServerImpl : boost::noncopyable
{
public:
    RaftServerImpl();
    ~RaftServerImpl();

    bool start(const Configure& conf);

    Result submit_command(const Command& cmd);
};

} // end namespace logbook
} // end namespace core

#endif
