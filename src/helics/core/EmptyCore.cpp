/*
Copyright (c) 2017-2022,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "EmptyCore.hpp"

#include "../common/JsonGeneration.hpp"
#include "core-exceptions.hpp"
#include "helicsVersion.hpp"

#include <iostream>

namespace helics {

// timeoutMon is a unique_ptr
EmptyCore::EmptyCore() noexcept {}

void EmptyCore::configure(const std::string& /*configureString*/) {}

void EmptyCore::configureFromArgs(int /*argc*/, char* /*argv*/[]) {}

void EmptyCore::configureFromVector(std::vector<std::string> /*args*/) {}

bool EmptyCore::connect()
{
    return false;
}

bool EmptyCore::isConnected() const
{
    return false;
}

const std::string& EmptyCore::getIdentifier() const
{
    static const std::string nullStr;
    return nullStr;
}

const std::string& EmptyCore::getAddress() const
{
    static const std::string nullStr;
    return nullStr;
}

void EmptyCore::disconnect() {}

bool EmptyCore::waitForDisconnect(std::chrono::milliseconds /*msToWait*/) const
{
    return true;
}

EmptyCore::~EmptyCore() {}

bool EmptyCore::isConfigured() const
{
    return false;
}

bool EmptyCore::isOpenToNewFederates() const
{
    return false;
}

bool EmptyCore::hasError() const
{
    return false;
}
void EmptyCore::globalError(LocalFederateId /*federateID*/,
                            int errorCode,
                            const std::string& errorString)
{
    throw(FederateError(errorCode, errorString));
}

void EmptyCore::localError(LocalFederateId /*federateID*/,
                           int errorCode,
                           const std::string& errorString)
{
    throw(FederateError(errorCode, errorString));
}

int EmptyCore::getErrorCode() const
{
    return 0;
}

std::string EmptyCore::getErrorMessage() const
{
    return "";
}

void EmptyCore::finalize(LocalFederateId /*federateID*/) {}

void EmptyCore::setCoreReadyToInit() {}

void EmptyCore::enterInitializingMode(LocalFederateId /*federateID*/) {}

IterationResult EmptyCore::enterExecutingMode(LocalFederateId /*federateID*/,
                                              IterationRequest /*iterate*/)
{
    return IterationResult::HALTED;
}

LocalFederateId EmptyCore::registerFederate(const std::string& /*name*/,
                                            const CoreFederateInfo& /*info*/)
{
    throw(RegistrationFailure(std::string("Registration is not possible for Null Core")));
}

const std::string& EmptyCore::getFederateName(LocalFederateId /*federateID*/) const
{
    throw(InvalidIdentifier("federateID not valid (federateName)"));
}

static const std::string unknownString("#unknown");

LocalFederateId EmptyCore::getFederateId(const std::string& /*name*/) const
{
    return {};
}

int32_t EmptyCore::getFederationSize()
{
    return 0;
}

Time EmptyCore::timeRequest(LocalFederateId /*federateID*/, Time /*next*/)
{
    throw(InvalidFunctionCall("time request should only be called in execution state"));
}

iteration_time EmptyCore::requestTimeIterative(LocalFederateId /*federateID*/,
                                               Time /*next*/,
                                               IterationRequest /*iterate*/)
{
    throw(InvalidFunctionCall("time request should only be called in execution state"));
}

void EmptyCore::processCommunications(LocalFederateId /*federateID*/,
                                      std::chrono::milliseconds /*msToWait*/)
{
}

Time EmptyCore::getCurrentTime(LocalFederateId /*federateID*/) const
{
    return Time::maxVal();
}

uint64_t EmptyCore::getCurrentReiteration(LocalFederateId /*federateID*/) const
{
    return 0U;
}

void EmptyCore::setIntegerProperty(LocalFederateId /*federateID*/,
                                   int32_t /*property*/,
                                   int16_t /*propertyValue*/)
{
}

void EmptyCore::setTimeProperty(LocalFederateId /*federateID*/, int32_t /*property*/, Time /*time*/)
{
}

Time EmptyCore::getTimeProperty(LocalFederateId /*federateID*/, int32_t /*property*/) const
{
    return Time::minVal();
}

int16_t EmptyCore::getIntegerProperty(LocalFederateId /*federateID*/, int32_t /*property*/) const
{
    return 0;
}

void EmptyCore::setFlagOption(LocalFederateId /*federateID*/, int32_t /*flag*/, bool /*flagValue*/)
{
}

bool EmptyCore::getFlagOption(LocalFederateId /*federateID*/, int32_t /*flag*/) const
{
    return false;
}

static const std::string emptyString;

InterfaceHandle EmptyCore::registerInput(LocalFederateId /*federateID*/,
                                         const std::string& /*key*/,
                                         const std::string& /*type*/,
                                         const std::string& /*units*/)
{
    return {};
}

InterfaceHandle EmptyCore::getInput(LocalFederateId /*federateID*/,
                                    const std::string& /*key*/) const
{
    return {};
}

