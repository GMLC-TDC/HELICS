/*
Copyright (c) 2017-2024,
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
#include <exception>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

/** this is a random identifier put in place when the federate or core or broker gets created*/
static constexpr int gCoreValidationIdentifier = 0x3784'24EC;
static constexpr int gBrokerValidationIdentifier = 0xA346'7D20;

namespace helics {
class Core;
class Federate;
class Broker;
class ValueFederate;
class MessageFederate;
class CallbackFederate;
class FederateInfo;
class Input;
class Publication;
class Endpoint;
class Filter;
class Translator;

namespace apps {
    class App;
}

class FilterObject;
class TranslatorObject;
class SmallBuffer;

/** type code embedded in the objects so the library knows how to cast them appropriately*/
enum class FederateType : int { GENERIC, VALUE, MESSAGE, COMBINATION, CALLBACK, INVALID };

/** object wrapping a broker for the c-api*/
class BrokerObject {
  public:
    std::shared_ptr<Broker> brokerptr;
    int index{-2};
    int valid{0};
};

/** get the brokerObject from a HelicsBroker and verify it is valid*/
BrokerObject* getBrokerObject(HelicsBroker broker, HelicsError* err) noexcept;
/** object wrapping a core for the c-api*/
class CoreObject {
  public:
    std::shared_ptr<Core> coreptr;
    std::vector<std::unique_ptr<FilterObject>> filters;  //!< list of filters created directly through the core
    std::vector<std::unique_ptr<TranslatorObject>> translators;  //!< list of filters created directly through the core
    int index{0};
    int valid{-2};
    CoreObject() = default;
    ~CoreObject();
};

/** get the CoreObject from a HelicsCore and verify it is valid*/
CoreObject* getCoreObject(HelicsCore core, HelicsError* err) noexcept;

/** object representing an app*/
class AppObject {
  public:
    std::string type;  //!< the target of the query
    std::shared_ptr<apps::App> app;
    int index{-2};
    int valid{0};
};

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
    FederateType type = FederateType::INVALID;
    int index{-2};
    int valid{0};
    std::shared_ptr<Federate> fedptr;
    MessageHolder messages;
    std::vector<std::unique_ptr<InputObject>> inputs;
    std::vector<std::unique_ptr<PublicationObject>> pubs;
    std::vector<std::unique_ptr<EndpointObject>> epts;
    std::vector<std::unique_ptr<FilterObject>> filters;
    std::vector<std::unique_ptr<TranslatorObject>> translators;
    std::pair<std::string, std::string> commandBuffer;
    FedObject() = default;
    ~FedObject();
};

/** get the FedObject from a HelicsBroker and verify it is valid*/
FedObject* getFedObject(HelicsFederate fed, HelicsError* err) noexcept;

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

/** object wrapping a filter*/
class FilterObject {
  public:
    bool cloning{false};  //!< indicator that the filter is a cloning filter
    bool custom{false};  //!< indicator that the filter is a custom filter and requires a callback
    int valid{0};
    Filter* filtPtr{nullptr};
    std::unique_ptr<Filter> uFilter;
    std::shared_ptr<Federate> fedptr;
    std::shared_ptr<Core> corePtr;
    std::string buffer;
};

