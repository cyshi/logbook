// Copyright (c) 2016 Baidu.com, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// 
// Author: shichengyi@baidu.com (Shi Chengyi)
// Data:2016-10-13 15:52

#include <core/raft_server.h>
#include <core/raft_server_impl.h>

namespace logbook
{
namespace core
{

RaftServer::RaftServer()
{
    _impl.reset(new RaftServerImpl());
}

RaftServer::~RaftServer()
{
}

bool RaftServer::start(const Configure& conf)
{
    return _impl->start(conf);
}

Result RaftServer::submit_command(const Command& cmd)
{
    return _impl->submit_command(cmd);
}

} // end namespace logbook
} // end namespace core