InterfaceHandle EmptyCore::registerPublication(LocalFederateId /*federateID*/,
                                               const std::string& /*key*/,
                                               const std::string& /*type*/,
                                               const std::string& /*units*/)
{
    return {};
}

InterfaceHandle EmptyCore::getPublication(LocalFederateId /*federateID*/,
                                          const std::string& /*key*/) const
{
    return {};
}

const std::string emptyStr;

const std::string& EmptyCore::getHandleName(InterfaceHandle /*handle*/) const
{
    return emptyStr;
}

const std::string& EmptyCore::getInjectionUnits(InterfaceHandle /*handle*/) const
{
    return emptyStr;
}  // namespace helics

const std::string& EmptyCore::getExtractionUnits(InterfaceHandle /*handle*/) const
{
    return emptyStr;
}

const std::string& EmptyCore::getInjectionType(InterfaceHandle /*handle*/) const
{
    return emptyStr;
}

const std::string& EmptyCore::getExtractionType(InterfaceHandle /*handle*/) const
{
    return emptyStr;
}

void EmptyCore::setHandleOption(InterfaceHandle /*handle*/,
                                int32_t /*option*/,
                                int32_t /*option_value*/)
{
}

int32_t EmptyCore::getHandleOption(InterfaceHandle /*handle*/, int32_t /*option*/) const
{
    return 0;
}

void EmptyCore::closeHandle(InterfaceHandle /*handle*/) {}

void EmptyCore::removeTarget(InterfaceHandle /*handle*/, std::string_view /*targetToRemove*/) {}

void EmptyCore::addDestinationTarget(InterfaceHandle /*handle*/,
                                     std::string_view /*dest*/,
                                     InterfaceType /*hint*/)
{
}

void EmptyCore::addSourceTarget(InterfaceHandle /*handle*/,
                                std::string_view /*targetName*/,
                                InterfaceType /*hint*/)
{
}

const std::string& EmptyCore::getDestinationTargets(InterfaceHandle /*handle*/) const
{
    return emptyStr;
}

const std::string& EmptyCore::getSourceTargets(InterfaceHandle /*handle*/) const
{
    return emptyStr;
}

void EmptyCore::setValue(InterfaceHandle /*handle*/, const char* /*data*/, uint64_t /*len*/) {}

const std::shared_ptr<const SmallBuffer>& EmptyCore::getValue(InterfaceHandle /*handle*/,
                                                              uint32_t* /*inputIndex*/)
{
    static const std::shared_ptr<const SmallBuffer> empty;
    return empty;
}

const std::vector<std::shared_ptr<const SmallBuffer>>&
    EmptyCore::getAllValues(InterfaceHandle /*handle*/)
{
    static const std::vector<std::shared_ptr<const SmallBuffer>> emptyV;
    return emptyV;
}

const std::vector<InterfaceHandle>& EmptyCore::getValueUpdates(LocalFederateId /*federateID*/)
{
    static const std::vector<InterfaceHandle> emptyV;

    return emptyV;
}

InterfaceHandle EmptyCore::registerEndpoint(LocalFederateId /*federateID*/,
                                            const std::string& /*name*/,
                                            const std::string& /*type*/)
{
    return {};
}

InterfaceHandle EmptyCore::registerTargetedEndpoint(LocalFederateId /*federateID*/,
                                                    const std::string& /*name*/,
                                                    const std::string& /*type*/)
{
    return {};
}

InterfaceHandle EmptyCore::getEndpoint(LocalFederateId /*federateID*/,
                                       const std::string& /*name*/) const
{
    return {};
}

InterfaceHandle EmptyCore::registerFilter(const std::string& /*filterName*/,
                                          const std::string& /*type_in*/,
                                          const std::string& /*type_out*/)
{
    return {};
}

InterfaceHandle EmptyCore::registerCloningFilter(const std::string& /*filterName*/,
                                                 const std::string& /*type_in*/,
                                                 const std::string& /*type_out*/)
{
    return {};
}

InterfaceHandle EmptyCore::getFilter(const std::string& /*name*/) const
{
    return {};
}

void EmptyCore::makeConnections(const std::string& /*file*/) {}

void EmptyCore::linkEndpoints(const std::string& /*source*/, const std::string& /*dest*/) {}

void EmptyCore::dataLink(const std::string& /*source*/, const std::string& /*target*/) {}

void EmptyCore::addSourceFilterToEndpoint(const std::string& /*filter*/,
                                          const std::string& /*endpoint*/)
{
}

void EmptyCore::addDestinationFilterToEndpoint(const std::string& /*filter*/,
                                               const std::string& /*endpoint*/)
{
}

void EmptyCore::addDependency(LocalFederateId /*federateID*/, const std::string& /*federateName*/)
{
}

void EmptyCore::sendTo(InterfaceHandle /*sourceHandle*/,
                       const void* /*data*/,
                       uint64_t /*length*/,
                       std::string_view /*destination*/)
{
}

void EmptyCore::sendToAt(InterfaceHandle /*sourceHandle*/,
                         const void* /*data*/,
                         uint64_t /*length*/,
                         std::string_view /*destination*/,
                         Time /*sendTime*/)
{
}

