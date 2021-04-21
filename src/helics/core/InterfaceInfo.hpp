/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once
#include "../common/GuardedTypes.hpp"
#include "EndpointInfo.hpp"
#include "InputInfo.hpp"
#include "PublicationInfo.hpp"
#include "federate_id_extra.hpp"
#include "gmlc/containers/DualMappedPointerVector.hpp"

#include "json/forwards.h"
#include <atomic>
#include <string>
#include <utility>
#include <vector>

/** @file container for keeping the set of different interfaces information for a federate
 */
namespace helics {
/** generic class for holding information about interfaces for a core federate structure*/
class InterfaceInfo {
  public:
    InterfaceInfo() = default;
    const PublicationInfo* getPublication(const std::string& pubName) const;
    const PublicationInfo* getPublication(interface_handle handle) const;
    PublicationInfo* getPublication(const std::string& pubName);
    PublicationInfo* getPublication(interface_handle handle);
    const InputInfo* getInput(const std::string& inputName) const;
    const InputInfo* getInput(interface_handle handle) const;
    InputInfo* getInput(const std::string& inputName);
    InputInfo* getInput(interface_handle handle);
    const EndpointInfo* getEndpoint(const std::string& endpointName) const;
    const EndpointInfo* getEndpoint(interface_handle handle) const;
    EndpointInfo* getEndpoint(const std::string& endpointName);
    EndpointInfo* getEndpoint(interface_handle handle);

    void createPublication(interface_handle handle,
                           const std::string& key,
                           const std::string& type,
                           const std::string& units);
    void createInput(interface_handle handle,
                     const std::string& key,
                     const std::string& type,
                     const std::string& units);
    void createEndpoint(interface_handle handle,
                        const std::string& endpointName,
                        const std::string& type);

    auto getEndpoints() { return endpoints.lock(); }
    auto getPublications() { return publications.lock(); }
    auto getInputs() { return inputs.lock(); }
    auto getEndpoints() const { return endpoints.lock_shared(); }
    auto getPublications() const { return publications.lock_shared(); }
    auto getInputs() const { return inputs.lock_shared(); }
    auto cgetEndpoints() const { return endpoints.lock_shared(); }
    auto cgetPublications() const { return publications.lock_shared(); }
    auto cgetInputs() const { return inputs.lock_shared(); }
    /** set the global id of the federate for use in the interfaces*/
    void setGlobalId(global_federate_id newglobalId) { global_id = newglobalId; }
    /** set the change update flag which controls when a subscription is updated*/
    void setChangeUpdateFlag(bool updateFlag);
    /** get the current value of the change update flag*/
    bool getChangeUpdateFlag() const { return only_update_on_change; }
    /** set a property on a specific interface*/
    bool setInputProperty(interface_handle id, int32_t option, int32_t value);
    bool setPublicationProperty(interface_handle id, int32_t option, int32_t value);
    bool setEndpointProperty(interface_handle id, int32_t option, int32_t value);
    /** get properties for an interface*/
    int32_t getInputProperty(interface_handle id, int32_t option) const;
    int32_t getPublicationProperty(interface_handle id, int32_t option) const;
    int32_t getEndpointProperty(interface_handle id, int32_t option) const;

    /** check the interfaces for specific issues*/
    std::vector<std::pair<int, std::string>> checkInterfacesForIssues();
    /** generate a configuration script for the interfaces*/
    void generateInferfaceConfig(Json::Value& base) const;
    /** load a dependency graph for the interfaces*/
    void GenerateDataFlowGraph(Json::Value& base) const;

  private:
    std::atomic<global_federate_id> global_id;
    bool only_update_on_change{
        false};  //!< flag indicating that subscriptions values should only be updated on change
    shared_guarded<
        gmlc::containers::DualMappedPointerVector<PublicationInfo, std::string, interface_handle>>
        publications;  //!< storage for all the publications
    shared_guarded<
        gmlc::containers::DualMappedPointerVector<EndpointInfo, std::string, interface_handle>>
        endpoints;  //!< storage for all the endpoints
    shared_guarded<
        gmlc::containers::DualMappedPointerVector<InputInfo, std::string, interface_handle>>
        inputs;  //!< storage for all the endpoints
};
}  // namespace helics
