/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#ifndef HELICS_CPP98_ENDPOINT_HPP_
#define HELICS_CPP98_ENDPOINT_HPP_
#pragma once

#include "../shared_api_library/MessageFederate.h"
#include "helicsExceptions.hpp"

#include <string>
#include <vector>

namespace helicscpp {
class Endpoint {
  public:
    /** construct from a helics_endpoint object*/
    explicit Endpoint(helics_endpoint hep) HELICS_NOTHROW: ep(hep) {}
    /** default constructor*/
    Endpoint() HELICS_NOTHROW: ep(HELICS_NULL_POINTER){};
    /** copy constructor*/
    Endpoint(const Endpoint& endpoint) HELICS_NOTHROW: ep(endpoint.ep) {}
    /** copy assignment*/
    Endpoint& operator=(const Endpoint& endpoint)
    {
        ep = endpoint.ep;
        return *this;
    }
    /** cast to a helics_endpoint object*/
    operator helics_endpoint() { return ep; }
    /** get the base helics_endpoint object for use in the c API functions*/
    helics_endpoint baseObject() const { return ep; }
    /* Checks if endpoint has unread messages **/
    bool hasMessage() const
    {
        // returns int, 1 = true, 0 = false
        return helicsEndpointHasMessage(ep) > 0;
    }
    /** set the default destination for an endpoint*/
    void setDefaultDestination(const std::string& dest)
    {
        helicsEndpointSetDefaultDestination(ep, dest.c_str(), hThrowOnError());
    }
    /** get the default destination for an endpoint*/
    const char* getDefaultDestination() const { return helicsEndpointGetDefaultDestination(ep); }
    /** Returns the number of pending receives for endpoint **/
    uint64_t pendingMessages() const { return helicsEndpointPendingMessages(ep); }

    /** Get a packet from an endpoint **/
    helics_message getMessage() { return helicsEndpointGetMessage(ep); }

    /** Methods for sending a message **/
    /** send a data block and length
    @param data pointer to data location
    @param data_size the length of the data
    */
    void sendMessage(const char* data, size_t data_size)
    {
        helicsEndpointSendMessageRaw(
            ep, HELICS_NULL_POINTER, data, static_cast<int>(data_size), hThrowOnError());
    }

    /** send a data block and length
    @param dest string name of the destination
    @param data pointer to data location
    @param data_size the length of the data
    */
    void sendMessage(const std::string& dest, const char* data, size_t data_size)
    {
        helicsEndpointSendMessageRaw(
            ep, dest.c_str(), data, static_cast<int>(data_size), hThrowOnError());
    }
    /** send a data block and length
   @param data pointer to data location
   @param data_size the length of the data
   @param time the time to send the message
   */
    void sendMessage(const char* data, size_t data_size, helics_time time)
    {
        helicsEndpointSendEventRaw(
            ep, HELICS_NULL_POINTER, data, static_cast<int>(data_size), time, hThrowOnError());
    }
    /** send a data block and length
	@param dest destination to send the message to
    @param data pointer to data location
    @param data_size the length of the data
    @param time the time to send the message
    */
    void sendMessage(const std::string& dest, const char* data, size_t data_size, helics_time time)
    {
        helicsEndpointSendEventRaw(
            ep, dest.c_str(), data, static_cast<int>(data_size), time, hThrowOnError());
    }
    /** send a string to the target destination
    @param data the information to send
    */
    void sendMessage(const std::string& data)
    {
        helicsEndpointSendMessageRaw(
            ep, HELICS_NULL_POINTER, data.c_str(), static_cast<int>(data.size()), hThrowOnError());
    }

    /** send a string to a particular destination
	 @param dest the target endpoint to send the data to
     @param data the information to send
   */
    void sendMessage(const std::string& dest, const std::string& data)
    {
        helicsEndpointSendMessageRaw(
            ep, dest.c_str(), data.c_str(), static_cast<int>(data.size()), hThrowOnError());
    }
    /** send a string at a particular time
     @param data the information to send
	 @param time the time the message should be delivered
   */
    void sendMessage(const std::string& data, helics_time time)
    {
        helicsEndpointSendEventRaw(
            ep,
            HELICS_NULL_POINTER,
            data.c_str(),
            static_cast<int>(data.size()),
            time,
            hThrowOnError());
    }
    /** send a string to a particular destination at a particular time
     @param dest the target endpoint to send the data to
     @param data the information to send
      @param time the time the message should be delivered
   */
    void sendMessage(const std::string& dest, const std::string& data, helics_time time)
    {
        helicsEndpointSendEventRaw(
            ep, dest.c_str(), data.c_str(), static_cast<int>(data.size()), time, hThrowOnError());
    }

    /** send a vector of data to the target destination
     @param data the information to send
   */
    void sendMessage(const std::vector<char>& data)
    {
        helicsEndpointSendMessageRaw(
            ep, HELICS_NULL_POINTER, data.data(), static_cast<int>(data.size()), hThrowOnError());
    }

    /** send a vector of data to a particular destination
     @param dest the target endpoint to send the data to
     @param data the information to send
   */
    void sendMessage(const std::string& dest, const std::vector<char>& data)
    {
        helicsEndpointSendMessageRaw(
            ep, dest.c_str(), data.data(), static_cast<int>(data.size()), hThrowOnError());
    }
    /** send a vector of data to the target destination at a particular time
     @param data the information to send
      @param time the time the message should be delivered
   */
    void sendMessage(const std::vector<char>& data, helics_time time)
    {
        helicsEndpointSendEventRaw(
            ep,
            HELICS_NULL_POINTER,
            data.data(),
            static_cast<int>(data.size()),
            time,
            hThrowOnError());
    }
    /** send a vector of data to a particular destination at a particular time
     @param dest the target endpoint to send the data to
     @param data the information to send
      @param time the time the message should be delivered
   */
    void sendMessage(const std::string& dest, const std::vector<char>& data, helics_time time)
    {
        helicsEndpointSendEventRaw(
            ep, dest.c_str(), data.data(), static_cast<int>(data.size()), time, hThrowOnError());
    }

    /** send a message object
   */
    void sendMessage(helics_message& message)
    {
        // returns helicsStatus
        helicsEndpointSendMessage(ep, &message, hThrowOnError());
    }
    /** get the name of the endpoint*/
    const char* getName() const { return helicsEndpointGetName(ep); }
    /** get the specified type of the endpoint*/
    std::string getType() { return helicsEndpointGetType(ep); }

    /** get the interface information field of the filter*/
    const char* getInfo() const { return helicsEndpointGetInfo(ep); }
    /** set the interface information field of the filter*/
    void setInfo(const std::string& info)
    {
        helicsEndpointSetInfo(ep, info.c_str(), HELICS_IGNORE_ERROR);
    }

  private:
    helics_endpoint ep; //!< the underlying helics_endpoint object
};
} // namespace helicscpp
#endif
