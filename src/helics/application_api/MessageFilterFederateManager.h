/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef _MESSAGE_FILTER_FEDERATE_MANAGER_
#define _MESSAGE_FILTER_FEDERATE_MANAGER_
#pragma once

#include "helics/core/core.h"
#include "identifierTypes.hpp"
#include "helics/common/simpleQueue.hpp"
#include "Message.h"
#include <utility> 
#include <cstdint>
#include <memory>
#include <map>
#include <functional>
#include <unordered_map>
#include <unordered_set>

namespace helics
{
	/** defining filter types  
	they can be source or destination filters*/
	enum class filter_type
	{
		source_filter,
		dest_filter
	};
	struct filter_info
	{
		std::string name;  //!< the name of the filter
		std::string endpoint; //!< the name of the endpoint the filter is targetting
		std::string inputType;  //!< the input type of the filter
		std::string outputType; //!< the output type of the filter
		filter_id_t id = 0;	
		Core::Handle handle;
		filter_type ftype = filter_type::source_filter;
		int callbackIndex=-1;

		filter_info(const std::string &filterName, const std::string &endpoint_, const std::string &inType, const std::string &outType) :name(filterName),
			endpoint(endpoint_), inputType(inType), outputType(outType)
		{
			if (outputType.empty())
			{
				outputType = inputType;
			}
		};
	};

	class MessageOperator;

	/** class handling the implementation details of a value Federate*/
	class MessageFilterFederateManager
	{
	public:
		/** constructor
		@param coreOb  a shared ptr to the core that is to be used
		@param id the identifier for the federate
		*/
		MessageFilterFederateManager(std::shared_ptr<Core> coreOb, Core::federate_id_t id);
		/** destructor */
		~MessageFilterFederateManager();

		/** define a filter interface on a source
		@details a source filter will be sent any packets that come from a particular source
		if multiple filters are defined on the same source, they will be placed in some order defined by the core
		@param[in] the name of the endpoint
		@param[in] the inputType which the source filter can receive
		*/
		filter_id_t registerSourceFilter(const std::string &filterName, const std::string &sourceEndpoint, const std::string &inputType, const std::string &outputType);
		/** define a filter interface for a destination
		@details a destination filter will be sent any packets that are going to a particular destination
		multiple filters are not allowed to specify the same destination
		@param[in] the name of the destination endpoint
		@param[in] the inputType which the destination filter can receive
		*/
		filter_id_t registerDestinationFilter(const std::string &filterName, const std::string &destEndpoint, const std::string &inputType, const std::string &outputType);


		/** check if any registered filters have packets*/
		bool hasMessageToFilter() const;
		/* check if a given filter endpoint has packets*/
		bool hasMessageToFilter(filter_id_t filter) const;
		/* get a packet for the specified filter
		@details the call is blocking
		@param[in] filter the specified filter
		@return a message View object
		*/
		std::unique_ptr<Message> getMessageToFilter(filter_id_t filter);


		/** update the time from oldTime to newTime
		@param[in] newTime the newTime of the federate
		@param[in] oldTime the oldTime of the federate
		*/
		void updateTime(Time newTime, Time oldTime);
		/** transition from Startup To the Initialize State*/
		void StartupToInitializeStateTransition();
		/** transition from initialize to executation State*/
		void InitializeToExecuteStateTransition();
		/** get the filter name
		@param[in] id the filter id
		@return a string with the federate name
		*/
		std::string getFilterName(filter_id_t id) const;
		/** get the name of a filter from its id
		@param[in] id the endpoint to query
		@return empty string if an invalid id is passed*/
		std::string getFilterEndpoint(filter_id_t id) const;

		/** get the input Type of an endpoint from its id
		@param[in] id the endpoint to query
		@return empty string if an invalid id is passed otherwise the string of the input type*/
		std::string getFilterInputType(filter_id_t id) const;
		/** get the output type of an endpoint from its id
		@param[in] id the endpoint to query
		@return empty string if an invalid id is passed otherwise the string of the output type*/
		std::string getFilterOutputType(filter_id_t id) const;

