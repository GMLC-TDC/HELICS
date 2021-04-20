/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "Federate.hpp"
#include "data_view.hpp"

#include <functional>
#include <memory>
#include <string>

namespace helics {
class MessageFederateManager;
class Endpoint;
/** class defining the block communication based interface */
class HELICS_CXX_EXPORT MessageFederate:
    public virtual Federate  // using virtual inheritance to allow combination federate
{
  public:
    /**constructor taking a federate information structure and using the default core
    @param fedName the name of the messageFederate, can be left empty to use a default or one from
    fi
    @param fi  a federate information structure
    */
    MessageFederate(const std::string& fedName, const FederateInfo& fi);
    /**constructor taking a core and a federate information structure, core information in fi is
    ignored
    @param fedName the name of the messageFederate, can be left empty to use a default or one from
    fi
    @param core a shared ptr to a core to join
    @param fi  a federate information structure
    */
    MessageFederate(const std::string& fedName,
                    const std::shared_ptr<Core>& core,
                    const FederateInfo& fi = FederateInfo{});

    /**constructor taking a core and a federate information structure, core information in fi is
    ignored
    @param fedName the name of the messageFederate, can be left empty to use a default or one from
    fi
    @param core a CoreApp object representing the core to connect to
    @param fi  a federate information structure
    */
    MessageFederate(const std::string& fedName,
                    CoreApp& core,
                    const FederateInfo& fi = FederateInfo{});

    /**constructor taking a string with the required information
    @param name the name of the federate, can be empty to get name from config
    @param configString can be either a JSON file, TOML file or a string containing JSON code
    */
    MessageFederate(const std::string& name, const std::string& configString);
    /**constructor taking a string with the required information
    @param configString can be either a JSON file, TOML file or a string containing JSON code, or
    command line arguments it can also be just the federate name
    */
    explicit MessageFederate(const std::string& configString);

    /**constructor taking a string as const char * with the required information
    @details; this constructor is to deconflict with the bool overload which can be triggered if a
    string literal is passed on some platforms
    @param configString can be either a JSON file, TOML file or a string containing JSON code, or
    command line arguments it can also be just the federate name
    */
    explicit MessageFederate(const char* configString);
    /** move constructor*/
    MessageFederate(MessageFederate&& mFed) noexcept;
    /** delete copy constructor*/
    MessageFederate(const MessageFederate& mFed) = delete;
    /** default constructor*/
    MessageFederate();
    /** special constructor should only be used by child classes in constructor due to virtual
     * inheritance*/
    explicit MessageFederate(bool res);
    // copy constructor and copy assignment are disabled
    /** destructor */
    virtual ~MessageFederate();
    /** move assignment*/
    MessageFederate& operator=(MessageFederate&& mFed) noexcept;
    /** delete copy assignment*/
    MessageFederate& operator=(const MessageFederate& mFed) = delete;

  protected:
    virtual void startupToInitializeStateTransition() override;
    virtual void initializeToExecuteStateTransition(iteration_result result) override;
    virtual void updateTime(Time newTime, Time oldTime) override;
    virtual std::string localQuery(const std::string& queryStr) const override;

  public:
    /** register an endpoint
    @details call is only valid in startup mode
    @param eptName the name of the endpoint
    @param type the defined type of the interface for endpoint checking if requested
    */
    Endpoint& registerEndpoint(const std::string& eptName = std::string(),
                               const std::string& type = std::string());

    /** register an endpoint directly without prepending the federate name
    @details call is only valid in startup mode
    @param eptName the name of the endpoint
    @param type the defined type of the interface for endpoint checking if requested
    @return a Reference to an Endpoint Object
    */
    Endpoint& registerGlobalEndpoint(const std::string& eptName,
                                     const std::string& type = std::string());

    /** register an indexed Endpoint
    @details register a global endpoint as part of a 1D array of endpoints
    @param eptName the name of the endpoint array
    @param index1 the index into a 1 dimensional array of endpoints
    @param type the optional type on the endpoint
    */
    Endpoint& registerIndexedEndpoint(const std::string& eptName,
                                      int index1,
                                      const std::string& type = std::string())
    {
        return registerGlobalEndpoint(eptName + '_' + std::to_string(index1), type);
    }

    virtual void registerInterfaces(const std::string& configString) override;

    /** register a set Message interfaces
    @details call is only valid in startup mode it is a protected call to add an
    @param configString  the location of the file(TOML or JSON) or JSON String to load to generate
    the interfaces
    */
    void registerMessageInterfaces(const std::string& configString);

  private:
    /** register a set Message interfaces from JSON
 @details call is only valid in startup mode it is a protected call to add an
 @param jsonString  the location of the file or JSON String to load to generate the interfaces
 */
    void registerMessageInterfacesJson(const std::string& jsonString);

    /** register a set Message interfaces using a toml file
  @details call is only valid in startup mode it is a protected call to add an
  @param tomlString  the location of the TOML to load to generate the interfaces
  */
    void registerMessageInterfacesToml(const std::string& tomlString);

  public:
    /** give the core a hint for known communication paths
    @details the function will generate an error in the core if a communication path is not present
    once the simulation is initialized
    @param localEndpoint the local endpoint of a known communication pair
    @param remoteEndpoint of a communication pair
    */
    void registerKnownCommunicationPath(const Endpoint& localEndpoint,
                                        const std::string& remoteEndpoint);
    /** subscribe to valueFederate publication to be delivered as Messages to the given endpoint
    @param ept the specified endpoint to deliver the values
    @param key the name of the publication to subscribe
    */
    void subscribe(const Endpoint& ept, const std::string& key);
    /** check if the federate has any outstanding messages*/
    bool hasMessage() const;
    /* check if a given endpoint has any unread messages*/
    bool hasMessage(const Endpoint& ept) const;

    /**
     * Returns the number of pending receives for the specified destination endpoint.
     */
    uint64_t pendingMessages(const Endpoint& ept) const;
    /**
     * Returns the number of pending receives for all endpoints.
     */
    uint64_t pendingMessages() const;
    /** receive a packet from a particular endpoint
    @param ept the identifier for the endpoint
    @return a message object*/
    std::unique_ptr<Message> getMessage(const Endpoint& ept);
    /** receive a communication message for any endpoint in the federate
    @details the return order will be in order of endpoint creation then order of arrival
    all messages for the first endpoint, then all for the second, and so on
    @return a unique_ptr to a Message object containing the message data*/
    std::unique_ptr<Message> getMessage();

    /** send a message
    @details send a message to a specific destination
    @param source the source endpoint
    @param dest a string naming the destination
    @param data a buffer containing the data
    @param dataLength the length of the data buffer
    */
    void sendMessage(const Endpoint& source,
                     const std::string& dest,
                     const char* data,
                     size_t dataLength)
    {
        sendMessage(source, dest, data_view(data, dataLength));
    }
    /** send a message
    @details send a message to a specific destination
    @param source the source endpoint
    @param dest a string naming the destination
    @param message a data_view of the message
    */
    void sendMessage(const Endpoint& source, const std::string& dest, const data_view& message);
    /** send an event message at a particular time
    @details send a message to a specific destination
    @param source the source endpoint
    @param dest a string naming the destination
    @param data a buffer containing the data
    @param dataLength the length of the data buffer
    @param sendTime the time the message should be sent
    */
    void sendMessage(const Endpoint& source,
                     const std::string& dest,
                     const char* data,
                     size_t dataLength,
                     Time sendTime)
    {
        sendMessage(source, dest, data_view(data, dataLength), sendTime);
    }
    /** send an event message at a particular time
    @details send a message to a specific destination
    @param source the source endpoint
    @param dest a string naming the destination
    @param message a data_view of the message data to send
    @param sendTime the time the message should be sent
    */
    void sendMessage(const Endpoint& source,
                     const std::string& dest,
                     const data_view& message,
                     Time sendTime);
    /** send an event message at a particular time
    @details send a message to a specific destination
    @param source the source endpoint
    @param message a pointer to the message
    */
    void sendMessage(const Endpoint& source, std::unique_ptr<Message> message);

    /** send an event message at a particular time
    @details send a message to a specific destination
    @param source the source endpoint
    @param message a message object
    */
    void sendMessage(const Endpoint& source, const Message& message);

    /** get an endpoint by its name
    @param name the Endpoint
    @return an Endpoint*/
    Endpoint& getEndpoint(const std::string& name) const;

    /** get an Endpoint from an index
    @param index the index of the endpoint to retrieve index is 0 based
    @return an Endpoint*/
    Endpoint& getEndpoint(int index) const;

    /** register a callback for all endpoints
    @param callback the function to execute upon receipt of a message for any endpoint
    */
    void setMessageNotificationCallback(const std::function<void(Endpoint&, Time)>& callback);
    /** register a callback for a specific endpoint
    @param ept the endpoint to associate with the specified callback
    @param callback the function to execute upon receipt of a message for the given endpoint
    */
    void setMessageNotificationCallback(const Endpoint& ept,
                                        const std::function<void(Endpoint&, Time)>& callback);

    /** add a named filter to an endpoint for all message coming from the endpoint*/
    void addSourceFilter(const Endpoint& ept, const std::string& filterName);
    /** add a named filter to an endpoint for all message going to the endpoint*/
    void addDestinationFilter(const Endpoint& ept, const std::string& filterName);

    virtual void disconnect() override;

    /**get the number of registered endpoints*/
    int getEndpointCount() const;

  private:
    /** @brief PIMPL design pattern with the implementation details for the MessageFederate*/
    std::unique_ptr<MessageFederateManager> mfManager;
};
}  // namespace helics
