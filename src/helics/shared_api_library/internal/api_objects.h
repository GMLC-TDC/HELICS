/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See the top-level NOTICE for
additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "../../application_api/helicsTypes.hpp"
#include "../../common/GuardedTypes.hpp"
#include "../../core/core-data.hpp"
#include "../api-data.h"
#include "gmlc/concurrency/TripWire.hpp"

#include <deque>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

/** this is a random identifier put in place when the federate or core or broker gets created*/
static const int coreValidationIdentifier = 0x378424EC;
static const int brokerValidationIdentifier = 0xA3467D20;

namespace helics {
class Core;
class Federate;
class Broker;
class ValueFederate;
class MessageFederate;
class Input;
class Publication;
class Endpoint;
class Filter;

class FilterObject;

/** type code embedded in the objects so the library knows how to cast them appropriately*/
enum class vtype : int { generic_fed, value_fed, message_fed, combination_fed, invalid_fed };

/** object wrapping a broker for the c-api*/
class BrokerObject {
  public:
    std::shared_ptr<Broker> brokerptr;
    int index{-2};
    int valid{0};
};

/** get the brokerObject from a helics_broker and verify it is valid*/
BrokerObject* getBrokerObject(helics_broker broker, helics_error* err) noexcept;
/** object wrapping a core for the c-api*/
class CoreObject {
  public:
    std::shared_ptr<Core> coreptr;
    std::vector<std::unique_ptr<FilterObject>> filters;  //!< list of filters created directly through the core
    int index{0};
    int valid{-2};
    CoreObject() = default;
    ~CoreObject();
};

/** get the CoreObject from a helics_core and verify it is valid*/
CoreObject* getCoreObject(helics_core core, helics_error* err) noexcept;

class InputObject;
class PublicationObject;
class EndpointObject;
/** generalized container for storing messages from a federate*/
class MessageHolder {
  private:
    std::vector<std::unique_ptr<Message>> messages;
    std::vector<int> freeMessageSlots;

  public:
    Message* addMessage(std::unique_ptr<Message>& mess);
    Message* newMessage();
    std::unique_ptr<Message> extractMessage(int index);
    void freeMessage(int index);
    void clear();
};
/** object wrapping a federate for the c-api*/
class FedObject {
  public:
    vtype type = vtype::invalid_fed;
    int index{-2};
    int valid{0};
    std::shared_ptr<Federate> fedptr;
    MessageHolder messages;
    std::vector<std::unique_ptr<InputObject>> inputs;
    std::vector<std::unique_ptr<PublicationObject>> pubs;
    std::vector<std::unique_ptr<EndpointObject>> epts;
    std::vector<std::unique_ptr<FilterObject>> filters;
    FedObject() = default;
    ~FedObject();
};

/** get the FedObject from a helics_broker and verify it is valid*/
FedObject* getFedObject(helics_federate fed, helics_error* err) noexcept;

/** object wrapping a subscription*/
class InputObject {
  public:
    int valid{0};
    std::shared_ptr<ValueFederate> fedptr;
    Input* inputPtr{nullptr};
};

/** object wrapping a publication*/
class PublicationObject {
  public:
    int valid{0};
    std::shared_ptr<ValueFederate> fedptr;
    Publication* pubPtr = nullptr;
};
/** object wrapping and endpoint*/
class EndpointObject {
  public:
    Endpoint* endPtr{nullptr};
    FedObject* fed{nullptr};
    std::shared_ptr<MessageFederate> fedptr;
    int valid{0};
};

/** object wrapping a source filter*/
class FilterObject {
  public:
    bool cloning{false};  //!< indicator that the filter is a cloning filter
    bool custom{false};  //!< indicator that the filter is a custom filter and requires a callback
    int valid{0};
    Filter* filtPtr{nullptr};
    std::unique_ptr<Filter> uFilter;
    std::shared_ptr<Federate> fedptr;
    std::shared_ptr<Core> corePtr;
};

/** object representing a query*/
class QueryObject {
  public:
    std::string target;  //!< the target of the query
    std::string query;  //!< the actual query itself
    std::string response;  //!< the response to the query
    std::shared_ptr<Federate> activeFed;  //!< pointer to the fed with the active Query
    bool activeAsync{false};
    helics_sequencing_mode mode{helics_sequencing_mode_fast};  //!< the ordering mode used for the query
    query_id_t asyncIndexCode;  //!< the index to use for the queryComplete call
    int valid{0};
};

}  // namespace helics

