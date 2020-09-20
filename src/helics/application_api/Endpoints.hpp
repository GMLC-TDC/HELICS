/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "Federate.hpp"
#include "data_view.hpp"

#include <memory>
#include <string>
#include <type_traits>
#include <utility>

namespace helics {
class MessageFederate;

/** class to manage an endpoint */
class HELICS_CXX_EXPORT Endpoint: public Interface {
  private:
    MessageFederate* fed{nullptr};  //!< the MessageFederate to interact with
    int referenceIndex{-1};  //!< an index used for callback lookup
    void* dataReference{nullptr};  //!< pointer to a piece of containing data
    bool disableAssign{false};  //!< disable assignment for the object
    std::string defDest;  //!< storage for a default destination
  public:
    /** default constructor*/
    Endpoint() = default;
    /**/
    // constructor used by messageFederateManager
    Endpoint(MessageFederate* mFed, const std::string& name, InterfaceHandle id, void* data);

    Endpoint(MessageFederate* mFed,
             const std::string& name,
             const std::string& type = std::string());

    template<class FedPtr>
    Endpoint(FedPtr& mFed, const std::string& name, const std::string& type = std::string()):
        Endpoint(std::addressof(*mFed), name, type)
    {
        static_assert(
            std::is_base_of<MessageFederate, std::remove_reference_t<decltype(*mFed)>>::value,
            "first argument must be a pointer to a MessageFederate");
    }
    /**constructor to build an endpoint object
    @param locality visibility of the endpoint either global or local
    @param mFed  the MessageFederate to use
    @param name the name of the endpoint
    @param type a named type associated with the endpoint
    */
    Endpoint(interface_visibility locality,
             MessageFederate* mFed,
             const std::string& name,
             const std::string& type = std::string());
    /**constructor to build an endpoint object
    @param locality visibility of the endpoint either global or local
    @param mFed  the MessageFederate to use
    @param name the name of the endpoint
    @param type a named type associated with the endpoint
    */
    template<class FedPtr>
    Endpoint(interface_visibility locality,
             FedPtr& mFed,
             const std::string& name,
             const std::string& type = std::string()):
        Endpoint(locality, std::addressof(*mFed), name, type)
    {
        static_assert(
            std::is_base_of<MessageFederate, std::remove_reference_t<decltype(*mFed)>>::value,
            "second argument must be a pointer to a MessageFederate");
    }
    const std::string& getType() { return getExtractionType(); }
    /** send a data block and length
    @param dest string name of the destination
    @param data pointer to data location
    @param data_size the length of the data
    */
    void send(const char* data, size_t data_size) const;

    /** subscribe the endpoint to a particular publication*/
    void subscribe(const std::string& key);

    /** send a data block and length
    @param dest string name of the destination
    @param data pointer to data location
    @param data_size the length of the data
    @param sendTime the time to send the message
    */
    void sendTo(std::string_view dest, const char* data, size_t data_size) const;
    /** send a data block and length
    @param sendTime the time to send the message
    @param data pointer to data location
    @param data_size the length of the data
    */
    void sendAt(Time sendTime, const char* data, size_t data_size) const;

    /** send a data_view
    @details a data view can convert from many different formats so this function should
    be catching many of the common use cases
    @param dest string name of the destination
    @param data the information to send
    */
    void sendTo(std::string_view dest, const data_view& data) const
    {
        sendTo(dest, data.data(), data.size());
    }
    /** send a data_view
    @details a data view can convert from many different formats so this function should
    be catching many of the common use cases
    @param dest string name of the destination
    @param data data representation to send
    @param sendTime  the time the message should be sent
    */
    void sendToAt(std::string_view dest, Time sendTime, const data_view& data) const
    {
        sendToAt(dest, sendTime, data.data(), data.size());
    }
    /** send a data block and length to a destination at a particular time
   @param dest string name of the destination
   @param data pointer to data location
   @param data_size the length of the data
   @param sendTime the time to send the message
   */
    void sendToAt(std::string_view dest, Time sendTime, const char* data, size_t data_size) const;

    /** send a data block and length to the target destination
    @param data pointer to data location
    @param data_size the length of the data
    */
    void send(const void* data, size_t data_size) const;

    /** send a data_view to the target destination
    @details a data view can convert from many different formats so this function should
    be catching many of the common use cases
    @param data the information to send
    */
    void send(const data_view& data) const { send(data.data(), data.size()); }
    /** send a data_view to the specified target destination
    @details a data view can convert from many different formats so this function should
    be catching many of the common use cases
    @param data a representation to send
    @param sendTime  the time the message should be sent
    */
    void sendAt(Time sendTime, const data_view& data) const
    {
        sendAt(sendTime, data.data(), data.size());
    }
    /** send a pointer to a message object*/
    void send(std::unique_ptr<Message> mess) const;

    /** send a message object
    @details this is to send a pre-built message
    @param mess a reference to an actual message object
    */
    void send(const Message& mess) const { send(std::make_unique<Message>(mess)); }

    /** get an available message if there is no message the returned object is empty*/
    std::unique_ptr<Message> getMessage() const;
    /** check if there is a message available*/
    bool hasMessage() const;
    /** check if there is a message available*/
    std::uint64_t pendingMessages() const;
    /** register a callback for an update notification
    @details the callback is called in the just before the time request function returns
    @param callback a function with signature void(endpoint_id_t, Time)
    time is the time the value was updated  This callback is a notification callback and doesn't
    return the value
    */
    void setCallback(const std::function<void(const Endpoint&, Time)>& callback);

    /** add a named filter to an endpoint for all message coming from the endpoint*/
    void addSourceFilter(const std::string& filterName);
    /** add a named filter to an endpoint for all message going to the endpoint*/
    void addDestinationFilter(const std::string& filterName);
    /** set a target destination for unspecified messages*/
    void setDefaultDestination(std::string_view target);
    /** get the target destination for the endpoint TODO(PT):: make this work*/
    const std::string& getDefaultDestination() const;

    virtual const std::string& getDisplayName() const override { return getName(); }

  private:
    friend class MessageFederateManager;
};
}  // namespace helics
