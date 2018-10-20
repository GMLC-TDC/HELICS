/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include "TcpCommsCommon.h"
#include "../../common/AsioServiceManager.h"
#include "../ActionMessage.hpp"
#include "../NetworkBrokerData.hpp"
#include "TcpHelperClasses.h"
#include <memory>

namespace helics
{
namespace tcp
{
TcpConnection::pointer makeConnection (boost::asio::io_service &io_service,
                                       const std::string &connection,
                                       const std::string &port,
                                       size_t bufferSize,
                                       std::chrono::milliseconds timeOut)
{
    using namespace std::chrono;
    auto tick = steady_clock::now ();
    milliseconds timeRemaining (timeOut);
    milliseconds timeRemPrev (timeOut);
    TcpConnection::pointer connectionPtr = TcpConnection::create (io_service, connection, port, bufferSize);
    int trycnt = 1;
    while (!connectionPtr->waitUntilConnected (timeRemaining))
    {
        auto tock = steady_clock::now ();
        timeRemaining = milliseconds (timeOut) - duration_cast<milliseconds> (tock - tick);
        if ((timeRemaining < milliseconds (0)) && (trycnt > 1))
        {
            connectionPtr = nullptr;
            break;
        }
        // make sure we slow down and sleep for a little bit
        if (timeRemPrev - timeRemaining < milliseconds (100))
        {
            std::this_thread::sleep_for (milliseconds (200));
        }
        timeRemPrev = timeRemaining;
        if (timeRemaining < milliseconds (0))
        {
            timeRemaining = milliseconds (400);
        }

        // lets try to connect again
        ++trycnt;
        connectionPtr = TcpConnection::create (io_service, connection, port, bufferSize);
    }
    return connectionPtr;
}
}  // namespace tcp
}  // namespace helics
