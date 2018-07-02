/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once
#include "../common/DualMappedPointerVector.hpp"
#include "../common/GuardedTypes.hpp"
#include "EndpointInfo.hpp"
#include "PublicationInfo.hpp"
#include "SubscriptionInfo.hpp"
#include "ControlInputInfo.hpp"
#include "ControlOutputInfo.hpp"
#include <atomic>

/** @file container for keeping the set of different interfaces information for a federate
 */
namespace helics
{
/** class containing all the interfaces to a federate*/
class InterfaceInfo
{
  public:
    InterfaceInfo () = default;
    const SubscriptionInfo *getSubscription (const std::string &subName) const;
    const SubscriptionInfo *getSubscription (Core::handle_id_t handle_) const;
    SubscriptionInfo *getSubscription (const std::string &subName);
    SubscriptionInfo *getSubscription (Core::handle_id_t handle_);
    const PublicationInfo *getPublication (const std::string &pubName) const;
    const PublicationInfo *getPublication (Core::handle_id_t handle_) const;
    PublicationInfo *getPublication (const std::string &pubName);
    PublicationInfo *getPublication (Core::handle_id_t handle_);
    const ControlOutputInfo *getControlOutput(const std::string &coName) const;
    const ControlOutputInfo *getControlOutput(Core::handle_id_t handle_) const;
    ControlOutputInfo *getControlOutput(const std::string &coName);
    ControlOutputInfo *getControlOutput(Core::handle_id_t handle_);
    const ControlInputInfo *getNamedInput(const std::string &ciName) const;
    const  ControlInputInfo *getNamedInput(Core::handle_id_t handle_) const;
    ControlInputInfo *getNamedInput(const std::string &ciName);
    ControlInputInfo *getNamedInput(Core::handle_id_t handle_);
    const EndpointInfo *getEndpoint (const std::string &endpointName) const;
    const EndpointInfo *getEndpoint (Core::handle_id_t handle_) const;
    EndpointInfo *getEndpoint (const std::string &endpointName);
    EndpointInfo *getEndpoint (Core::handle_id_t handle_);

    void createSubscription (Core::handle_id_t handle,
                             const std::string &key,
                             const std::string &type,
                             const std::string &units,
                             handle_check_mode check_mode);
    void createControlOutput(Core::handle_id_t handle,
        const std::string &key,
        const std::string &type,
        const std::string &units);
    void createPublication (Core::handle_id_t handle,
                            const std::string &key,
                            const std::string &type,
                            const std::string &units);
    void createControlInput(Core::handle_id_t handle,
        const std::string &key,
        const std::string &type,
        const std::string &units);
    void createEndpoint (Core::handle_id_t handle, const std::string &key, const std::string &type);

    auto getEndpoints () { return endpoints.lock (); }
    auto getSubscriptions () { return subscriptions.lock (); }
    auto getPublications () { return publications.lock (); }
    auto getControlOutputs() { return controlOutputs.lock(); }
    auto getControlInputs() { return controlInputs.lock(); }
    auto getEndpoints () const { return endpoints.lock_shared (); }
    auto getSubscriptions () const { return subscriptions.lock_shared (); }
    auto getPublications () const { return publications.lock_shared (); }
    auto getControlOutputs() const { return controlOutputs.lock_shared(); }
    auto getControlInputs() const { return controlInputs.lock_shared(); }
    auto cgetEndpoints () const { return endpoints.lock_shared (); }
    auto cgetSubscriptions () const { return subscriptions.lock_shared (); }
    auto cgetPublications () const { return publications.lock_shared (); }
    auto cgetControlOutputs() const { return controlOutputs.lock_shared(); }
    auto cgetControlInputs() const { return controlInputs.lock_shared(); }
    /** set the global id of the federate for use in the interfaces*/
    void setGlobalId (Core::federate_id_t newglobalId) { global_id = newglobalId; }
    /** set the change update flag which controls when a subscription is updated*/
    void setChangeUpdateFlag (bool updateFlag);

  private:
    std::atomic<Core::federate_id_t> global_id{invalid_fed_id};
    bool only_update_on_change{
      false};  //!< flag indicating that subscriptions values should only be updated on change
    shared_guarded<DualMappedPointerVector<SubscriptionInfo, std::string, Core::handle_id_t>>
      subscriptions;  //!< storage for all the subscriptions
    shared_guarded<DualMappedPointerVector<PublicationInfo, std::string, Core::handle_id_t>>
      publications;  //!< storage for all the publications
    shared_guarded<DualMappedPointerVector<EndpointInfo, std::string, Core::handle_id_t>>
      endpoints;  //!< storage for all the endpoints
    shared_guarded<DualMappedPointerVector<ControlOutputInfo, std::string, Core::handle_id_t>>
        controlOutputs;  //!< storage for all the endpoints
    shared_guarded<DualMappedPointerVector<ControlInputInfo, std::string, Core::handle_id_t>>
        controlInputs;  //!< storage for all the endpoints
};
}  // namespace helics
