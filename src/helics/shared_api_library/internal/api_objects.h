/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#pragma once

#include <memory>
#include "../api-data.h"
#include "../../application_api/helicsTypes.hpp"
#include "../../core/core-data.hpp"
#include "../../common/GuardedTypes.hpp"
#include "../../common/TripWire.hpp"
#include <mutex>

/** this is a random identifier put in place when the federate or core or broker gets created*/
static const int coreValidationIdentifier = 0x378424EC;
static const int brokerValidationIdentifier = 0xA3467D20;

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
		combinationFed
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
        std::vector<std::unique_ptr<FilterObject>> filters; //!< list of filters created directly through the core
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
        std::vector<std::unique_ptr<SubscriptionObject>> subs;
        std::vector<std::unique_ptr<PublicationObject>> pubs;
        std::vector<std::unique_ptr<EndpointObject>> epts;
        std::vector<std::unique_ptr<FilterObject>> filters;
        FedObject() = default;
        ~FedObject();
	};

	/** object wrapping a subscription*/
	class SubscriptionObject
	{
	public:
		std::unique_ptr<Subscription> subptr;
		subscription_id_t id;
        int valid;
		bool rawOnly = false;
		std::shared_ptr<ValueFederate> fedptr;
	};
	/** object wrapping a publication*/
	class PublicationObject
	{
	public:
		std::unique_ptr<Publication> pubptr;
		publication_id_t id;
        int valid;
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
        int valid;
	};

    /** enumeration of possible filter object types*/
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
        std::shared_ptr<Federate> activeFed; //!< pointer to the fed with the active Query
        query_id_t asyncIndexCode=invalid_id_value;  //!< the index to use for the queryComplete call
        bool activeAsync = false;
        int valid;

	};
}

helics::Federate *getFed(helics_federate fed);
helics::ValueFederate *getValueFed(helics_federate fed);
helics::MessageFederate *getMessageFed(helics_federate fed);
helics::Core *getCore(helics_core core);
helics::Broker *getBroker(helics_broker broker);

std::shared_ptr<helics::Federate> getFedSharedPtr(helics_federate fed);
std::shared_ptr<helics::ValueFederate> getValueFedSharedPtr(helics_federate fed);
std::shared_ptr<helics::MessageFederate> getMessageFedSharedPtr(helics_federate fed);
std::shared_ptr<helics::Core> getCoreSharedPtr(helics_core core);
/**centralized error handler for the C interface*/
helics_status helicsErrorHandler() noexcept;

/** class for containing all the objects associated with a federation*/
class MasterObjectHolder
{
private:
    guarded<std::vector<std::unique_ptr<helics::BrokerObject>>> brokers;
    guarded<std::vector<std::unique_ptr<helics::CoreObject>>> cores;
    guarded<std::vector<std::unique_ptr<helics::FedObject>>> feds;
    tripwire::TripWireDetector tripDetect;
public:
    MasterObjectHolder() noexcept;
    ~MasterObjectHolder();
    helics::FedObject *findFed(const std::string &fedName);
    int addBroker(std::unique_ptr<helics::BrokerObject> broker);
    int addCore(std::unique_ptr<helics::CoreObject> core);
    int addFed(std::unique_ptr<helics::FedObject> fed);
    void clearBroker(int index);
    void clearCore(int index);
    void clearFed(int index);
    void deleteAll();
};

std::shared_ptr<MasterObjectHolder> getMasterHolder();
void clearAllObjects();

