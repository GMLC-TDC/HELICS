/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "EmptyCore.hpp"

#include "../common/JsonGeneration.hpp"
#include "core-exceptions.hpp"
#include "helicsVersion.hpp"

#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace helics {

// NOLINTNEXTLINE
const std::string EmptyCore::emptyString{};

// timeoutMon is a unique_ptr
EmptyCore::EmptyCore() noexcept {}

void EmptyCore::configure(std::string_view /*configureString*/) {}

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
    return emptyString;
}

const std::string& EmptyCore::getAddress() const
{
    return emptyString;
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
                            std::string_view errorString)
{
    throw(FederateError(errorCode, errorString));
}

void EmptyCore::localError(LocalFederateId /*federateID*/,
                           int errorCode,
                           std::string_view errorString)
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

bool EmptyCore::enterInitializingMode(LocalFederateId /*federateID*/, IterationRequest /*request*/)
{
    return false;
}

iteration_time EmptyCore::enterExecutingMode(LocalFederateId /*federateID*/,
                                             IterationRequest /*iterate*/)
{
    return {Time::maxVal(), IterationResult::HALTED};
}

LocalFederateId EmptyCore::registerFederate(std::string_view /*name*/,
                                            const CoreFederateInfo& /*info*/)
{
    throw(RegistrationFailure(std::string("Registration is not possible for Null Core")));
}

const std::string& EmptyCore::getFederateName(LocalFederateId /*federateID*/) const
{
    throw(InvalidIdentifier("federateID not valid (federateName)"));
}

// static const std::string unknownString("#unknown");

LocalFederateId EmptyCore::getFederateId(std::string_view /*name*/) const
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

InterfaceHandle EmptyCore::registerInput(LocalFederateId /*federateID*/,
                                         std::string_view /*key*/,
                                         std::string_view /*type*/,
                                         std::string_view /*units*/)
{
    return {};
}

InterfaceHandle EmptyCore::registerTranslator(std::string_view /*translatorName*/,
                                              std::string_view /*message_type*/,
                                              std::string_view /*units*/)
{
    return {};
}

InterfaceHandle EmptyCore::getInput(LocalFederateId /*federateID*/, std::string_view /*key*/) const
{
    return {};
}

InterfaceHandle EmptyCore::registerPublication(LocalFederateId /*federateID*/,
                                               std::string_view /*key*/,
                                               std::string_view /*type*/,
                                               std::string_view /*units*/)
{
    return {};
}

InterfaceHandle EmptyCore::getPublication(LocalFederateId /*federateID*/,
                                          std::string_view /*key*/) const
{
    return {};
}

const std::string& EmptyCore::getHandleName(InterfaceHandle /*handle*/) const
{
    return emptyString;
}

const std::string& EmptyCore::getInjectionUnits(InterfaceHandle /*handle*/) const
{
    return emptyString;
}  // namespace helics

const std::string& EmptyCore::getExtractionUnits(InterfaceHandle /*handle*/) const
{
    return emptyString;
}

const std::string& EmptyCore::getInjectionType(InterfaceHandle /*handle*/) const
{
    return emptyString;
}

const std::string& EmptyCore::getExtractionType(InterfaceHandle /*handle*/) const
{
    return emptyString;
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
    throw(InvalidFunctionCall("core is not connected unable to add targets"));
}

void EmptyCore::addSourceTarget(InterfaceHandle /*handle*/,
                                std::string_view /*targetName*/,
                                InterfaceType /*hint*/)
{
    throw(InvalidFunctionCall("core is not connected unable to add targets"));
}

const std::string& EmptyCore::getDestinationTargets(InterfaceHandle /*handle*/) const
{
    return emptyString;
}

const std::string& EmptyCore::getSourceTargets(InterfaceHandle /*handle*/) const
{
    return emptyString;
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
                                            std::string_view /*name*/,
                                            std::string_view /*type*/)
{
    return {};
}

InterfaceHandle EmptyCore::registerTargetedEndpoint(LocalFederateId /*federateID*/,
                                                    std::string_view /*name*/,
                                                    std::string_view /*type*/)
{
    return {};
}

InterfaceHandle EmptyCore::getEndpoint(LocalFederateId /*federateID*/,
                                       std::string_view /*name*/) const
{
    return {};
}

