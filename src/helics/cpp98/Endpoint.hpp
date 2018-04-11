/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#ifndef HELICS_CPP98_ENDPOINT_HPP_
#define HELICS_CPP98_ENDPOINT_HPP_
#pragma once

#include "../shared_api_library/MessageFederate.h"


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
        return helicsEndpointHasMessage(ep) > 0;
    }

    /** Returns the number of pending receives for endpoint **/
    uint64_t receiveCount() const
    {
        return helicsEndpointReceiveCount(ep);
    }

    /** Get a packet from an endpoint **/
    message_t getMessage()
    {
        return helicsEndpointGetMessage(ep);
    }

   
    /** Methods for sending a message **/
    void sendMessage(const std::string &dest, const char *data, size_t len)
    {
        helicsEndpointSendMessageRaw(ep, dest.c_str(), data, static_cast<int>(len));
    }

    void sendMessage( const std::string &dest, const char *data, size_t len, helics_time_t time)
    {
        helicsEndpointSendEventRaw(ep, dest.c_str(), data, static_cast<int>(len), time);
    }

    void sendMessage( message_t &message)
    {
        // returns helicsStatus
        helicsEndpointSendMessage(ep, &message);
    }

    std::string getName() const
    {
        char str[255];
        helicsEndpointGetName(ep, &str[0], sizeof(str));
        std::string result(str);
        return result;
    }

    std::string getType()
    {
        char str[255];
        helicsEndpointGetType(ep, &str[0], sizeof(str));
        std::string result(str);
        return result;
    }
private:
    helics_endpoint ep;
};
#endif
