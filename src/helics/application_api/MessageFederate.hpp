/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once

#include "Federate.hpp"
#include "data_view.hpp"
#include <functional>

namespace helics
{
class MessageFederateManager;
/** class defining the block communication based interface */
class MessageFederate : public virtual Federate  // using virtual inheritance to allow combination federate
{
  public:
    /**constructor taking a federate information structure and using the default core
    @param[in] fi  a federate information structure
    */
    explicit MessageFederate (const std::string &name, const FederateInfo &fi);
    /**constructor taking a core and a federate information structure, sore information in fi is ignored
    @param[in] core a shared ptr to a core to join
    @param[in] fi  a federate information structure
    */
    MessageFederate (const std::string &name, const std::shared_ptr<Core> &core, const FederateInfo &fi);
    /**constructor taking a string with the required information
    @param[in] configString can be either a json file, toml file or a string containing json code
    */
    explicit MessageFederate (const std::string &configString);
    /**constructor taking a string with the required information
    @param[in] name the name of the federate
    @param[in] configString can be either a json file, toml file or a string containing json code
    */
    MessageFederate (const std::string &name, const std::string &configString);
    /** move constructor*/
    MessageFederate (MessageFederate &&mFed) noexcept;
    /** default constructor*/
    MessageFederate ();
    /** special constructor should only be used by child classes in constructor due to virtual inheritance*/
    explicit MessageFederate (bool res);
    // copy constructor and copy assignment are disabled
  public:
    /** destructor */
    ~MessageFederate ();
    /** move assignment*/
    MessageFederate &operator= (MessageFederate &&mFed) noexcept;

  protected:
    virtual void startupToInitializeStateTransition () override;
    virtual void initializeToExecuteStateTransition () override;
    virtual void updateTime (Time newTime, Time oldTime) override;

  public:
    /** register an endpoint
    @details call is only valid in startup mode
    @param[in] name the name of the endpoint
    @param[in] type the defined type of the interface for endpoint checking if requested
    */
    endpoint_id_t registerEndpoint (const std::string &name, const std::string &type = "");

    /** register an endpoint directly without prepending the federate name
    @details call is only valid in startup mode
    @param[in] name the name of the endpoint
    @param[in] type the defined type of the interface for endpoint checking if requested
    */
    endpoint_id_t registerGlobalEndpoint (const std::string &name, const std::string &type = "");
    virtual void registerInterfaces (const std::string &configString) override;

    /** register a set Message interfaces
    @details call is only valid in startup mode it is a protected call to add an
    @param[in] configString  the location of the file or json String to load to generate the interfaces
    */
    void registerMessageInterfaces (const std::string &configString);

  private:
    /** register a set Message interfaces
 @details call is only valid in startup mode it is a protected call to add an
 @param[in] configString  the location of the file or json String to load to generate the interfaces
 */
    void registerMessageInterfacesJson (const std::string &jsonString);

	 /** register a set Message interfaces
   @details call is only valid in startup mode it is a protected call to add an
   @param[in] configString  the location of the toml to load to generate the interfaces
   */
    void registerMessageInterfacesToml (const std::string &tomlString);

  public:

    /** give the core a hint for known communication paths
    @details the function will generate an error in the core if a communication path is not present once the
    simulation is initialized
    @param[in] localEndpoint the local endpoint of a known communication pair
    @param[in] remoteEndpoint of a communication pair
    */
    void registerKnownCommunicationPath (endpoint_id_t localEndpoint, const std::string &remoteEndpoint);
    /** subscribe to valueFederate publication to be delivered as Messages to the given endpoint
    @param[in] endpoint the specified endpoint to deliver the values
    @param[in] name the name of the publication to subscribe
    @param[in] type the type of publication
    */
    void subscribe (endpoint_id_t endpoint, const std::string &name);
    /** check if the federate has any outstanding messages*/
    bool hasMessage () const;
    /* check if a given endpoint has any unread messages*/
    bool hasMessage (endpoint_id_t id) const;