/** definitions to simplify error returns if an error already exists*/
#define HELICS_ERROR_CHECK(err, retval)                                                                                                    \
    do {                                                                                                                                   \
        if (((err) != nullptr) && ((err)->error_code != 0)) {                                                                              \
            return (retval);                                                                                                               \
        }                                                                                                                                  \
    } while (false)

/** assign an error string and code to an error object if it exists*/
inline void assignError(helics_error* err, int errorCode, const char* string)
{
    if (err != nullptr) {
        err->error_code = errorCode;
        err->message = string;
    }
}

extern const std::string emptyStr;
extern const std::string nullStringArgument;
#define AS_STRING(str) ((str) != nullptr) ? std::string(str) : emptyStr

#define CHECK_NULL_STRING(str, retval)                                                                                                     \
    do {                                                                                                                                   \
        if ((str) == nullptr) {                                                                                                            \
            assignError(err, helics_error_invalid_argument, nullStringArgument.c_str());                                                   \
            return (retval);                                                                                                               \
        }                                                                                                                                  \
    } while (false)

helics::Federate* getFed(helics_federate fed, helics_error* err);
helics::ValueFederate* getValueFed(helics_federate fed, helics_error* err);
helics::MessageFederate* getMessageFed(helics_federate fed, helics_error* err);
helics::Core* getCore(helics_core core, helics_error* err);
helics::Broker* getBroker(helics_broker broker, helics_error* err);
helics::Message* getMessageObj(helics_message_object message, helics_error* err);
/** create a message object from a message pointer*/
helics_message_object createMessageObject(std::unique_ptr<helics::Message>& mess);

std::shared_ptr<helics::Federate> getFedSharedPtr(helics_federate fed, helics_error* err);
std::shared_ptr<helics::ValueFederate> getValueFedSharedPtr(helics_federate fed, helics_error* err);
std::shared_ptr<helics::MessageFederate> getMessageFedSharedPtr(helics_federate fed, helics_error* err);
std::shared_ptr<helics::Core> getCoreSharedPtr(helics_core core, helics_error* err);
/**centralized error handler for the C interface*/
void helicsErrorHandler(helics_error* err) noexcept;
/** check if the output argument string is valid
@details  it takes and const char * since it doesn't modify it but it is intended to checked for output strings
function checks the output String is not nullptr and if the maxlen >0
fill the err term if it is not valid and return false,  otherwise return true if everything looks fine

*/
bool checkOutArgString(const char* outputString, int maxlen, helics_error* err);

/** class for containing all the objects associated with a federation*/
class MasterObjectHolder {
  private:
    guarded<std::deque<std::unique_ptr<helics::BrokerObject>>> brokers;
    guarded<std::deque<std::unique_ptr<helics::CoreObject>>> cores;
    guarded<std::deque<std::unique_ptr<helics::FedObject>>> feds;
    gmlc::concurrency::TripWireDetector tripDetect;  //!< detector for library termination
    guarded<std::deque<std::string>> errorStrings;  //!< container for strings generated from error conditions
  public:
    MasterObjectHolder() noexcept;
    ~MasterObjectHolder();
    helics::FedObject* findFed(const std::string& fedName);
    /** add a broker to the holder*/
    int addBroker(std::unique_ptr<helics::BrokerObject> broker);
    /** add a core to the holder*/
    int addCore(std::unique_ptr<helics::CoreObject> core);
    /** add a federate to the holder*/
    int addFed(std::unique_ptr<helics::FedObject> fed);
    void clearBroker(int index);
    void clearCore(int index);
    void clearFed(int index);
    void deleteAll();
    void abortAll(int errorCode, const std::string& error);
    /** store an error string to a string buffer
    @return a pointer to the memory location*/
    const char* addErrorString(std::string newError);
};

std::shared_ptr<MasterObjectHolder> getMasterHolder();
void clearAllObjects();
