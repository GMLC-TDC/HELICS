/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "ZmqCommsCommon.h"

#include "../NetworkBrokerData.hpp"
#include "cppzmq/zmq.hpp"

#include <string>
#include <thread>

namespace helics::zeromq {
using std::chrono::milliseconds;
/** bind a zmq socket, with a timeout and timeout period*/
bool bindzmqSocket(zmq::socket_t& socket,
                   const std::string& address,
                   int port,
                   milliseconds timeout,
                   milliseconds period)
{
    bool bindsuccess = false;
    milliseconds tcount{0};
    while (!bindsuccess) {
        try {
            socket.bind(gmlc::networking::makePortAddress(address, port));
            bindsuccess = true;
        }
        catch ([[maybe_unused]] const zmq::error_t& ze) {
            if (tcount == milliseconds{0}) {
                // std::cerr << "zmq binding error on socket sleeping then will try again \n";
            }
            if (tcount > timeout) {
                break;
            }
            std::this_thread::sleep_for(period);
            tcount += period;
        }
    }
    return bindsuccess;
}

std::string getZMQVersion()
{
    auto vers = zmq::version();
    return std::string("ZMQ v") + std::to_string(std::get<0>(vers)) + '.' +
        std::to_string(std::get<1>(vers)) + '.' + std::to_string(std::get<2>(vers));
}

}  // namespace helics::zeromq
