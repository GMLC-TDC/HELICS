/*
Copyright © 2017-2018,
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
    explicit Endpoint(helics_endpoint hep) :ep(hep)
    {
    }
    Endpoint() {};

    Endpoint(const Endpoint &endpoint) :ep(endpoint.ep)
    {
    }

    Endpoint &operator=(const Endpoint &endpoint)
    {
        ep = endpoint.ep;
        return *this;
    }
    operator helics_endpoint() { return ep; }

    helics_endpoint baseObject() const { return ep; }
    /* Checks if endpoint has unread messages **/
    bool hasMessage() const
    {
        // returns int, 1 = true, 0 = false
        return helicsEndpointHasMessage(ep,NULL) > 0;
    }

    /** Returns the number of pending receives for endpoint **/
    uint64_t pendingMessages() const
    {
        return helicsEndpointPendingMessages(ep,NULL);
    }

    /** Get a packet from an endpoint **/
    message_t getMessage()
    {
        return helicsEndpointGetMessage(ep,NULL);
    }


    /** Methods for sending a message **/
    void sendMessage(const std::string &dest, const char *data, size_t len)
    {
        helicsEndpointSendMessageRaw(ep, dest.c_str(), data, static_cast<int>(len),hThrowOnError());
    }

    void sendMessage(const std::string &dest, const char *data, size_t len, helics_time_t time)
    {
        helicsEndpointSendEventRaw (ep, dest.c_str (), data, static_cast<int> (len), time, hThrowOnError ());
    }
    /** Methods for sending a message **/
    void sendMessage(const std::string &dest, const std::string &data)
    {
        helicsEndpointSendMessageRaw (ep, dest.c_str (), data.c_str (), static_cast<int> (data.size ()),
                                      hThrowOnError ());
    }

    void sendMessage(const std::string &dest, const std::string &data, helics_time_t time)
    {
        helicsEndpointSendEventRaw (ep, dest.c_str (), data.c_str (), static_cast<int> (data.size ()), time,
                                    hThrowOnError ());
    }

    /** Methods for sending a message **/
    void sendMessage(const std::string &dest, const std::vector<char> &data)
    {
        helicsEndpointSendMessageRaw (ep, dest.c_str (), data.data (), static_cast<int> (data.size ()),
                                      hThrowOnError ());
    }

    void sendMessage(const std::string &dest, const std::vector<char> &data, helics_time_t time)
    {
        helicsEndpointSendEventRaw (ep, dest.c_str (), data.data (), static_cast<int> (data.size ()), time,
                                    hThrowOnError ());
    }

    void sendMessage(message_t &message)
    {
        // returns helicsStatus
        helicsEndpointSendMessage (ep, &message, hThrowOnError ());
    }

    std::string getName() const
    {
        char str[255];
        helicsEndpointGetName(ep, &str[0], sizeof(str),NULL);
        std::string result(str);
        return result;
    }

    std::string getType()
    {
        char str[255];
        helicsEndpointGetType(ep, &str[0], sizeof(str),NULL);
        std::string result(str);
        return result;
    }
private:
    helics_endpoint ep;
};
} //namespace helicscpp
#endif
