/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once
#include "global_federate_id.hpp"

#include <chrono>
#include <vector>

namespace helics {
class CommonCore;
class CoreBroker;
class ActionMessage;

/** struct for managing the timeouts on the individual connections*/
struct linkConnection {
    bool waitingForPingReply{false};  //!< indicator that the connection is waiting
    bool activeConnection{false};  //!< indicator that the connection is active
    bool disablePing{false};  //!< indicator that the connection doesn't respond to pings
    global_federate_id connection{0};  //!< the id of the connection
    decltype(std::chrono::steady_clock::now()) lastPing;
};
/** class to handle timeouts and other issues for cores and brokers*/
class TimeoutMonitor {
  public:
    /** tick function for a core,  executes one tick*/
    void tick(CommonCore* core);
    /** tick function for a broker,  executes one tick*/
    void tick(CoreBroker* brk);
    /** get a ping reply*/
    void pingReply(const ActionMessage& m, CoreBroker* brk = nullptr);
    /**  set the overall timeout for the connection monitor*/
    void setTimeout(std::chrono::milliseconds to) { timeout = to; }
    /** reset the monitor to initial conditions*/
    void reset();
    /** ping all a brokers sub connections*/
    void pingSub(CoreBroker* brk);
    /** set the parent id*/
    void setParentId(global_broker_id parent_id) { parentConnection.connection = parent_id; }
    /** set the "pingability" of a parent connection*/
    void disableParentPing(bool value = true)
    {
        parentConnection.disablePing = value;
        parentConnection.waitingForPingReply = false;
    }

  private:
    std::chrono::milliseconds timeout{100'000'000};  //!< timeout for connections
    bool waitingForConnection{false};  //!< waiting for initial connection
    decltype(std::chrono::steady_clock::now()) startWaiting;  //!< time that the waiting has started
    linkConnection parentConnection;  //!< the connection information for the parent
    std::vector<linkConnection> connections;  //!< connection information for the other connections

    // int tickCounter;
};

}  // namespace helics