		/** get the id of a source filter from the name of the endpoint
		@param[in] filterName the name of the filter
		@return invalid_filter_id if name is not recognized otherwise returns the filter id*/
		filter_id_t getFilterId(const std::string &filterName) const;

		/** get the id of a source filter from the name of the endpoint
		@param[in] endpointName or filter name of the filter
		@return invalid_filter_id if name is not recognized otherwise returns the filter id*/
		filter_id_t getSourceFilterId(const std::string &endpointName) const;

		/** get the id of a destination filter from the name of the endpoint or filterName
		@param[in] endpointName the publication id
		@return invalid_filter_id if name is not recognized otherwise returns the filter id*/
		filter_id_t getDestFilterId(const std::string &endpointName) const;

		/** @brief register a global callback for all filters
		@details any incoming message to filter will trigger this callback
		the callback has a signature of void(filter_id_t,Time)  the filter_id of the triggering filter is sent as an
		argument to the callback
		@param[in] callback the function to trigger when a filter has an incoming message
		*/
		void registerFilterCallback(std::function<void(filter_id_t,Time)> callback);
		/** @brief register a callback for a specific filter
		@details if the specified filter has a message to process this notification callback is triggered
		the callback has a signature of void(Time)  intended as a notification system to the caller which would then call
		the getMessageToFilter function
		@param[in] id the identifier for the callback function
		@param[in] callback the function to trigger when the specified filter has an incoming message
		*/
		void registerFilterCallback(filter_id_t id, std::function<void(filter_id_t, Time)> callback);
		/** @brief register a callback for a set of filters
		@details if one of the specified filters has a message to process this notification callback is triggered
		the callback has a signature of void(Time)  intended as a notification system to the caller which would then call
		the getMessageToFilter function
		@param[in] ids the identifiers for the filters associated with this callback
		@param[in] callback the function to trigger when one of the specified filters has an incoming message
		*/
		void registerFilterCallback(const std::vector<filter_id_t> &ids, std::function<void(filter_id_t, Time)> callback);
		
		/** @brief register a global operator for all filters
		@details for time_agnostic federates only,  all other settings would trigger an error
		The MessageOperator gets called when there is a message to filter, There is no order or state to this
		message can come in in any order.
		@param[in] mo A shared_ptr to a message operator
		*/
		void registerMessageOperator(std::shared_ptr<MessageOperator> mo);
		/** @brief register a operator for the specified filter
		@details for time_agnostic federates only,  all other settings would trigger an error
		The MessageOperator gets called when there is a message to filter, There is no order or state to this
		message can come in in any order.
		@param[in] filter the identifier for the filter to trigger
		@param[in] mo A shared_ptr to a message operator
		*/
		void registerMessageOperator(filter_id_t filter, std::shared_ptr<MessageOperator> mo);
		/** @brief register a operator for the specified filters
		@details for time_agnostic federates only,  all other settings would trigger an error
		The MessageOperator gets called when there is a message to filter, There is no order or state to this
		message can come in in any order.
		@param[in] filters the identifier for the filter to trigger
		@param[in] mo A shared_ptr to a message operator
		*/
		void registerMessageOperator(const std::vector<filter_id_t> &filters, std::shared_ptr<MessageOperator> mo);
	private:

		std::unordered_map<std::string, filter_id_t> SourceFilterNames;  //!< map for source filters
		std::unordered_map<std::string, filter_id_t> DestFilterNames;		//!< map for destination filters
		std::vector<simpleQueue<std::unique_ptr<Message>>> filterQueues; //!< the storage for the message queues
		std::map<Core::Handle, filter_id_t> handleLookup; //!< map to lookup endpoints from core handles
		std::vector<filter_info> filters; //!< vector the filters
		std::vector<std::function<void(filter_id_t,Time)>> callbacks;
		Time CurrentTime; //!< the current simulation time
		std::vector<std::shared_ptr<MessageOperator>> operators; //!< storage for MessageOperators
		std::shared_ptr<Core> coreObject; //!< the pointer to the actual core
		Core::federate_id_t fedID;  //!< the federate ID of the object
		int allCallbackIndex = -1;  //!< the index of the all callback function
		mutable std::mutex filterLock;  //!< lock protecting the filter structure
	};

}
#endif