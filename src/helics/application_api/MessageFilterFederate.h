/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef _MESSAGE_FILTER_FEDERATE_H_
#define _MESSAGE_FILTER_FEDERATE_H_
#pragma once

#include "MessageFederate.h"

namespace helics
{
/** @brief PIMPL design pattern with the implementation details*/
class MessageFilterFederateManager;
/** class for use with time_agnostic functions on messages*/
class MessageOperator;
/** class defining the packet based interface */
class MessageFilterFederate : public virtual MessageFederate
{
  public:
    MessageFilterFederate ();
    /**constructor taking a federate information structure and using the default core
    @param[in] fi  a federate information structure
    */
    MessageFilterFederate (const FederateInfo &fi);
	/**constructor taking a core and a federate information structure, sore information in fi is ignored
	@param[in] core a shared ptr to a core to join
	@param[in] fi  a federate information structure
	*/
	MessageFilterFederate(std::shared_ptr<Core> core, const FederateInfo &fi);
    /**constructor taking a string with the required information
    @param[in] jsonString can be either a json file or a string containing json code
    */
    MessageFilterFederate (const std::string &jsonString);

    MessageFilterFederate (MessageFilterFederate &&fed) noexcept;
    /** destructor */
    ~MessageFilterFederate ();

    MessageFilterFederate &operator= (MessageFilterFederate &&fed) noexcept;

  protected:
    virtual void StartupToInitializeStateTransition () override;
    virtual void InitializeToExecuteStateTransition () override;
    virtual void updateTime (Time newTime, Time oldTime) override;

  public:
    virtual void registerInterfaces (const std::string &jsonString) override;

    /** define a filter interface on a source
    @details a source filter will be sent any packets that come from a particular source
    if multiple filters are defined on the same source, they will be placed in some order defined by the core
    @param[in] the name of the endpoint
    @param[in] the inputType which the source filter can receive
    */
    filter_id_t registerSourceFilter (const std::string &filterName,
                                      const std::string &sourceEndpoint,
                                      const std::string &inputType = "",
                                      const std::string &outputType = "");
    /** define a filter interface for a destination
    @details a destination filter will be sent any packets that are going to a particular destination
    multiple filters are not allowed to specify the same destination
    @param[in] the name of the destination endpoint
    @param[in] the inputType which the destination filter can receive
    */
    filter_id_t registerDestinationFilter (const std::string &filterName,
                                           const std::string &destEndpoint,
                                           const std::string &inputType = "",
                                           const std::string &outputType = "");
    /** define a filter interface on a source
    @details a source filter will be sent any packets that come from a particular source
    if multiple filters are defined on the same source, they will be placed in some order defined by the core
    @param[in] the name of the endpoint
    @param[in] the inputType which the source filter can receive
    */
    filter_id_t registerSourceFilter (const std::string &sourceEndpoint);
    /** define a filter interface for a destination
    @details a destination filter will be sent any packets that are going to a particular destination
    multiple filters are not allowed to specify the same destination
    @param[in] the name of the destination endpoint
    @param[in] the inputType which the destination filter can receive
    */
    filter_id_t registerDestinationFilter (const std::string &destEndpoint);

    /** check if any registered filters have packets*/
    bool hasMessageToFilter () const;
    /* check if a given filter endpoint has packets*/
    bool hasMessageToFilter (filter_id_t filter) const;
    /* get a packet for the specified filter
    @details the call is blocking
    @param[in] filter the specified filter
    @return a pointer to the message*/
    std::unique_ptr<Message> getMessageToFilter (filter_id_t filter);

    /** get the name of a filer
    @param[in] id the filter to query
    @return empty string if an invalid id is passed*/
    std::string getFilterName (filter_id_t id) const;

    /** get the name of the endpoint that a filter is associated with
    @param[in] id the filter to query
    @return empty string if an invalid id is passed*/
    std::string getFilterEndpoint (filter_id_t id) const;

    /** get the input type of a filter from its id
    @param[in] id the endpoint to query
    @return empty string if an invalid id is passed*/
    std::string getFilterInputType (filter_id_t id) const;

    /** get the output type of a filter from its id
    @param[in] id the endpoint to query
    @return empty string if an invalid id is passed*/
    std::string getFilterOutputType (filter_id_t id) const;
    /** get the id of a source filter from the name of the endpoint
    @param[in] filterName the name of the filter
    @return invalid_filter_id if name is not recognized otherwise returns the filter id*/
    filter_id_t getFilterId (const std::string &filterName) const;
    /** get the id of a source filter from the name of the filter
    @param[in] filterName the publication id
    @return invalid_filter_id if name is not recognized otherwise returns the filter id*/
    filter_id_t getSourceFilterId (const std::string &filterName) const;

    /** get the id of a destination filter from the name of the endpoint
    @param[in] filterName the publication id
    @return invalid_filter_id if name is not recognized otherwise returns the filter id*/
    filter_id_t getDestFilterId (const std::string &filterName) const;

    /** @brief register a global callback
    @details any incoming message to filter will trigger this callback
    the callback has a signature of void(filter_id_t,Time)  the filter_id of the triggering filter is sent as an
    argument to the callback
    @param[in] callbackFunc the function to trigger when a filter has an incoming message
    */
    void registerFilterCallback (std::function<void(filter_id_t, Time)> callback);
    /** @brief register a callback for a specific filter
    @details if the specified filter has a message to process this notification callback is triggered
    the callback has a signature of void(Time)  intended as a notification system to the caller which would then
    call the getMessageToFilter function
    @param[in] id the identifer for the filter
    @param[in] callback the function to trigger when the specified filter has an incoming message
    */
    void registerFilterCallback (filter_id_t id, std::function<void(filter_id_t, Time)> callback);
    /** @brief register a callback for a set of filters
    @details if one of the specified filters has a message to process this notification callback is triggered
    the callback has a signature of void(Time)  intended as a notification system to the caller which would then
    call the getMessageToFilter function
    @param[in] filters the set of filters to use the callback for
    @param[in] callback the function to trigger when one of the specified filters has an incoming message
    */
    void registerFilterCallback (const std::vector<filter_id_t> &filters,
                                 std::function<void(filter_id_t, Time)> callback);
    /** @brief register a global operator for all filters
    @details for time_agnostic federates only,  all other settings would trigger an error
    The MessageOperator gets called when there is a message to filter, There is no order or state to this
    message can come in in any order.
    @param[in] mo A shared_ptr to a message operator
    */
    void registerMessageOperator (std::shared_ptr<MessageOperator> mo);
    /** @brief register a operator for the specified filter
    @details for time_agnostic federates only,  all other settings would trigger an error
    The MessageOperator gets called when there is a message to filter, There is no order or state to this
    messages can come in any order.
    @param[in] filter the identifier for the filter to trigger
    @param[in] mo A shared_ptr to a message operator
    */
    void registerMessageOperator (filter_id_t filter, std::shared_ptr<MessageOperator> mo);
    /** @brief register a operator for the specified filters
    @details for time_agnostic federates only,  all other settings would trigger an error
    The MessageOperator gets called when there is a message to filter, There is no order or state to this
    message can come in in any order.
    @param[in] filters the identifier for the filter to trigger
    @param[in] mo A shared_ptr to a message operator
    */
    void registerMessageOperator (const std::vector<filter_id_t> &filters, std::shared_ptr<MessageOperator> mo);

  private:
    /** @brief PIMPL design pattern with the implementation details*/
    std::unique_ptr<MessageFilterFederateManager> filterManager;
    std::atomic<int> filterCount{0};
};
} //namespace helics
#endif