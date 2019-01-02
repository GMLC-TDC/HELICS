/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#include "ZmqCore.h"
#include "../../common/zmqContextManager.h"
#include "../NetworkCore_impl.hpp"
#include "ZmqComms.h"
#include "ZmqCommsSS.h"

namespace helics
{
namespace zeromq
{
ZmqCore::ZmqCore () noexcept { netInfo.server_mode = NetworkBrokerData::server_mode_options::server_deactivated; }

ZmqCore::ZmqCore (const std::string &core_name) : NetworkCore (core_name)
{
    netInfo.server_mode = NetworkBrokerData::server_mode_options::server_deactivated;
}

bool ZmqCore::brokerConnect ()
{
    zmqContextManager::startContext ();
    return NetworkCore::brokerConnect ();
}

ZmqCoreSS::ZmqCoreSS () noexcept { netInfo.server_mode = NetworkBrokerData::server_mode_options::server_deactivated; }

ZmqCoreSS::ZmqCoreSS (const std::string &core_name) : NetworkCore (core_name)
{
    netInfo.server_mode = NetworkBrokerData::server_mode_options::server_deactivated;
}

bool ZmqCoreSS::brokerConnect ()
{
    zmqContextManager::startContext ();
    return NetworkCore::brokerConnect ();
}

}  // namespace zeromq
}  // namespace helics
