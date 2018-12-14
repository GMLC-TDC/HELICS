/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once
#include "../common/DualMappedPointerVector.hpp"
#include "../common/GuardedTypes.hpp"
#include "EndpointInfo.hpp"
#include "NamedInputInfo.hpp"
#include "PublicationInfo.hpp"
#include <atomic>

/** @file container for keeping the set of different interfaces information for a federate
 */
namespace helics
{
class InterfaceInfo
{
  public:
    InterfaceInfo () = default;
    const PublicationInfo *getPublication (const std::string &pubName) const;
    const PublicationInfo *getPublication (interface_handle handle_) const;
    PublicationInfo *getPublication (const std::string &pubName);
    PublicationInfo *getPublication (interface_handle handle_);
    const NamedInputInfo *getInput (const std::string &ciName) const;
    const NamedInputInfo *getInput (interface_handle handle_) const;
    NamedInputInfo *getInput (const std::string &ciName);
    NamedInputInfo *getInput (interface_handle handle_);
    const EndpointInfo *getEndpoint (const std::string &endpointName) const;
    const EndpointInfo *getEndpoint (interface_handle handle_) const;
    EndpointInfo *getEndpoint (const std::string &endpointName);
    EndpointInfo *getEndpoint (interface_handle handle_);

    void createPublication (interface_handle handle,
                            const std::string &key,
                            const std::string &type,
                            const std::string &units);
    void createInput (interface_handle handle,
                      const std::string &key,
                      const std::string &type,
                      const std::string &units);
    void createEndpoint (interface_handle handle, const std::string &key, const std::string &type);

    auto getEndpoints () { return endpoints.lock (); }
    auto getPublications () { return publications.lock (); }
    auto getInputs () { return inputs.lock (); }
    auto getEndpoints () const { return endpoints.lock_shared (); }
    auto getPublications () const { return publications.lock_shared (); }
    auto getInputs () const { return inputs.lock_shared (); }
    auto cgetEndpoints () const { return endpoints.lock_shared (); }
    auto cgetPublications () const { return publications.lock_shared (); }
    auto cgetInputs () const { return inputs.lock_shared (); }
    /** set the global id of the federate for use in the interfaces*/
    void setGlobalId (global_federate_id newglobalId) { global_id = newglobalId; }
    /** set the change update flag which controls when a subscription is updated*/
    void setChangeUpdateFlag (bool updateFlag);
    /** get the current value of the change update flag*/
    bool getChangeUpdateFlag () const { return only_update_on_change; }
    /** set a property on a specific interface*/
    bool setInputProperty (interface_handle id, int option, bool value);
    bool setPublicationProperty (interface_handle id, int option, bool value);
    bool setEndpointProperty (interface_handle id, int option, bool value);
    /** check the interfaces for specific issues*/
    std::vector<std::pair<int, std::string>> checkInterfacesForIssues ();

  private:
        std::atomic<global_federate_id> global_id;
    bool only_update_on_change{
      false};  //!< flag indicating that subscriptions values should only be updated on change
    shared_guarded<DualMappedPointerVector<PublicationInfo, std::string, interface_handle>>
      publications;  //!< storage for all the publications
    shared_guarded<DualMappedPointerVector<EndpointInfo, std::string, interface_handle>>
      endpoints;  //!< storage for all the endpoints
    shared_guarded<DualMappedPointerVector<NamedInputInfo, std::string, interface_handle>>
      inputs;  //!< storage for all the endpoints
};
}  // namespace helics
