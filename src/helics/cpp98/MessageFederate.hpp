/*

Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#ifndef HELICS_CPP98_MESSAGE_FEDERATE_HPP_
#define HELICS_CPP98_MESSAGE_FEDERATE_HPP_
#pragma once

#include "helics.hpp"
#include "Federate.hpp"

namespace helics
{
class MessageFederate : public virtual Federate
{
  public:
    explicit MessageFederate (FederateInfo &fi)
    {
        fed = helicsCreateMessageFederate (fi.getInfo ());
    }

    explicit MessageFederate (const std::string &jsonString)
    {
        fed = helicsCreateMessageFederateFromJson (jsonString.c_str());
    }

    // Default constructor, not meant to be used
    MessageFederate () {}

    virtual ~MessageFederate ()
    {
    }

    /** Methods for registering endpoints **/
    helics_endpoint registerEndpoint (const std::string &name, const std::string &type = "")
    {
        helics_endpoint ep = helicsFederateRegisterEndpoint (fed, name.c_str(), type.c_str());
        local_endpoints.push_back(ep);
        return ep;
    }

    helics_endpoint registerGlobalEndpoint (const std::string &name, const std::string &type = "")
    {
        helics_endpoint ep = helicsFederateRegisterGlobalEndpoint (fed, name.c_str(), type.c_str());
        local_endpoints.push_back(ep);
        return ep;
    }

    /** Subscribe to an endpoint **/
    void subscribe (helics_endpoint ep, const std::string &name, const std::string &type)
    {
        // returns helicsStatus
        helicsEndpointSubscribe (ep, name.c_str(), type.c_str());
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
        helicsEndpointSendMessageRaw (source, dest.c_str(), data, len);
    }

    void sendMessage (helics_endpoint source, const std::string &dest, const char *data, size_t len, helics_time_t time)
    {
        helicsEndpointSendEventRaw (source, dest.c_str(), data, len, time);
    }

    void sendMessage (helics_endpoint source, message_t &message)
    {
        // returns helicsStatus
        helicsEndpointSendMessage (source, &message);
    }

    std::string getEndpointName (helics_endpoint ep) const
    {
        char str[255];
        helicsEndpointGetName (ep, &str[0], sizeof(str));
        std::string result (str);
        return result;
    }

    std::string getEndpointType (helics_endpoint ep)
    {
        char str[255];
        helicsEndpointGetType (ep, &str[0], sizeof(str));
        std::string result (str);
        return result;
    }

  private:
    std::vector<helics_endpoint> local_endpoints;

};
} //namespace helics
#endif

