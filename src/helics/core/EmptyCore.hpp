/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "Core.hpp"

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace helics {

/** a dummy core that responds like a disconnected finished core*/
class EmptyCore: public Core {
  public:
    /** default constructor*/
    EmptyCore() noexcept;

    virtual ~EmptyCore();

    virtual void configure(std::string_view configureString) override;
    virtual void configureFromArgs(int argc, char* argv[]) override;
    virtual void configureFromVector(std::vector<std::string> args) override;
    virtual bool isConfigured() const override;
    virtual bool isOpenToNewFederates() const override;
    virtual bool hasError() const override;
    virtual void globalError(LocalFederateId federateID,
                             int errorCode,
                             std::string_view errorString) override;
    virtual void localError(LocalFederateId federateID,
                            int errorCode,
                            std::string_view errorString) override;
    virtual int getErrorCode() const override;
    virtual std::string getErrorMessage() const override;
    virtual void finalize(LocalFederateId federateID) override;
    virtual bool enterInitializingMode(LocalFederateId federateID,
                                       IterationRequest request) override;
    virtual void setCoreReadyToInit() override;
    virtual iteration_time enterExecutingMode(LocalFederateId federateID,
                                              IterationRequest iterate = NO_ITERATION) override;
    virtual LocalFederateId registerFederate(std::string_view name,
                                             const CoreFederateInfo& info) override;
    virtual const std::string& getFederateName(LocalFederateId federateID) const override;
    virtual LocalFederateId getFederateId(std::string_view name) const override;
    virtual int32_t getFederationSize() override;
    virtual Time timeRequest(LocalFederateId federateID, Time next) override;
    virtual iteration_time requestTimeIterative(LocalFederateId federateID,
                                                Time next,
                                                IterationRequest iterate) override;
    virtual void processCommunications(LocalFederateId fedId,
                                       std::chrono::milliseconds msToWait) override final;
    virtual Time getCurrentTime(LocalFederateId federateID) const override;
    virtual void setTimeProperty(LocalFederateId federateID, int32_t property, Time time) override;
    virtual void setIntegerProperty(LocalFederateId federateID,
                                    int32_t property,
                                    int16_t propertyValue) override;
    virtual Time getTimeProperty(LocalFederateId federateID, int32_t property) const override;
    virtual int16_t getIntegerProperty(LocalFederateId federateID, int32_t property) const override;
    virtual void
        setFlagOption(LocalFederateId federateID, int32_t flag, bool flagValue = true) override;
    virtual bool getFlagOption(LocalFederateId federateID, int32_t flag) const override;

    virtual InterfaceHandle registerPublication(LocalFederateId federateID,
                                                std::string_view key,
                                                std::string_view type,
                                                std::string_view units) override;
    virtual InterfaceHandle getPublication(LocalFederateId federateID,
                                           std::string_view key) const override;
    virtual InterfaceHandle registerInput(LocalFederateId federateID,
                                          std::string_view key,
                                          std::string_view type,
                                          std::string_view units) override;

    virtual InterfaceHandle getInput(LocalFederateId federateID,
                                     std::string_view key) const override;

    virtual InterfaceHandle registerTranslator(std::string_view translatorName,
                                               std::string_view message_type,
                                               std::string_view units) override;

    virtual const std::string& getHandleName(InterfaceHandle handle) const override;

    virtual void
        setHandleOption(InterfaceHandle handle, int32_t option, int32_t option_value) override;

    virtual int32_t getHandleOption(InterfaceHandle handle, int32_t option) const override;
    virtual void closeHandle(InterfaceHandle handle) override;
    virtual void removeTarget(InterfaceHandle handle, std::string_view targetToRemove) override;
    virtual void addDestinationTarget(InterfaceHandle handle,
                                      std::string_view dest,
                                      InterfaceType hint) override;
    virtual void
        addSourceTarget(InterfaceHandle handle, std::string_view name, InterfaceType hint) override;
    virtual const std::string& getDestinationTargets(InterfaceHandle handle) const override;

    virtual const std::string& getSourceTargets(InterfaceHandle handle) const override;
    virtual const std::string& getInjectionUnits(InterfaceHandle handle) const override;
    virtual const std::string& getExtractionUnits(InterfaceHandle handle) const override;
    virtual const std::string& getInjectionType(InterfaceHandle handle) const override;
    virtual const std::string& getExtractionType(InterfaceHandle handle) const override;
    virtual void setValue(InterfaceHandle handle, const char* data, uint64_t len) override;
    virtual const std::shared_ptr<const SmallBuffer>& getValue(InterfaceHandle handle,
                                                               uint32_t* inputIndex) override;
    virtual const std::vector<std::shared_ptr<const SmallBuffer>>&
        getAllValues(InterfaceHandle handle) override;
    virtual const std::vector<InterfaceHandle>&
        getValueUpdates(LocalFederateId federateID) override;
    virtual InterfaceHandle registerEndpoint(LocalFederateId federateID,
                                             std::string_view name,
                                             std::string_view type) override;

    virtual InterfaceHandle registerTargetedEndpoint(LocalFederateId federateID,
                                                     std::string_view name,
                                                     std::string_view type) override;
    virtual InterfaceHandle getEndpoint(LocalFederateId federateID,
                                        std::string_view name) const override;

    virtual InterfaceHandle registerDataSink(LocalFederateId federateID,
                                             std::string_view name) override final;

    virtual InterfaceHandle getDataSink(LocalFederateId federateID,
                                        std::string_view name) const override final;

    virtual InterfaceHandle registerFilter(std::string_view filterName,
                                           std::string_view type_in,
                                           std::string_view type_out) override;
    virtual InterfaceHandle registerCloningFilter(std::string_view filterName,
                                                  std::string_view type_in,
                                                  std::string_view type_out) override;
    virtual InterfaceHandle getFilter(std::string_view name) const override;
    virtual InterfaceHandle getTranslator(std::string_view name) const override;
    virtual void addDependency(LocalFederateId federateID, std::string_view federateName) override;
    virtual void linkEndpoints(std::string_view source, std::string_view dest) override;
    virtual void addAlias(std::string_view interfaceKey, std::string_view alias) override;
    virtual void makeConnections(const std::string& file) override;
    virtual void dataLink(std::string_view source, std::string_view target) override;
    virtual void addSourceFilterToEndpoint(std::string_view filter,
                                           std::string_view endpoint) override;
    virtual void addDestinationFilterToEndpoint(std::string_view filter,
                                                std::string_view endpoint) override;
    virtual void send(InterfaceHandle sourceHandle, const void* data, uint64_t length) override;
    virtual void
        sendAt(InterfaceHandle sourceHandle, const void* data, uint64_t length, Time time) override;
    virtual void sendTo(InterfaceHandle sourceHandle,
                        const void* data,
                        uint64_t length,
                        std::string_view destination) override;
    virtual void sendToAt(InterfaceHandle sourceHandle,
                          const void* data,
                          uint64_t length,
                          std::string_view destination,
                          Time time) override;
    virtual void sendMessage(InterfaceHandle sourceHandle,
                             std::unique_ptr<Message> message) override;
    virtual uint64_t receiveCount(InterfaceHandle destination) override;
    virtual std::unique_ptr<Message> receive(InterfaceHandle destination) override;
    virtual std::unique_ptr<Message> receiveAny(LocalFederateId federateID,
                                                InterfaceHandle& endpoint_id) override;
    virtual uint64_t receiveCountAny(LocalFederateId federateID) override;
    virtual void logMessage(LocalFederateId federateID,
                            int logLevel,
                            std::string_view messageToLog) override;
    virtual void setFilterOperator(InterfaceHandle filter,
                                   std::shared_ptr<FilterOperator> callback) override;
    virtual void setTranslatorOperator(InterfaceHandle translator,
                                       std::shared_ptr<TranslatorOperator> callback) override;
    virtual void setFederateOperator(LocalFederateId federateID,
                                     std::shared_ptr<FederateOperator> callback) override;
    /** get the local identifier for the core*/
    virtual const std::string& getIdentifier() const override;
    virtual const std::string& getAddress() const override;

    /** set the core logging level*/
    virtual void setLoggingLevel(int logLevel) override;
    virtual void setLoggingCallback(
        LocalFederateId federateID,
        std::function<void(int, std::string_view, std::string_view)> logFunction) override;

    virtual void setLogFile(std::string_view lfile) override;

    virtual std::string query(std::string_view target,
                              std::string_view queryStr,
                              HelicsSequencingModes mode) override;
    virtual void setQueryCallback(LocalFederateId federateID,
                                  std::function<std::string(std::string_view)> queryFunction,
                                  int order) override;
    virtual void setGlobal(std::string_view valueName, std::string_view value) override;
    virtual void sendCommand(std::string_view target,
                             std::string_view commandStr,
                             std::string_view source,
                             HelicsSequencingModes mode) override;
    virtual std::pair<std::string, std::string> getCommand(LocalFederateId federateID) override;

    virtual std::pair<std::string, std::string> waitCommand(LocalFederateId federateID) override;

    virtual bool connect() override;
    virtual bool isConnected() const override;
    virtual void disconnect() override;
    virtual bool waitForDisconnect(
        std::chrono::milliseconds msToWait = std::chrono::milliseconds(0)) const override;

    /** set the local information field of the interface*/
    virtual void setInterfaceInfo(InterfaceHandle handle, std::string_view info) override;
    /** get the local information field of the interface*/
    virtual const std::string& getInterfaceInfo(InterfaceHandle handle) const override;

    virtual void setInterfaceTag(InterfaceHandle handle,
                                 std::string_view tag,
                                 std::string_view value) override;
    virtual const std::string& getInterfaceTag(InterfaceHandle handle,
                                               std::string_view tag) const override;

    virtual void
        setFederateTag(LocalFederateId fid, std::string_view tag, std::string_view value) override;
    virtual const std::string& getFederateTag(LocalFederateId fid,
                                              std::string_view tag) const override;

  private:
    static const std::string emptyString;
};

}  // namespace helics
