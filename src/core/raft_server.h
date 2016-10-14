// Copyright (c) 2016 Baidu.com, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// 
// Author: shichengyi@baidu.com (Shi Chengyi)
// Data:2016-10-11 10:04

#ifndef BAIDU_LOGBOOK_RAFT_SERVER_H
#define BAIDU_LOGBOOK_RAFT_SERVER_H

#include <string>
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>

namespace logbook
{

namespace log
{
class LogEntry;
} // end namespace log

namespace core
{

class Command;
class Configure;
class Peer;
class RaftServerImpl;

enum Result
{
    SUCCESS,
    FAIL,
    NOT_LEADER
};

class RaftServer : boost::noncopyable
{
public:
    typedef boost::function<void (const Peer&)> LeadershipChangeCallback;
    void set_leadership_change_callback(const LeadershipChangeCallback& cb);

    typedef boost::function<
        void (const logbook::log::LogEntry&)
        > ApplyCommittedCallback;
    void set_apply_committed_callback(ApplyCommittedCallback& cb);

    RaftServer();

    ~RaftServer();

    bool start(const Configure& conf);

    Result submit_command(const Command& cmd);
private:
    boost::scoped_ptr<RaftServerImpl> _impl;
};

} // end namespace logbook
} // end namespace core

#endif