/** object wrapping a translator*/
class TranslatorObject {
  public:
    bool custom{false};  //!< indicator that the translator is a custom translator and requires callbacks
    int valid{0};
    Translator* transPtr{nullptr};
    std::unique_ptr<Translator> mTrans;
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
    HelicsSequencingModes mode{HELICS_SEQUENCING_MODE_FAST};  //!< the ordering mode used for the query
    QueryId asyncIndexCode;  //!< the index to use for the queryComplete call
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
inline void assignError(HelicsError* err, int error_code, const char* string)
{
    if (err != nullptr) {
        err->error_code = error_code;
        err->message = string;
    }
}

extern const std::string gHelicsEmptyStr;
constexpr char gHelicsNullStringArgument[] = "The supplied string argument is null and therefore invalid";
#define AS_STRING(str) ((str) != nullptr) ? std::string(str) : gHelicsEmptyStr

#define AS_STRING_VIEW(str) ((str) != nullptr) ? std::string_view(str) : std::string_view(gHelicsEmptyStr)

#define CHECK_NULL_STRING(str, retval)                                                                                                     \
    do {                                                                                                                                   \
        if ((str) == nullptr) {                                                                                                            \
            assignError(err, HELICS_ERROR_INVALID_ARGUMENT, gHelicsNullStringArgument);                                                    \
            return (retval);                                                                                                               \
        }                                                                                                                                  \
    } while (false)

helics::Federate* getFed(HelicsFederate fed, HelicsError* err);
helics::ValueFederate* getValueFed(HelicsFederate fed, HelicsError* err);
helics::MessageFederate* getMessageFed(HelicsFederate fed, HelicsError* err);
helics::CallbackFederate* getCallbackFed(HelicsFederate fed, HelicsError* err);
helics::FederateInfo* getFedInfo(HelicsFederateInfo fedInfo, HelicsError* err);
helics::Core* getCore(HelicsCore core, HelicsError* err);
helics::Broker* getBroker(HelicsBroker broker, HelicsError* err);
helics::Message* getMessageObj(HelicsMessage message, HelicsError* err);
helics::apps::App* getApp(HelicsApp, HelicsError* err);
/** generate a new helicsFederate and store it in the master*/
HelicsFederate generateNewHelicsFederateObject(std::shared_ptr<helics::Federate> fed, helics::FederateType type);

std::unique_ptr<helics::Message> getMessageUniquePtr(HelicsMessage message, HelicsError* err);
/** create a message object from a message pointer*/
HelicsMessage createAPIMessage(std::unique_ptr<helics::Message>& mess);

/** add required information to SmallBuffer and return a HelicsDataBuffer object*/
HelicsDataBuffer createAPIDataBuffer(helics::SmallBuffer& buff);
/** get the small buffer point from a HelicsDataBuffer*/
helics::SmallBuffer* getBuffer(HelicsDataBuffer data);

std::shared_ptr<helics::Federate> getFedSharedPtr(HelicsFederate fed, HelicsError* err);
std::shared_ptr<helics::ValueFederate> getValueFedSharedPtr(HelicsFederate fed, HelicsError* err);
std::shared_ptr<helics::MessageFederate> getMessageFedSharedPtr(HelicsFederate fed, HelicsError* err);
std::shared_ptr<helics::CallbackFederate> getCallbackFedSharedPtr(HelicsFederate fed, HelicsError* err);
std::shared_ptr<helics::Core> getCoreSharedPtr(HelicsCore core, HelicsError* err);

std::shared_ptr<helics::apps::App> getAppSharedPtr(HelicsApp app, HelicsError* err);
/**centralized error handler for the C interface*/
void helicsErrorHandler(HelicsError* err) noexcept;
/** check if the output argument string is valid
@details  it takes and const char * since it doesn't modify it but it is intended to checked for output strings
function checks the output String is not nullptr and if the maxlen >0
fill the err term if it is not valid and return false,  otherwise return true if everything looks fine

*/
bool checkOutputArgString(const char* outputString, int maxlen, HelicsError* err);

/** class for containing all the objects associated with a federation*/
class MasterObjectHolder {
  private:
    guarded<std::deque<std::unique_ptr<helics::BrokerObject>>> brokers;
    guarded<std::deque<std::unique_ptr<helics::CoreObject>>> cores;
    guarded<std::deque<std::unique_ptr<helics::FedObject>>> feds;
    guarded<std::deque<std::unique_ptr<helics::AppObject>>> apps;
    gmlc::concurrency::TripWireDetector tripDetect;  //!< detector for library termination
    guarded<std::deque<std::string>> errorStrings;  //!< container for strings generated from error conditions
  public:
    MasterObjectHolder() noexcept;
    ~MasterObjectHolder();
    helics::FedObject* findFed(std::string_view fedName);
    /** find a specific fed by name and validation code*/
    helics::FedObject* findFed(std::string_view fedName, int validationCode);
    /** add a broker to the holder*/
    int addBroker(std::unique_ptr<helics::BrokerObject> broker);
    /** add a core to the holder*/
    int addCore(std::unique_ptr<helics::CoreObject> core);
    /** add a federate to the holder*/
    int addFed(std::unique_ptr<helics::FedObject> fed);
    /** remove a federate object*/
    bool removeFed(std::string_view name, int validationCode);
    /** add an app to the holder*/
    int addApp(std::unique_ptr<helics::AppObject> app);

    void clearBroker(int index);
    void clearCore(int index);
    void clearFed(int index);
    void clearApp(int index);
    void deleteAll();
    void abortAll(int errorCode, std::string_view error);
    /** store an error string to a string buffer
    @return a pointer to the memory location*/
    const char* addErrorString(std::string_view newError);
};

std::shared_ptr<MasterObjectHolder> getMasterHolder();
void clearAllObjects();
