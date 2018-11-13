/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#include "ZmqBroker.h"
#include "../../common/zmqContextManager.h"
#include "ZmqComms.h"
#include "ZmqCommsTest.h"
#include "../NetworkBroker_impl.hpp"

namespace helics
{
namespace zeromq
{
ZmqBroker::ZmqBroker(bool rootBroker) noexcept : NetworkBroker(rootBroker) { netInfo.server_mode = NetworkBrokerData::server_mode_options::server_active; }

ZmqBroker::ZmqBroker (const std::string &broker_name) : NetworkBroker (broker_name) { netInfo.server_mode = NetworkBrokerData::server_mode_options::server_active; }

bool ZmqBroker::brokerConnect ()
{
    zmqContextManager::startContext ();
    return NetworkBroker::brokerConnect();
}

void ZmqBroker::displayHelp(bool localOnly)
{
    NetworkBroker::displayHelp(localOnly);
}

ZmqBrokerTest::ZmqBrokerTest(bool rootBroker) noexcept : NetworkBroker(rootBroker) { netInfo.server_mode = NetworkBrokerData::server_mode_options::server_active; }

ZmqBrokerTest::ZmqBrokerTest (const std::string &broker_name) : NetworkBroker (broker_name) { netInfo.server_mode = NetworkBrokerData::server_mode_options::server_active; }

bool ZmqBrokerTest::brokerConnect ()
{
    zmqContextManager::startContext ();
    return NetworkBroker::brokerConnect();
}

void ZmqBrokerTest::displayHelp(bool localOnly)
{
    NetworkBroker::displayHelp(localOnly);
}

}  // namespace zeromq
}  // namespace helics
