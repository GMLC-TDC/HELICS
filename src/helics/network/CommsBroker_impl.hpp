/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once
#include "CommsBroker.hpp"
#include "CommsInterface.hpp"
#include "helics/core/BrokerBase.hpp"

#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <utility>
namespace helics {
template<class COMMS, class BrokerT>
CommsBroker<COMMS, BrokerT>::CommsBroker() noexcept
{
    static_assert(std::is_base_of<CommsInterface, COMMS>::value,
                  "COMMS object must be a CommsInterface Object");
    static_assert(std::is_base_of<BrokerBase, BrokerT>::value,
                  "Broker must be an object  with a base of BrokerBase");
    loadComms();
}

template<class COMMS, class BrokerT>
CommsBroker<COMMS, BrokerT>::CommsBroker(bool arg) noexcept: BrokerT(arg)
{
    static_assert(std::is_base_of<CommsInterface, COMMS>::value,
                  "COMMS object must be a CommsInterface Object");
    static_assert(std::is_base_of<BrokerBase, BrokerT>::value,
                  "Broker must be an object  with a base of BrokerBase");
    loadComms();
}

template<class COMMS, class BrokerT>
CommsBroker<COMMS, BrokerT>::CommsBroker(const std::string& obj_name): BrokerT(obj_name)
{
    static_assert(std::is_base_of<CommsInterface, COMMS>::value,
                  "COMMS object must be a CommsInterface Object");
    static_assert(std::is_base_of<BrokerBase, BrokerT>::value,
                  "Broker must be an object  with a base of BrokerBase");
    loadComms();
}
template<class COMMS, class BrokerT>
void CommsBroker<COMMS, BrokerT>::loadComms()
{
    comms = std::make_unique<COMMS>();
    comms->setCallback([this](ActionMessage&& M) { BrokerBase::addActionMessage(std::move(M)); });
    comms->setLoggingCallback(BrokerBase::getLoggingCallback());
}

template<class COMMS, class BrokerT>
CommsBroker<COMMS, BrokerT>::~CommsBroker()
{
    BrokerBase::haltOperations = true;
    int exp = 2;
    while (!disconnectionStage.compare_exchange_weak(exp, 3)) {
        if (exp == 0) {
            commDisconnect();
            exp = 1;
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }
    comms = nullptr;  // need to ensure the comms are deleted before the callbacks become invalid
    BrokerBase::joinAllThreads();
}

template<class COMMS, class BrokerT>
void CommsBroker<COMMS, BrokerT>::brokerDisconnect()
{
    commDisconnect();
}

template<class COMMS, class BrokerT>
void CommsBroker<COMMS, BrokerT>::commDisconnect()
{
    int exp = 0;
    if (disconnectionStage.compare_exchange_strong(exp, 1)) {
        comms->disconnect();
        disconnectionStage = 2;
    }
}

template<class COMMS, class BrokerT>
bool CommsBroker<COMMS, BrokerT>::tryReconnect()
{
    return comms->reconnect();
}

template<class COMMS, class BrokerT>
void CommsBroker<COMMS, BrokerT>::transmit(route_id rid, const ActionMessage& cmd)
{
    comms->transmit(rid, cmd);
}

template<class COMMS, class BrokerT>
void CommsBroker<COMMS, BrokerT>::transmit(route_id rid, ActionMessage&& cmd)
{
    comms->transmit(rid, std::move(cmd));
}

template<class COMMS, class BrokerT>
void CommsBroker<COMMS, BrokerT>::addRoute(route_id rid,
                                           int /*interfaceId*/,
                                           const std::string& routeInfo)
{
    comms->addRoute(rid, routeInfo);
}

template<class COMMS, class BrokerT>
void CommsBroker<COMMS, BrokerT>::removeRoute(route_id rid)
{
    comms->removeRoute(rid);
}

template<class COMMS, class BrokerT>
COMMS* CommsBroker<COMMS, BrokerT>::getCommsObjectPointer()
{
    return comms.get();
}

}  // namespace helics
