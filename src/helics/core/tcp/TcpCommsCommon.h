/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once

/** @file
@details function in this file are common function used between the different TCP comms */

#include "TcpHelperClasses.h"
#include <chrono>

#include "../../common/AsioServiceManagerFwd.hpp"
namespace helics
{
	namespace tcp
	{
    /** establish a connection to a server by as associated timeout*/
    TcpConnection::pointer makeConnection (boost::asio::io_service &io_service,
                                           const std::string &connection,
                                           const std::string &port,
                                           size_t bufferSize, std::chrono::milliseconds timeOut);
	}
}
