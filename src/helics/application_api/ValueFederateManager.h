/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef _VALUE_FEDERATE_MANAGER_H_
#define _VALUE_FEDERATE_MANAGER_H_
#pragma once

#include "Message.h"
#include "helics/core/core.h"
#include "identifierTypes.hpp"
#include <utility> 
#include <cstdint>
#include <memory>
#include <functional>
#include <unordered_map>
#include <map>
#include <mutex>

namespace helics
{
	/** forward declaration of Core*/
	class Core;

	/** structure used to contain information about a publication*/
	struct publication_info
	{
		std::string name; //!< publication name
		std::string type; //!< publication type
		std::string units; //!< publication units
		Core::Handle coreID; //!< Handle from the core
		publication_id_t id=0; //!< the id used as the identifier
		int size = -1;  //!< required size of a publication
		bool forward=true; 
		publication_info(const std::string &n_name, const std::string &n_type, const std::string &n_units) :
			name(n_name), type(n_type), units(n_units)
		{};
	};
	/** structure used to contain information about a subscription*/
	struct subscription_info
	{
		std::string name;  //!< subscription name
		std::string type; //!< subscription type
		std::string units; //!< subscription units
		std::string pubtype; //!< the listed type of the corresponding publication
		Core::Handle coreID; //!< Handle from the core
		subscription_id_t id=0;  //!< the id used as the identifier
		Time lastUpdate = Time(0.0); //!< the time the subscription was last updated
		Time lastQuery = Time(0.0); //!< the time the query was made
		int callbackIndex = -1;  //!< index for the callback
		subscription_info(const std::string &n_name, const std::string &n_type, const std::string &n_units) :
			name(n_name), type(n_type), units(n_units)
		{};
	};

	/** class handling the implementation details of a value Federate*/
	class ValueFederateManager
	{
	
	public:
		ValueFederateManager(std::shared_ptr<Core> &coreOb, Core::federate_id_t id);
		~ValueFederateManager();

		publication_id_t registerPublication(const std::string &name, const std::string &type, const std::string &units);
		/** register a subscription
		@details call is only valid in startup mode
		*/
		subscription_id_t registerRequiredSubscription(const std::string &name, const std::string &type, const std::string &units);


		/** register a subscription
		@details call is only valid in startup mode
		*/
		subscription_id_t registerOptionalSubscription(const std::string &name, const std::string &type, const std::string &units);
	
		/** add a shortcut for locating a subscription
		@details primarily for use in looking up an id from a different location
		creates a local shortcut for referring to a subscription which may have a long actual name
		@param[in] the subscription identifier
		@param[in] shortcutName the name of the shortcut
		*/
		void addSubscriptionShortcut(subscription_id_t subid, const std::string &shortcutName);

		/** set the default value for a subscription
		@details this is the value returned prior to any publications
		@param[in] id the subscription identifier
		@param[in] block the data block representing the default value
		*/
		void setDefaultValue(subscription_id_t id, data_view block);

		/** get a value as raw data block from the system
		@param[in] id the identifier for the subscription
		@return a constant data block
		*/
		data_view getValue(subscription_id_t id);

		/** publish a value*/
		void publish(publication_id_t id, data_view block);

		/** check if a given subscription has and update*/
		bool queryUpdate(subscription_id_t sub_id) const;
		/** get the time of the last update*/
		Time queryLastUpdate(subscription_id_t sub_id) const;

		/** update the time from oldTime to newTime
		@param[in] newTime the newTime of the federate
		@param[in] oldTime the oldTime of the federate
		*/
		void updateTime(Time newTime, Time oldTime);
		/** transition from Startup To the Initialize State*/
		void StartupToInitializeStateTransition();
		/** transition from initialize to execution State*/
		void InitializeToExecuteStateTransition();

		
		/** get a list of all the values that have been updated since the last call
		@return a vector of subscription_ids with all the values that have not been retrieved since updated
		*/
		std::vector<subscription_id_t>  queryUpdates();

		/** get the name of a subscription from its id
		@return empty string if an invalid id is passed*/
		std::string getSubscriptionName(subscription_id_t) const;
		/** get the id of a subscription
		@return ivalid_subscription_id if name is not a recognized*/
		subscription_id_t getSubscriptionId(const std::string &name) const;

		/** get the name of a publication from its id
		@return empty string if an invalid id is passed*/
		std::string getPublicationName(publication_id_t) const;

		/** get the id of a registered publication from its id
		@param[in] name the publication id
		@return ivalid_publication_id if name is not recognized otherwise returns the publication_id*/
		publication_id_t getPublicationId(const std::string &name) const;

		/** get the units of a subscriptions from its id
		@param[in] id the subscription id to query
		@return the name or empty string on unrecognized id*/
		std::string getSubscriptionUnits(subscription_id_t id) const;

		/** get the units of a publication from its id
		@param[in] id the publication id to query
		@return the units or empty string on unrecognized id*/
		std::string getPublicationUnits(publication_id_t id) const;

		/** get the type of a subscription from its id
		@param[in] id the subscription id to query
		@return the type or empty string on unrecognized id*/
		std::string getSubscriptionType(subscription_id_t id) const;

		/** get the type of a publication from its id
		@param[in] id the publication id to query
		@return the type or empty string on unrecognized id*/
		std::string getPublicationType(publication_id_t id) const;

		/** get the type of a publication from its subscription
		@param[in] id the subscription id to query
		@return the type or empty string on unrecognized id*/
		std::string getPublicationType(subscription_id_t id) const;

		/** register a callback function to call when any subscribed value is updated
		@details there can only be one generic callback
		@param[in] callback the function to call
		*/
		void registerCallback(std::function<void(subscription_id_t,Time)>  callback);
		/** register a callback function to call when the specified subscription is updated
		@param[in] id  the id to register the callback for
		@param[in] callback the function to call
		*/
		void registerCallback(subscription_id_t id, std::function<void(subscription_id_t, Time)> callback);
		/** register a callback function to call when the specified subscription is updated
		@param[in] ids  the set of ids to register the callback for
		@param[in] callback the function to call
		*/
		void registerCallback(const std::vector<subscription_id_t> &ids, std::function<void(subscription_id_t, Time)> callback);
	private:
		std::unordered_map<std::string, publication_id_t> publicationNames; //!< map of the publication names
		std::unordered_map<std::string, subscription_id_t> subscriptionNames; //!< map of the subscription names
		std::map<Core::Handle, subscription_id_t> handleLookup;  //!< a map to translate the Handles to subscription_id_t
		std::vector<subscription_info> subs;	//!< all the subscription info
		std::vector<publication_info> pubs;	//!< all the publication info
		std::vector<std::function<void(subscription_id_t,Time)>> callbacks;	//!< the all callback function
		std::vector<data_view> lastData;	//!< the last data to arrive
		Time CurrentTime=Time(-1.0); //!< the current simulation time
		std::shared_ptr<Core> coreObject; //!< the pointer to the actual core
		Core::federate_id_t fedID;  //!< the federation ID from the core API
		int allCallbackIndex = -1;	//!< index of the allCallback function
		
		mutable std::mutex subscription_mutex; //!< mutex protecting the subscription information
		mutable std::mutex publication_mutex; //!< mutex protecting the publication information
	private:
		void getUpdateFromCore(Core::Handle handle);
	};

} //namespace helics
#endif