/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#ifndef HELICS_CPP98_ENDPOINT_HPP_
#define HELICS_CPP98_ENDPOINT_HPP_
#pragma once

#include "../shared_api_library/MessageFederate.h"
#include "helicsExceptions.hpp"

namespace helicscpp
{
class Endpoint
{
  public:
    explicit Endpoint (helics_endpoint hep) : ep (hep) {}
    Endpoint (){};

    Endpoint (const Endpoint &endpoint) : ep (endpoint.ep) {}

    Endpoint &operator= (const Endpoint &endpoint)
    {
        ep = endpoint.ep;
        return *this;
    }
    operator helics_endpoint () { return ep; }

    helics_endpoint baseObject () const { return ep; }
    /* Checks if endpoint has unread messages **/
    bool hasMessage () const
    {
        // returns int, 1 = true, 0 = false
        return helicsEndpointHasMessage (ep) > 0;
    }
    /** set the default destination for an endpoint*/
    void setDefaultDestination (const std::string &dest)
    {
        helicsEndpointSetDefaultDestination (ep, dest.c_str (), hThrowOnError ());
    }
    /** get the default destination for an endpoint*/
    const char *getDefaultDestination () const { return helicsEndpointGetDefaultDestination (ep); }
    /** Returns the number of pending receives for endpoint **/
    uint64_t pendingMessages () const { return helicsEndpointPendingMessages (ep); }

    /** Get a packet from an endpoint **/
    helics_message getMessage () { return helicsEndpointGetMessage (ep); }

    /** Methods for sending a message **/
    void sendMessage (const char *data, size_t len)
    {
        helicsEndpointSendMessageRaw (ep, NULL, data, static_cast<int> (len), hThrowOnError ());
    }

    /** Methods for sending a message **/
    void sendMessage (const std::string &dest, const char *data, size_t len)
    {
        helicsEndpointSendMessageRaw (ep, dest.c_str (), data, static_cast<int> (len), hThrowOnError ());
    }

    void sendMessage (const char *data, size_t len, helics_time time)
    {
        helicsEndpointSendEventRaw (ep, NULL, data, static_cast<int> (len), time, hThrowOnError ());
    }

    void sendMessage (const std::string &dest, const char *data, size_t len, helics_time time)
    {
        helicsEndpointSendEventRaw (ep, dest.c_str (), data, static_cast<int> (len), time, hThrowOnError ());
    }
    /** Methods for sending a message **/
    void sendMessage (const std::string &data)
    {
        helicsEndpointSendMessageRaw (ep, NULL, data.c_str (), static_cast<int> (data.size ()), hThrowOnError ());
    }

    /** Methods for sending a message **/
    void sendMessage (const std::string &dest, const std::string &data)
    {
        helicsEndpointSendMessageRaw (ep, dest.c_str (), data.c_str (), static_cast<int> (data.size ()),
                                      hThrowOnError ());
    }

    void sendMessage (const std::string &data, helics_time time)
    {
        helicsEndpointSendEventRaw (ep, NULL, data.c_str (), static_cast<int> (data.size ()), time,
                                    hThrowOnError ());
    }

    void sendMessage (const std::string &dest, const std::string &data, helics_time time)
    {
        helicsEndpointSendEventRaw (ep, dest.c_str (), data.c_str (), static_cast<int> (data.size ()), time,
                                    hThrowOnError ());
    }

    /** Methods for sending a message **/
    void sendMessage (const std::vector<char> &data)
    {
        helicsEndpointSendMessageRaw (ep, NULL, data.data (), static_cast<int> (data.size ()), hThrowOnError ());
    }

    /** Methods for sending a message **/
    void sendMessage (const std::string &dest, const std::vector<char> &data)
    {
        helicsEndpointSendMessageRaw (ep, dest.c_str (), data.data (), static_cast<int> (data.size ()),
                                      hThrowOnError ());
    }

    void sendMessage (const std::vector<char> &data, helics_time time)
    {
        helicsEndpointSendEventRaw (ep, NULL, data.data (), static_cast<int> (data.size ()), time,
                                    hThrowOnError ());
    }

    void sendMessage (const std::string &dest, const std::vector<char> &data, helics_time time)
    {
        helicsEndpointSendEventRaw (ep, dest.c_str (), data.data (), static_cast<int> (data.size ()), time,
                                    hThrowOnError ());
    }

    void sendMessage (helics_message &message)
    {
        // returns helicsStatus
        helicsEndpointSendMessage (ep, &message, hThrowOnError ());
    }

    const char *getName () const { return helicsEndpointGetName (ep); }

    std::string getType () { return helicsEndpointGetType (ep); }

  private:
    helics_endpoint ep;
};
}  // namespace helicscpp
#endif