InterfaceHandle EmptyCore::registerDataSink(LocalFederateId /*federateID*/,
                                            std::string_view /*name*/)
{
    return {};
}

InterfaceHandle EmptyCore::getDataSink(LocalFederateId /*federateID*/,
                                       std::string_view /*name*/) const
{
    return {};
}

InterfaceHandle EmptyCore::registerFilter(std::string_view /*filterName*/,
                                          std::string_view /*type_in*/,
                                          std::string_view /*type_out*/)
{
    return {};
}

InterfaceHandle EmptyCore::registerCloningFilter(std::string_view /*filterName*/,
                                                 std::string_view /*type_in*/,
                                                 std::string_view /*type_out*/)
{
    return {};
}

InterfaceHandle EmptyCore::getFilter(std::string_view /*name*/) const
{
    return {};
}

InterfaceHandle EmptyCore::getTranslator(std::string_view /*name*/) const
{
    return {};
}
void EmptyCore::makeConnections(const std::string& /*file*/) {}

void EmptyCore::linkEndpoints(std::string_view /*source*/, std::string_view /*dest*/) {}

void EmptyCore::addAlias(std::string_view /*interfaceKey*/, std::string_view /*alias*/) {}
void EmptyCore::dataLink(std::string_view /*source*/, std::string_view /*target*/) {}

void EmptyCore::addSourceFilterToEndpoint(std::string_view /*filter*/,
                                          std::string_view /*endpoint*/)
{
}

void EmptyCore::addDestinationFilterToEndpoint(std::string_view /*filter*/,
                                               std::string_view /*endpoint*/)
{
}

void EmptyCore::addDependency(LocalFederateId /*federateID*/, std::string_view /*federateName*/) {}

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
                           std::string_view messageToLog)
{
    if (logLevel <= HELICS_LOG_LEVEL_WARNING) {
        std::cerr << messageToLog << std::endl;
    } else {
        std::cout << messageToLog << std::endl;
    }
}

void EmptyCore::setLoggingLevel(int /*logLevel*/) {}

void EmptyCore::setLogFile(std::string_view /*lfile*/) {}

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

void EmptyCore::setTranslatorOperator(InterfaceHandle /*translator*/,
                                      std::shared_ptr<TranslatorOperator> /*callback*/)
{
}

void EmptyCore::setFederateOperator(LocalFederateId /*federateID */,
                                    std::shared_ptr<FederateOperator> /*callback*/)
{
}

void EmptyCore::setQueryCallback(LocalFederateId /*federateID*/,
                                 std::function<std::string(std::string_view)> /*queryFunction*/,
                                 int /*order*/)
{
}

static std::string quickCoreQueries(std::string_view queryStr)
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

std::string EmptyCore::query(std::string_view target,
                             std::string_view queryStr,
                             HelicsSequencingModes /*mode*/)
{
    if (target == "core" || target == getIdentifier() || target.empty()) {
        return quickCoreQueries(queryStr);
    }
    return generateJsonErrorResponse(JsonErrorCodes::DISCONNECTED, "Federate is disconnected");
}

void EmptyCore::setGlobal(std::string_view /*valueName*/, std::string_view /*value*/) {}

void EmptyCore::sendCommand(std::string_view /*target*/,
                            std::string_view /*commandStr*/,
                            std::string_view /*source*/,
                            HelicsSequencingModes /*mode*/)
{
}

const std::string& EmptyCore::getInterfaceInfo(InterfaceHandle /*handle*/) const
{
    return emptyString;
}

void EmptyCore::setInterfaceInfo(helics::InterfaceHandle /*handle*/, std::string_view /*info*/) {}

const std::string& EmptyCore::getInterfaceTag(InterfaceHandle /*handle*/,
                                              std::string_view /*tag*/) const
{
    return emptyString;
}

void EmptyCore::setInterfaceTag(InterfaceHandle /*handle*/,
                                std::string_view /*tag*/,
                                std::string_view /*value*/)
{
}

const std::string& EmptyCore::getFederateTag(LocalFederateId /*fid*/,
                                             std::string_view /*tag*/) const
{
    return emptyString;
}

void EmptyCore::setFederateTag(LocalFederateId /*fid*/,
                               std::string_view /*tag*/,
                               std::string_view /*value*/)
{
}

}  // namespace helics
