/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/

#ifndef HELICS_CPP98_MESSAGE_FEDERATE_HPP_
#define HELICS_CPP98_MESSAGE_FEDERATE_HPP_
#pragma once

#include "MessageFederate_c.h"

namespace helics
{
class MessageFederate : public virtual Federate
{
  public:
    MessageFederate (const FederateInfo &fi)
    {
        fed = helicsCreateMessageFederate (fi.getInfo ());
    }

    MessageFederate (const std::string &jsonString)
    {
        fed = helicsCreateMessageFederateFromFile (jsonString.c_str());
    }

    virtual ~MessageFederate ()
    {
    }

    /** Methods for registering endpoints **/
    helics_endpoint registerEndpoint (const std::string &name, const std::string &type = "")
    {
        helics_endpoint ep = helicsRegisterEndpoint (fed, name.c_str(), type.c_str());
        local_endpoints.push_back(ep);
        return ep;
    }

    helics_endpoint registerGlobalEndpoint (const std::string &name, const std::string &type = "")
    {
        helics_endpoint ep = helicsRegisterGlobalEndpoint (fed, name.c_str(), type.c_str());
        local_endpoints.push_back(ep);
        return ep;
    }

    /** Subscribe to an endpoint **/
    void subscribe (helics_endpoint ep, const std::string &name, const std::string &type)
    {
        // returns helicsStatus
        helicsSubscribe (ep, name.c_str(), type.c_str());
    }

    /** Checks if federate has any messages **/
    bool hasMessage () const
    {
        // returns int, 1 = true, 0 = false
        return helicsFederateHasMessage (fed) > 0;
    }

    /* Checks if endpoint has unread messages **/
    bool hasMessage (helics_endpoint ep) const
    {
        // returns int, 1 = true, 0 = false
        return helicsEndpointHasMessage (ep) > 0;
    }

    /** Returns the number of pending receives for endpoint **/
    uint64_t receiveCount (helics_endpoint ep) const
    {
        return helicsEndpointReceiveCount (ep);
    }

    /** Returns the number of pending receives for all endpoints. **/
    uint64_t receiveCount () const
    {
        return helicsFederateReceiveCount (fed);
    }

    /** Get a packet from an endpoint **/
    message_t getMessage (helics_endpoint ep)
    {
        return helicsEndpointGetMessage (ep);
    }

    /** Get a packet for any endpoints in the federate **/
    message_t getMessage ()
    {
        return helicsFederateGetMessage (fed);
    }

    /** Methods for sending a message **/
    void sendMessage (helics_endpoint source, const std::string &dest, const char *data, size_t len)
    {
        helicsSendMessageRaw (source, dest.c_str(), data, len);
    }
    
    void sendMessage (helics_endpoint source, const std::string &dest, const char *data, size_t len, helics_time_t time)
    {
        helicsSendEventRaw (source, dest.c_str(), data, len, time);
    }

    void sendMessage (helics_endpoint source, const message_t &message)
    {
        // returns helicsStatus
        helicsSendMessage (source, &message)
    }

    std::string getEndpointName (helics_endpoint ep) const
    {
        char str[255];
        helicsGetEndpointName (ep, &str[0], sizeof(str));
        std::string result (str);
        return result;
    }

    std::string getEndpointType (helics_endpoint ep);
    {
        char str[255];
        helicsGetEndpointType (ep, &str[0], sizeof(str));
        std::string result (str);
        return result;
    }
 
  private:
    std::vector<helics_endpoint> local_endpoints;
 
};
} //namespace helics
#endif
