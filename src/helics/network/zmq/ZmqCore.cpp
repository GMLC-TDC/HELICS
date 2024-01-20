/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "ZmqCore.h"

#include "../NetworkCore_impl.hpp"
#include "ZmqComms.h"
#include "ZmqCommsSS.h"
#include "ZmqContextManager.h"

#include <string>

namespace helics {
namespace zeromq {
    ZmqCore::ZmqCore() noexcept
    {
        netInfo.server_mode = NetworkBrokerData::ServerModeOptions::SERVER_DEACTIVATED;
    }

    ZmqCore::ZmqCore(std::string_view coreName): NetworkCore(coreName)
    {
        netInfo.server_mode = NetworkBrokerData::ServerModeOptions::SERVER_DEACTIVATED;
    }

    bool ZmqCore::brokerConnect()
    {
        ZmqContextManager::startContext();
        return NetworkCore::brokerConnect();
    }

    ZmqCoreSS::ZmqCoreSS() noexcept
    {
        netInfo.server_mode = NetworkBrokerData::ServerModeOptions::SERVER_DEACTIVATED;
        netInfo.appendNameToAddress = true;
    }

    ZmqCoreSS::ZmqCoreSS(std::string_view coreName): NetworkCore(coreName)
    {
        netInfo.server_mode = NetworkBrokerData::ServerModeOptions::SERVER_DEACTIVATED;
        netInfo.appendNameToAddress = true;
    }

    bool ZmqCoreSS::brokerConnect()
    {
        ZmqContextManager::startContext();
        return NetworkCore::brokerConnect();
    }

}  // namespace zeromq
}  // namespace helics
