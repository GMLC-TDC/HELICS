/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include "NetworkCommsInterface.hpp"
#include "../common/fmt_format.h"
#include "ActionMessage.hpp"
#include "NetworkBrokerData.hpp"
#include <memory>


namespace helics
{

NetworkCommsInterface::NetworkCommsInterface()
{

}

/** load network information into the comms object*/
void NetworkCommsInterface::loadNetworkInfo (const NetworkBrokerData &netInfo)
{
    CommsInterface::loadNetworkInfo (netInfo);
    if (!propertyLock ())
    {
        return;
    }
    brokerPort = netInfo.brokerPort;
    PortNumber = netInfo.portNumber;
    if (localTarget_.empty ())
    {
        if ((brokerTarget_ == "udp://127.0.0.1") || (brokerTarget_ == "udp://localhost") ||
            (brokerTarget_ == "localhost"))
        {
            localTarget_ = "localhost";
        }
        else if (brokerTarget_.empty ())
        {
            switch (interfaceNetwork)
            {
            case interface_networks::local:
                localTarget_ = "localhost";
                break;
            default:
                localTarget_ = "*";
                break;
            }
        }
        else
        {
            localTarget_ = generateMatchingInterfaceAddress (brokerTarget_, interfaceNetwork);
        }
    }
    if (netInfo.portStart > 0)
    {
        openPortStart = netInfo.portStart;
    }

    if (PortNumber > 0)
    {
        autoPortNumber = false;
    }
    propertyUnLock ();
}
/** destructor*/
NetworkCommsInterface::~NetworkCommsInterface() { }

void NetworkCommsInterface::setBrokerPort (int brokerPortNumber)
{
    if (propertyLock ())
    {
        brokerPort = brokerPortNumber;
        propertyUnLock ();
    }
}



int NetworkCommsInterface::findOpenPort ()
{
    int start = openPortStart;
    if (openPortStart < 0)
    {
        //start = (hasBroker) ? BEGIN_OPEN_PORT_RANGE_SUBBROKER : BEGIN_OPEN_PORT_RANGE;
    }
    while (usedPortNumbers.find (start) != usedPortNumbers.end ())
    {
        start += 2;
    }
    usedPortNumbers.insert (start);
    return start;
}

void NetworkCommsInterface::setPortNumber (int localPortNumber)
{
    if (propertyLock ())
    {
        PortNumber = localPortNumber;
        if (PortNumber > 0)
        {
            autoPortNumber = false;
        }
        propertyUnLock ();
    }
}

void NetworkCommsInterface::setAutomaticPortStartPort (int startingPort)
{
    if (propertyLock ())
    {
        openPortStart = startingPort;
        propertyUnLock ();
    }
}

ActionMessage NetworkCommsInterface::generateReplyToIncomingMessage (ActionMessage &M)
{
    if (isProtocolCommand (M))
    {
        switch (M.messageID)
        {
        case QUERY_PORTS:
        {
            ActionMessage portReply (CMD_PROTOCOL);
            portReply.messageID = PORT_DEFINITIONS;
            portReply.source_id = PortNumber;
            return portReply;
        }
        break;
        case REQUEST_PORTS:
        {
            auto openPort = findOpenPort ();
            ActionMessage portReply (CMD_PROTOCOL);
            portReply.messageID = PORT_DEFINITIONS;
            portReply.source_id = PortNumber;
            portReply.source_handle = openPort;
            return portReply;
        }
        break;
        default:
            break;
        }
    }
    ActionMessage resp (CMD_IGNORE);
    return resp;
}

std::string NetworkCommsInterface::getAddress () const { return makePortAddress (localTarget_, PortNumber); }

}  // namespace helics