void EmptyCore::send(InterfaceHandle /*sourceHandle*/, const void* /*data*/, uint64_t /*length*/) {}

void EmptyCore::sendAt(InterfaceHandle /*sourceHandle*/,
                       const void* /*data*/,
                       uint64_t /*length*/,
                       Time /*time*/)
{
}

void EmptyCore::sendMessage(InterfaceHandle /*sourceHandle*/, std::unique_ptr<Message> /*message*/)
{
}

uint64_t EmptyCore::receiveCount(InterfaceHandle /*destination*/)
{
    return 0;
}

std::unique_ptr<Message> EmptyCore::receive(InterfaceHandle /*destination*/)
{
    return nullptr;
}

std::unique_ptr<Message> EmptyCore::receiveAny(LocalFederateId /*federateID*/,
                                               InterfaceHandle& /*endpoint_id*/)
{
    return nullptr;
}

uint64_t EmptyCore::receiveCountAny(LocalFederateId /*federateID*/)
{
    return 0;
}

void EmptyCore::logMessage(LocalFederateId /*federateID*/,
                           int logLevel,
                           const std::string& messageToLog)
{
    if (logLevel <= HELICS_LOG_LEVEL_WARNING) {
        std::cerr << messageToLog << std::endl;
    } else {
        std::cout << messageToLog << std::endl;
    }
}

void EmptyCore::setLoggingLevel(int /*logLevel*/) {}

void EmptyCore::setLogFile(const std::string& /*lfile*/) {}

std::pair<std::string, std::string> EmptyCore::getCommand(LocalFederateId /*federateID*/)
{
    return {};
}

std::pair<std::string, std::string> EmptyCore::waitCommand(LocalFederateId /*federateID*/)
{
    return {};
}

void EmptyCore::setLoggingCallback(
    LocalFederateId /*federateID*/,
    std::function<void(int, std::string_view, std::string_view)> /*logFunction*/)
{
}

void EmptyCore::setFilterOperator(InterfaceHandle /*filter*/,
                                  std::shared_ptr<FilterOperator> /*callback*/)
{
}

void EmptyCore::setQueryCallback(LocalFederateId /*federateID*/,
                                 std::function<std::string(std::string_view)> /*queryFunction*/)
{
}

static std::string quickCoreQueries(const std::string& queryStr)
{
    if ((queryStr == "queries") || (queryStr == "available_queries")) {
        return "[\"isinit\",\"isconnected\",\"exists\",\"name\",\"identifier\",\"address\",\"queries\",\"address\",\"federates\",\"inputs\",\"endpoints\",\"filtered_endpoints\","
               "\"publications\",\"filters\",\"tags\",\"version\",\"version_all\",\"federate_map\",\"dependency_graph\",\"data_flow_graph\",\"dependencies\",\"dependson\",\"dependents\",\"current_time\",\"global_time\",\"global_state\",\"global_flush\",\"current_state\"]";
    }
    if (queryStr == "isconnected" || queryStr == "isinit") {
        return "false";
    }
    if (queryStr == "name" || queryStr == "identifier") {
        return std::string{"\""} + "null" + '"';
    }
    if (queryStr == "exists") {
        return "true";
    }
    if (queryStr == "version") {
        return std::string{"\""} + versionString + '"';
    }
    return generateJsonErrorResponse(JsonErrorCodes::DISCONNECTED, "Core is disconnected");
}

std::string EmptyCore::query(const std::string& target,
                             const std::string& queryStr,
                             HelicsSequencingModes /*mode*/)
{
    if (target == "core" || target == getIdentifier() || target.empty()) {
        return quickCoreQueries(queryStr);
    }
    return generateJsonErrorResponse(JsonErrorCodes::DISCONNECTED, "Federate is disconnected");
}

void EmptyCore::setGlobal(const std::string& /*valueName*/, const std::string& /*value*/) {}

void EmptyCore::sendCommand(const std::string& /*target*/,
                            const std::string& /*commandStr*/,
                            const std::string& /*source*/,
                            HelicsSequencingModes /*mode*/)
{
}

const std::string& EmptyCore::getInterfaceInfo(InterfaceHandle /*handle*/) const
{
    return emptyStr;
}

void EmptyCore::setInterfaceInfo(helics::InterfaceHandle /*handle*/, std::string /*info*/) {}

const std::string& EmptyCore::getInterfaceTag(InterfaceHandle /*handle*/,
                                              const std::string& /*tag*/) const
{
    return emptyStr;
}

void EmptyCore::setInterfaceTag(InterfaceHandle /*handle*/,
                                const std::string& /*tag*/,
                                const std::string& /*value*/)
{
}

const std::string& EmptyCore::getFederateTag(LocalFederateId /*fid*/,
                                             const std::string& /*tag*/) const
{
    return emptyStr;
}

void EmptyCore::setFederateTag(LocalFederateId /*fid*/,
                               const std::string& /*tag*/
                               ,
                               const std::string& /*value*/)
{
}

}  // namespace helics
