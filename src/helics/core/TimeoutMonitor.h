/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once
#include <chrono>

namespace helics
{
class CommonCore;
class CoreBroker;
class ActionMessage;

/** class to handle timeouts and other issues for cores and brokers*/
class TimeoutMonitor
{
  public:
    void tick (CommonCore *core);
    void tick (CoreBroker *brk);
    void pingReply (const ActionMessage &m);

    void setTimeout (std::chrono::milliseconds to) { timeout = to; }

    void reset ()
    {
        waitingForPingReply = false;
        waitingForConnection = false;
    }

  private:
    bool waitingForPingReply{false};
    decltype (std::chrono::steady_clock::now ()) lastParentPing;
    decltype (std::chrono::steady_clock::now ()) startWaiting;
    std::chrono::milliseconds timeout{100000000};
    bool waitingForConnection{false};
    // int tickCounter;
};

}  // namespace helics
