/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
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

class Message {
  public:
    /** default constructor*/
    Message() HELICS_NOTHROW: mo(HELICS_NULL_POINTER) {}

    /** construct from a helics_message object*/
    explicit Message(helics_message_object hmo) HELICS_NOTHROW: mo(hmo) {}

    /** copy constructor*/
    Message(const Message& mess) HELICS_NOTHROW: mo(helicsMessageClone(mess.mo, HELICS_IGNORE_ERROR)) {}
    /** copy assignment*/
    Message& operator=(const Message& mess) HELICS_NOTHROW
    {
        if (mo!=HELICS_NULL_POINTER) {
            helicsMessageFree(mo);
        }
        mo = helicsMessageClone(mess.mo, HELICS_IGNORE_ERROR);
        return *this;
    }
#    ifdef HELICS_HAS_RVALUE_REFS
    /** copy constructor*/
    Message(Message&& mess) HELICS_NOTHROW: mo(mess.release()) {}
    /** copy assignment*/
    Message& operator=(Message&& mess) HELICS_NOTHROW
    {
        mo = mess.release();
        return *this;
    }
#endif
    ~Message()
    {
        if (mo != HELICS_NULL_POINTER) {
            helicsMessageFree(mo);
        }
    }
    /** cast to a helics_message object*/
    operator helics_message_object() const { return mo; }

    bool isValid() const { return (helicsMessageIsValid(mo) == helics_true); }

    const char* source() const { return helicsMessageGetSource(mo); }
    Message& source(const std::string& src)
    {
        helicsMessageSetSource(mo, src.c_str(), hThrowOnError());
        return *this;
    }

    const char* destination() const { return helicsMessageGetDestination(mo); }
    Message& destination(const std::string& dest)
    {
        helicsMessageSetDestination(mo, dest.c_str(), hThrowOnError());
        return *this;
    }
    const char* originalSource() const { return helicsMessageGetOriginalSource(mo); }
    Message& originalSource(const std::string& osrc)
    {
        helicsMessageSetOriginalSource(mo, osrc.c_str(), hThrowOnError());
        return *this;
    }
    const char* originalDestination() const { return helicsMessageGetOriginalDestination(mo); }
    Message& originalDestination(const std::string& odest)
    {
        helicsMessageSetOriginalDestination(mo, odest.c_str(), hThrowOnError());
        return *this;
    }

    int size() const { return helicsMessageGetRawDataSize(mo); }

    void resize(int newSize) { helicsMessageResize(mo, newSize, hThrowOnError()); }

    void reserve(int newSize) { helicsMessageReserve(mo, newSize, hThrowOnError()); }

    void* data() const { return helicsMessageGetRawDataPointer(mo); }

    Message& data(const void* raw, int size)
    {
        helicsMessageSetData(mo, raw, size, hThrowOnError());
        return *this;
    }

    Message& data(const std::string& str)
    {
        helicsMessageSetString(mo, str.c_str(), hThrowOnError());
        return *this;
    }

    Message& data(const char* str)
    {
        helicsMessageSetString(mo, str, hThrowOnError());
        return *this;
    }

     Message& append(const void* raw, int size)
    {
        helicsMessageAppendData(mo, raw, size, hThrowOnError());
        return *this;
    }

    Message& append(const std::string& str)
    {
        helicsMessageAppendData(mo, str.c_str(), static_cast<int>(str.size()), hThrowOnError());
        return *this;
    }

    const char* c_str() const { return helicsMessageGetString(mo); }

    helics_time time() const { return helicsMessageGetTime(mo); }

    Message& time(helics_time val)
    {
        helicsMessageSetTime(mo, val, hThrowOnError());
        return *this;
    }

    Message& setFlag(int flag,bool val) { helicsMessageSetFlagOption(mo, flag,val?helics_true:helics_false,hThrowOnError());
        return *this;
    }

    bool checkFlag(int flag) const { return (helicsMessageCheckFlag(mo, flag) == helics_true);
    }

    int messageID() const { return helicsMessageGetMessageID(mo); }

    Message& messageID(int newId)
    {
        helicsMessageSetMessageID(mo, newId, hThrowOnError());
        return *this;
    }

    helics_message_object release() { 
        helics_message_object mreturn = mo;
        mo = HELICS_NULL_POINTER;
        return mreturn;
    }

  private:
    helics_message_object mo;
};

class Endpoint {
  public:
    /** construct from a helics_endpoint object*/
    explicit Endpoint(helics_endpoint hep) HELICS_NOTHROW: ep(hep) {}
    /** default constructor*/
    Endpoint() HELICS_NOTHROW: ep(HELICS_NULL_POINTER) {}
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
    /** check if the input is valid */
    bool isValid() const { return (helicsEndpointIsValid(ep) == helics_true); }
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
    Message getMessage() { return Message(helicsEndpointGetMessageObject(ep)); }

     /** create a message object */
    Message createMessage()
    {
        return Message(helicsEndpointCreateMessageObject(ep, hThrowOnError()));
    }

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
        helicsEndpointSendEventRaw(ep,
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
        helicsEndpointSendEventRaw(ep,
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
    void sendMessage(const Message& message)
    {
        // returns helicsStatus
        helicsEndpointSendMessageObject(ep, message, hThrowOnError());
    }
#    ifdef HELICS_HAS_RVALUE_REFS
    /** send a message object
     */
    void sendMessage(Message&& message)
    {
        // returns helicsStatus
        helicsEndpointSendMessageObjectZeroCopy(ep, message.release(), hThrowOnError());
    }
#endif 
        /** send a message object
     */
    void sendMessageZeroCopy(Message& message)
    {
        // returns helicsStatus
        helicsEndpointSendMessageObjectZeroCopy(ep, static_cast<helics_message_object>(message), hThrowOnError());
        message.release();
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
    helics_endpoint ep;  //!< the underlying helics_endpoint object
};
}  // namespace helicscpp
#endif
