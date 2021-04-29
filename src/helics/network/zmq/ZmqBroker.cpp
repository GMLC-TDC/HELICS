/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "ZmqBroker.h"

#include "../NetworkBroker_impl.hpp"
#include "ZmqComms.h"
#include "ZmqCommsSS.h"
#include "ZmqContextManager.h"

#include <string>

namespace helics {
namespace zeromq {
    ZmqBroker::ZmqBroker(bool rootBroker) noexcept: NetworkBroker(rootBroker)
    {
        netInfo.server_mode = NetworkBrokerData::server_mode_options::server_active;
    }

    ZmqBroker::ZmqBroker(const std::string& brokerName): NetworkBroker(brokerName)
    {
        netInfo.server_mode = NetworkBrokerData::server_mode_options::server_active;
    }

    bool ZmqBroker::brokerConnect()
    {
        ZmqContextManager::startContext();
        return NetworkBroker::brokerConnect();
    }

    ZmqBrokerSS::ZmqBrokerSS(bool rootBroker) noexcept: NetworkBroker(rootBroker)
    {
        netInfo.server_mode = NetworkBrokerData::server_mode_options::server_active;
    }

    ZmqBrokerSS::ZmqBrokerSS(const std::string& broker_name): NetworkBroker(broker_name)
    {
        netInfo.server_mode = NetworkBrokerData::server_mode_options::server_active;
    }

    bool ZmqBrokerSS::brokerConnect()
    {
        ZmqContextManager::startContext();
        return NetworkBroker::brokerConnect();
    }

}  // namespace zeromq
}  // namespace helics
