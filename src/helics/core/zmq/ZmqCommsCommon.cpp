/*
Copyright (c) 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "ZmqCommsCommon.h"
#include "../NetworkBrokerData.hpp"
#include <thread>

namespace helics
{
namespace hzmq
{
using namespace std::chrono;
/** bind a zmq socket, with a timeout and timeout period*/
bool bindzmqSocket (zmq::socket_t &socket,
                    const std::string &address,
                    int port,
                    milliseconds timeout,
                    milliseconds period)
{
    bool bindsuccess = false;
    milliseconds tcount{0};
    while (!bindsuccess)
    {
        try
        {
            socket.bind (helics::makePortAddress (address, port));
            bindsuccess = true;
        }
        catch (const zmq::error_t &)
        {
            if (tcount == milliseconds (0))
            {
                // std::cerr << "zmq binding error on socket sleeping then will try again \n";
            }
            if (tcount > timeout)
            {
                break;
            }
            std::this_thread::sleep_for (period);
            tcount += period;
        }
    }
    return bindsuccess;
}

}  // namespace hzmq
}  // namespace helics
