/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#pragma once
#include "BrokerBase.hpp"
#include "CommsBroker.hpp"
#include "CommsInterface.hpp"
#include <atomic>
#include <mutex>
#include <thread>
namespace helics
{
template <class COMMS, class BrokerT>
CommsBroker<COMMS, BrokerT>::CommsBroker () noexcept
{
    static_assert (std::is_base_of<CommsInterface, COMMS>::value, "COMMS object must be a CommsInterface Object");
    static_assert (std::is_base_of<BrokerBase, BrokerT>::value,
                   "Broker must be an object  with a base of BrokerBase");
    loadComms ();
}

template <class COMMS, class BrokerT>
CommsBroker<COMMS, BrokerT>::CommsBroker (bool arg) noexcept : BrokerT (arg)
{
    static_assert (std::is_base_of<CommsInterface, COMMS>::value, "COMMS object must be a CommsInterface Object");
    static_assert (std::is_base_of<BrokerBase, BrokerT>::value,
                   "Broker must be an object  with a base of BrokerBase");
    loadComms ();
}

template <class COMMS, class BrokerT>
CommsBroker<COMMS, BrokerT>::CommsBroker (const std::string &obj_name) : BrokerT (obj_name)
{
    static_assert (std::is_base_of<CommsInterface, COMMS>::value, "COMMS object must be a CommsInterface Object");
    static_assert (std::is_base_of<BrokerBase, BrokerT>::value,
                   "Broker must be an object  with a base of BrokerBase");
    loadComms ();
}
template <class COMMS, class BrokerT>
void CommsBroker<COMMS, BrokerT>::loadComms ()
{
    comms = std::make_unique<COMMS> ();
    comms->setCallback ([this](ActionMessage &&M) { BrokerBase::addActionMessage (std::move (M)); });
}

template <class COMMS, class BrokerT>
CommsBroker<COMMS, BrokerT>::~CommsBroker ()
{
    BrokerBase::haltOperations = true;
    int exp = 2;
    while (!disconnectionStage.compare_exchange_weak (exp, 3))
    {
        if (exp == 0)
        {
            commDisconnect ();
            exp = 1;
        }
        else
        {
            std::this_thread::sleep_for (std::chrono::milliseconds (50));
        }
    }
    comms = nullptr;  // need to ensure the comms are deleted before the callbacks become invalid
    BrokerBase::joinAllThreads ();
}

template <class COMMS, class BrokerT>
void CommsBroker<COMMS, BrokerT>::brokerDisconnect ()
{
    commDisconnect ();
}

template <class COMMS, class BrokerT>
void CommsBroker<COMMS, BrokerT>::commDisconnect ()
{
    int exp = 0;
    if (disconnectionStage.compare_exchange_strong (exp, 1))
    {
        comms->disconnect ();
        disconnectionStage = 2;
    }
}

template <class COMMS, class BrokerT>
bool CommsBroker<COMMS, BrokerT>::tryReconnect ()
{
    return comms->reconnect ();
}

template <class COMMS, class BrokerT>
void CommsBroker<COMMS, BrokerT>::transmit (route_id_t route_id, const ActionMessage &cmd)
{
    comms->transmit (route_id, cmd);
}

template <class COMMS, class BrokerT>
void CommsBroker<COMMS, BrokerT>::transmit (route_id_t route_id, ActionMessage &&cmd)
{
    comms->transmit (route_id, std::move (cmd));
}

template <class COMMS, class BrokerT>
void CommsBroker<COMMS, BrokerT>::addRoute (route_id_t route_id, const std::string &routeInfo)
{
    comms->addRoute (route_id, routeInfo);
}

template <class COMMS, class BrokerT>
void CommsBroker<COMMS, BrokerT>::removeRoute (route_id_t route_id)
{
    comms->removeRoute (route_id);
}

}  // namespace helics
