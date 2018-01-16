
/*
Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/

#ifndef HELICS_API_OBJECTS_H_
#define HELICS_API_OBJECTS_H_
#pragma once

#include <memory>
#include "../api-data.h"
#include "../../application_api/helicsTypes.hpp"
#include "../../core/core-data.h"
#include <mutex>
namespace helics
{
	class Core;
	class Federate;
	class Broker;
	class ValueFederate;
	class MessageFederate;
	class Subscription;
	class Publication;
	class Endpoint;
	class Filter;

    class FilterObject;

    /** type code embedded in the objects so the library knows how to cast them appropriately*/
	enum class vtype:int
	{
		genericFed,
		valueFed,
		messageFed,
		combinFed,
	};

	/** object wrapping a broker for the c-api*/
	class BrokerObject
	{
	public:
		std::shared_ptr<Broker> brokerptr;
        int index;
        int valid;
	};

	/** object wrapping a core for the c-api*/
	class CoreObject
	{
	public:
		std::shared_ptr<Core> coreptr;
        std::vector<FilterObject *> filters; //!< list of filters created directly through the core
        int index;
        int valid;
        CoreObject() = default;
        ~CoreObject();
	};
	
    


    class SubscriptionObject;
    class PublicationObject;
    class EndpointObject;
   

	/** object wrapping a federate for the c-api*/
	class FedObject
	{
	public:
		vtype type;
		int valid;
        int index;
		std::shared_ptr<Federate> fedptr;
		std::unique_ptr<Message> lastMessage;
        std::vector<SubscriptionObject *> subs;
        std::vector<PublicationObject *> pubs;
        std::vector<EndpointObject *> epts;
        std::vector<FilterObject *> filters;
        FedObject() = default;
        ~FedObject();
	};

	/** object wrapping a subscription*/
	class SubscriptionObject
	{
	public:
		std::unique_ptr<Subscription> subptr;
		subscription_id_t id;
		bool rawOnly = false;
		std::shared_ptr<ValueFederate> fedptr;
	};
	/** object wrapping a publication*/
	class PublicationObject
	{
	public:
		std::unique_ptr<Publication> pubptr;
		publication_id_t id;
		bool rawOnly = false;
		std::shared_ptr<ValueFederate> fedptr;
	};
	/** object wrapping and endpoint*/
	class EndpointObject
	{
	public:
		std::unique_ptr<Endpoint> endptr;
		std::shared_ptr<MessageFederate> fedptr;
		std::unique_ptr<Message> lastMessage;
	};

    enum class ftype
    {
        source,
        dest,
        clone,
    };

	/** object wrapping a source filter*/
	class FilterObject
	{
	public:
        ftype type;
        int valid;
		std::unique_ptr<Filter> filtptr;
		std::shared_ptr<Federate> fedptr;
        std::shared_ptr<Core> corePtr;
	};

    /** object representing a query*/
	class queryObject
	{
	public:
		std::string target; //!< the target of the query
		std::string query; //!< the actual query itself
		std::string response;   //!< the response to the query
        query_id_t asyncIndexCode=invalid_id_value;  //!< the index to use for the queryComplete call
        bool activeAsync = false;
        std::shared_ptr<Federate> activeFed; //!< pointer to the fed with the active Query
	};
}

helics::Federate *getFed(helics_federate fed);
helics::ValueFederate *getValueFed(helics_federate fed);
helics::MessageFederate *getMessageFed(helics_federate fed);
helics::Core *getCore(helics_core core);

std::shared_ptr<helics::Federate> getFedSharedPtr(helics_federate fed);
std::shared_ptr<helics::ValueFederate> getValueFedSharedPtr(helics_federate fed);
std::shared_ptr<helics::MessageFederate> getMessageFedSharedPtr(helics_federate fed);
std::shared_ptr<helics::Core> getCoreSharedPtr(helics_core core);

/** class for containing all the objects associated with a federation*/
class MasterObjectHolder
{
private:
    std::mutex ObjectLock;
    std::vector<helics::BrokerObject *> brokers;
    std::vector<helics::CoreObject *> cores;
    std::vector<helics::FedObject *> feds;
public:
    MasterObjectHolder() noexcept;
    ~MasterObjectHolder();
    int addBroker(helics::BrokerObject * broker);
    int addCore(helics::CoreObject *core);
    int addFed(helics::FedObject *fed);
    void clearBroker(int index);
    void clearCore(int index);
    void clearFed(int index);
    void deleteAll();
};

MasterObjectHolder *getMasterHolder();
void clearAllObjects();

#endif