    /**
     * Returns the number of pending receives for the specified destination endpoint.
     */
    uint64_t pendingMessages (endpoint_id_t id) const;
    /**
     * Returns the number of pending receives for all endpoints.
     */
    uint64_t pendingMessages () const;
    /** receive a packet from a particular endpoint
    @param[in] endpoint the identifier for the endpoint
    @return a message object*/
    std::unique_ptr<Message> getMessage (endpoint_id_t endpoint);
    /** receive a communication message for any endpoint in the federate
    @details the return order will be in order of endpoint creation then order of arrival
    all messages for the first endpoint, then all for the second, and so on
    @return a unique_ptr to a Message object containing the message data*/
    std::unique_ptr<Message> getMessage ();

    /** send a message
    @details send a message to a specific destination
    @param[in] source the source endpoint
    @param[in] dest a string naming the destination
    @param[in] data a buffer containing the data
    @param[in] len the length of the data buffer
    */
    void sendMessage (endpoint_id_t source, const std::string &dest, const char *data, size_t len)
    {
        sendMessage (source, dest, data_view (data, len));
    }
    /** send a message
    @details send a message to a specific destination
    @param[in] source the source endpoint
    @param[in] dest a string naming the destination
    @param[in] message a data_view of the message
    */
    void sendMessage (endpoint_id_t source, const std::string &dest, const data_view &message);
    /** send an event message at a particular time
    @details send a message to a specific destination
    @param[in] source the source endpoint
    @param[in] dest a string naming the destination
    @param[in] data a buffer containing the data
    @param[in] len the length of the data buffer
    @param[in] Time the time the message should be sent
    */
    void sendMessage (endpoint_id_t source, const std::string &dest, const char *data, size_t len, Time sendTime)
    {
        sendMessage (source, dest, data_view (data, len), sendTime);
    }
    /** send an event message at a particular time
    @details send a message to a specific destination
    @param[in] source the source endpoint
    @param[in] dest a string naming the destination
    @param[in] message a data_view of the message data to send
    @param[in] Time the time the message should be sent
    */
    void sendMessage (endpoint_id_t source, const std::string &dest, const data_view &message, Time sendTime);
    /** send an event message at a particular time
    @details send a message to a specific destination
    @param[in] source the source endpoint
    @param[in] message a pointer to the message
    */
    void sendMessage (endpoint_id_t source, std::unique_ptr<Message> message);

    /** send an event message at a particular time
    @details send a message to a specific destination
    @param[in] source the source endpoint
    @param[in] message a message object
    */
    void sendMessage (endpoint_id_t source, const Message &message);

    /** get the name of an endpoint from its id
    @param[in] id the endpoint to query
    @return empty string if an invalid id is passed*/
    std::string getEndpointName (endpoint_id_t id) const;

    /** get the id of a registered publication from its id
    @param[in] name the publication id
    @return ivalid_publication_id if name is not recognized otherwise returns the publication_id*/
    endpoint_id_t getEndpointId (const std::string &name) const;

    /** get the type associated with an endpoint
    @param[in] ep the endpoint identifier
    @return a string containing the endpoint type
    */
    std::string getEndpointType (endpoint_id_t ep);

    /** register a callback for all endpoints
    @param[in] callback the function to execute upon receipt of a message for any endpoint
    */
    void registerEndpointCallback (const std::function<void(endpoint_id_t, Time)> &callback);
    /** register a callback for a specific endpoint
    @param[in] ep the endpoint to associate with the specified callback
    @param[in] callback the function to execute upon receipt of a message for the given endpoint
    */
    void registerEndpointCallback (endpoint_id_t ep, const std::function<void(endpoint_id_t, Time)> &callback);
    /** register a callback for a set of specific endpoint
    @param[in] ep a vector of endpoints to associate with the specified callback
    @param[in] callback the function to execute upon receipt of a message for the given endpoint
    */
    void registerEndpointCallback (const std::vector<endpoint_id_t> &ep,
                                   const std::function<void(endpoint_id_t, Time)> &callback);

    virtual void disconnect () override;

    /**get the number of registered endpoints*/
    int getEndpointCount () const;

  private:
    /** @brief PIMPL design pattern with the implementation details for the MessageFederate*/
    std::unique_ptr<MessageFederateManager> mfManager;
};
}  // namespace helics
