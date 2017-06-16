/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/

#ifndef _HELICS_MESSAGE_FEDERATE_API_
#define _HELICS_MESSAGE_FEDERATE_API_
#pragma once

#include "Federate.h"
#include "Message.h"
#include "identifierTypes.hpp"
#include <functional>
#include <vector>

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
    MessageFederate (const FederateInfo &fi);
	/**constructor taking a string with the required information
	@param[in] jsonString can be either a json file or a string containing json code
	*/
	MessageFederate(const std::string &jsonString);

	MessageFederate(MessageFederate &&mFed) noexcept;
    MessageFederate ();
	/** special constructor should only be used by child classes in constructor due to virtual inheritance*/
	MessageFederate(bool res);
public:
    /** destructor */
    ~MessageFederate ();

	MessageFederate &operator= (MessageFederate &&mFed) noexcept;
  protected:
    virtual void StartupToInitializeStateTransition () override;
    virtual void InitializeToExecuteStateTransition () override;
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
    virtual void registerInterfaces (const std::string &jsonString) override;

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
    void subscribe (endpoint_id_t endpoint, const std::string &name, const std::string &type);
    /** check if the federate has any outstanding messages*/
    bool hasMessage () const;
    /* check if a given endpoint has any unread messages*/
    bool hasMessage (endpoint_id_t id) const;

	/**
	* Returns the number of pending receives for the specified destination endpoint.
	*/
	uint64_t receiveCount(endpoint_id_t id) const;
	/**
	* Returns the number of pending receives for the specified destination endpoint.
	*/
	uint64_t receiveCount() const;
    /** receive a packet from a particular endpoint
    @param[in] endpoint the identifier for the endpoint
    @return a message object*/
    Message_view getMessage (endpoint_id_t endpoint);
    /** receive a communication message for any endpoint in the federate
    @details the return order will in order of endpoint creation then order of arrival
    all messages for the first endpoint, then all for the second, and so on
    @return a Message_view object containing the message data*/
    Message_view getMessage ();

    /** send a message
	@details send a message to a specific destination
	@param[in] source the source endpoint
	@param[in] dest a string naming the destination
	@param[in] data a buffer containing the data
	@param[in] len the length of the data buffer
	*/
	void sendMessage(endpoint_id_t source, const std::string &dest, const char *data, size_t len)
	{
		sendMessage(source, dest, data_view(data,len));
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
	void sendMessage(endpoint_id_t source, const std::string &dest, const char *data, size_t len,Time sendTime)
	{
		sendMessage(source, dest, data_view(data, len),sendTime);
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
	@param[in] message a view of the message
	*/
    void sendMessage (endpoint_id_t source, const Message_view &message);


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
    void registerEndpointCallback (std::function<void(endpoint_id_t, Time)> callback);
	/** register a callback for a specific endpoint
	@param[in] ep the endpoint to associate with the specified callback
	@param[in] callback the function to execute upon receipt of a message for the given endpoint
	*/
	void registerEndpointCallback (endpoint_id_t ep, std::function<void(endpoint_id_t, Time)> callback);
	/** register a callback for a set of specific endpoint
	@param[in] ep a vector of endpoints to associate with the specified callback
	@param[in] callback the function to execute upon receipt of a message for the given endpoint
	*/
	void registerEndpointCallback (const std::vector<endpoint_id_t> &ep, std::function<void(endpoint_id_t, Time)> callback);

  private:
    /** @brief PIMPL design pattern with the implementation details for the MessageFederate*/
    std::unique_ptr<MessageFederateManager> mfManager;
};
}
#endif