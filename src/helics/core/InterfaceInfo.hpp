/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once
#include "../common/GuardedTypes.hpp"
#include "EndpointInfo.hpp"
#include "FederateIdExtra.hpp"
#include "InputInfo.hpp"
#include "PublicationInfo.hpp"
#include "gmlc/containers/DualMappedPointerVector.hpp"
#include "nlohmann/json_fwd.hpp"

#include <atomic>
#include <string>
#include <utility>
#include <vector>

/** @file InterfaceInfo.hpp container for keeping the set of different interfaces information for a
 * federate
 */
namespace helics {
/** generic class for holding information about interfaces for a core federate structure*/
class InterfaceInfo {
  public:
    InterfaceInfo() = default;
    const PublicationInfo* getPublication(const std::string& pubName) const;
    const PublicationInfo* getPublication(InterfaceHandle handle) const;
    PublicationInfo* getPublication(const std::string& pubName);
    PublicationInfo* getPublication(InterfaceHandle handle);
    const InputInfo* getInput(const std::string& inputName) const;
    const InputInfo* getInput(InterfaceHandle handle) const;
    InputInfo* getInput(const std::string& inputName);
    InputInfo* getInput(InterfaceHandle handle);
    const EndpointInfo* getEndpoint(const std::string& endpointName) const;
    const EndpointInfo* getEndpoint(InterfaceHandle handle) const;
    EndpointInfo* getEndpoint(const std::string& endpointName);
    EndpointInfo* getEndpoint(InterfaceHandle handle);

    void createPublication(InterfaceHandle handle,
                           std::string_view key,
                           std::string_view type,
                           std::string_view units,
                           std::uint16_t flags);
    void createInput(InterfaceHandle handle,
                     std::string_view key,
                     std::string_view type,
                     std::string_view units,
                     std::uint16_t flags);
    void createEndpoint(InterfaceHandle handle,
                        std::string_view endpointName,
                        std::string_view type,
                        std::uint16_t flags);

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
    void setGlobalId(GlobalFederateId newglobalId) { global_id = newglobalId; }
    /** set the change update flag which controls when a subscription is updated*/
    void setChangeUpdateFlag(bool updateFlag);
    /** get the current value of the change update flag*/
    bool getChangeUpdateFlag() const { return only_update_on_change; }
    /** set a property on a specific interface*/
    bool setInputProperty(InterfaceHandle hid, int32_t option, int32_t value);
    bool setPublicationProperty(InterfaceHandle hid, int32_t option, int32_t value);
    bool setEndpointProperty(InterfaceHandle hid, int32_t option, int32_t value);
    /** get properties for an interface*/
    int32_t getInputProperty(InterfaceHandle hid, int32_t option) const;
    int32_t getPublicationProperty(InterfaceHandle hid, int32_t option) const;
    int32_t getEndpointProperty(InterfaceHandle hid, int32_t option) const;

    /** check the interfaces for specific issues*/
    std::vector<std::pair<int, std::string>> checkInterfacesForIssues();
    /** generate a configuration script for the interfaces*/
    void generateInferfaceConfig(nlohmann::json& base) const;
    /** load a dependency graph for the interfaces*/
    void generateDataFlowGraph(nlohmann::json& base) const;
    /** generate a list of unconnected interfaces*/
    void getUnconnectedInterfaces(nlohmann::json& base) const;
    /** reset the interfaceInfo to a new state*/
    void reset();
    /** disconnect a federate from communications */
    void disconnectFederate(GlobalFederateId fedToDisconnect, Time disconnectTime);

  private:
    std::atomic<GlobalFederateId> global_id;
    /// flag indicating that subscriptions values should only be updated on change
    bool only_update_on_change{false};
    shared_guarded<
        gmlc::containers::DualMappedPointerVector<PublicationInfo, std::string, InterfaceHandle>>
        publications;  //!< storage for all the publications
    shared_guarded<
        gmlc::containers::DualMappedPointerVector<EndpointInfo, std::string, InterfaceHandle>>
        endpoints;  //!< storage for all the endpoints
    shared_guarded<
        gmlc::containers::DualMappedPointerVector<InputInfo, std::string, InterfaceHandle>>
        inputs;  //!< storage for all the endpoints
};
}  // namespace helics
