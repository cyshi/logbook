// Copyright (c) 2016 Baidu.com, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// 
// Author: shichengyi@baidu.com (Shi Chengyi)
// Data:2016-10-13 16:01

#include <core/raft_server_impl.h>

namespace logbook
{
namespace core
{

RaftServerImpl::RaftServerImpl() { }
RaftServerImpl::~RaftServerImpl() { }

bool RaftServerImpl::start(const Configure& /*conf*/)
{
    return false;
}

Result RaftServerImpl::submit_command(const Command& /*cmd*/)
{
    return SUCCESS;
}

} // end namespace logbook
} // end namespace core
