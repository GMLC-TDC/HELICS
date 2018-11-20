/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include "TestComms.h"
#include "../../common/fmt_format.h"
#include "../ActionMessage.hpp"
#include "../BrokerFactory.hpp"
#include "../CommonCore.hpp"
#include "../CoreBroker.hpp"
#include "../CoreFactory.hpp"
#include "../NetworkBrokerData.hpp"

#include <memory>

namespace helics
{
namespace testcore
{
TestComms::TestComms () : CommsInterface (CommsInterface::thread_generation::single) {}

/** destructor*/
TestComms::~TestComms () { disconnect (); }

void TestComms::loadNetworkInfo (const NetworkBrokerData &netInfo)
{
    CommsInterface::loadNetworkInfo (netInfo);
    if (!propertyLock ())
    {
        return;
    }
    // brokerPort = netInfo.brokerPort;
    // PortNumber = netInfo.portNumber;
    if (localTarget_.empty ())
    {
        localTarget_ = name;
    }

    // if (PortNumber > 0)
    //{
    //    autoPortNumber = false;
    //}
    propertyUnLock ();
}

void TestComms::queue_rx_function () {}

void TestComms::queue_tx_function ()
{
    // make sure the link to the localTarget_ is in place
    if (name != localTarget_)
    {
        if (!brokerName_.empty ())
        {
            if (!CoreFactory::copyCoreIdentifier (name, localTarget_))
            {
                if (!BrokerFactory::copyBrokerIdentifier (name, localTarget_))
                {
                    setRxStatus (connection_status::error);
                    setTxStatus (connection_status::error);
                    return;
                }
            }
        }
        else
        {
            if (!BrokerFactory::copyBrokerIdentifier (name, localTarget_))
            {
                setRxStatus (connection_status::error);
                setTxStatus (connection_status::error);
                return;
            }
        }
    }
    setRxStatus (connection_status::connected);
    std::shared_ptr<CoreBroker> tbroker;

    if (brokerName_.empty ())
    {
        if (!brokerTarget_.empty ())
        {
            brokerName_ = brokerTarget_;
        }
    }

    if (!brokerName_.empty ())
    {
        std::chrono::milliseconds totalSleep (0);
        while (!tbroker)
        {

            auto broker = BrokerFactory::findBroker(brokerName_);
            tbroker = std::dynamic_pointer_cast<CoreBroker> (broker);
            if (!tbroker)
            {
                if (autoBroker)
                {
                    tbroker = std::static_pointer_cast<CoreBroker> (
                        BrokerFactory::create(core_type::TEST, brokerName_, brokerInitString_));
                    tbroker->connect();
                }
                else
                {
                    if (totalSleep > connectionTimeout)
                    {
                        setTxStatus(connection_status::error);
                        setRxStatus(connection_status::error);
                        return;
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(200));
                    totalSleep += std::chrono::milliseconds(200);
                }
            }
            else
            {
                if (!tbroker->isOpenToNewFederates())
                {
                    std::cerr << "broker is not open to new federates " << brokerName_ << std::endl;
                    tbroker = nullptr;
                    broker = nullptr;
                    BrokerFactory::cleanUpBrokers(std::chrono::milliseconds(200));
                    totalSleep += std::chrono::milliseconds(200);
                    if (totalSleep > std::chrono::milliseconds(connectionTimeout))
                    {
                        setTxStatus(connection_status::error);
                        setRxStatus(connection_status::error);
                        return;
                    }
                }
            }
        }
    }
    else if (!serverMode)
    {
        std::chrono::milliseconds totalSleep(0);
        while (!tbroker)
        {
            auto broker = BrokerFactory::findJoinableBrokerOfType(core_type::TEST);
            tbroker = std::dynamic_pointer_cast<CoreBroker> (broker);
            if (!tbroker)
            {
                if (autoBroker)
                {
                    tbroker = std::static_pointer_cast<CoreBroker> (
                        BrokerFactory::create(core_type::TEST, "", brokerInitString_));
                    tbroker->connect();
                }
                else
                {
                    if (totalSleep > connectionTimeout)
                    {
                        setTxStatus(connection_status::error);
                        setRxStatus(connection_status::error);
                        return;
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(200));
                    totalSleep += std::chrono::milliseconds(200);
                }
            }
        }
        
    }
    if (tbroker)
    {
        if (tbroker->getIdentifier() == localTarget_)
        {
            logError("broker == target");
        }
		if (!tbroker->isOpenToNewFederates())
		{
            logError ("broker is not open to new federates");
		}
    }
    
    setTxStatus (connection_status::connected);
    std::map<route_id_t, std::shared_ptr<BrokerBase>> routes;

    while (true)
    {
        route_id_t route_id;
        ActionMessage cmd;

        std::tie (route_id, cmd) = txQueue.pop ();
        bool processed = false;
        if (isProtocolCommand (cmd))
        {
            if (route_id == control_route)
            {
                switch (cmd.messageID)
                {
                case NEW_ROUTE:
                {
                    auto &newroute = cmd.payload;
                    bool foundRoute = false;
                    auto core = CoreFactory::findCore (newroute);
                    if (core)
                    {
                        auto tcore = std::dynamic_pointer_cast<CommonCore> (core);
                        if (tcore)
                        {
                            routes.emplace (route_id_t (cmd.getExtraData ()), std::move (tcore));
                            foundRoute = true;
                        }
                    }
                    auto brk = BrokerFactory::findBroker (newroute);

                    if (brk)
                    {
                        auto cbrk = std::dynamic_pointer_cast<CoreBroker> (brk);
                        if (cbrk)
                        {
                            routes.emplace (route_id_t (cmd.getExtraData ()), std::move (cbrk));
                            foundRoute = true;
                        }
                    }
					if (!foundRoute)
					{
                        logError (std::string("unable to establish Route to ")+newroute);
					}
                    processed = true;
                }
                break;
                case CLOSE_RECEIVER:
                    setRxStatus (connection_status::terminated);
                    processed = true;
                    break;
                case DISCONNECT:
                    goto CLOSE_TX_LOOP;  // break out of loop
                }
            }
        }
        if (processed)
        {
            continue;
        }

        if (route_id == parent_route_id)
        {
            if (tbroker)
            {
                tbroker->addActionMessage (std::move (cmd));
            }
            else
            {
                logWarning (
                  fmt::format ("message directed to broker of comm system with no broker, message dropped {}",
                               prettyPrintString (cmd)));
            }
        }
        else
        {
            auto rt_find = routes.find (route_id);
            if (rt_find != routes.end ())
            {
                rt_find->second->addActionMessage (std::move (cmd));
            }
            else
            {
                if (tbroker)
                {
                    tbroker->addActionMessage (std::move (cmd));
                }
                else
                {
                    logWarning ("unknown route, message dropped");
                }
            }
        }
    }
CLOSE_TX_LOOP:

    routes.clear ();
    tbroker = nullptr;

    setTxStatus (connection_status::terminated);
}

void TestComms::closeReceiver ()
{
    if (getTxStatus () == connection_status::connected)
    {
        ActionMessage cmd (CMD_PROTOCOL);
        cmd.messageID = CLOSE_RECEIVER;
        transmit (control_route, cmd);
    }
}

std::string TestComms::getAddress () const { return localTarget_; }

}  // namespace testcore

}  // namespace helics
