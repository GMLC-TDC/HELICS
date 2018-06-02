/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once
#include "../common/GuardedTypes.hpp"
#include "../common/DualMappedPointerVector.hpp"
#include "SubscriptionInfo.hpp"
#include "PublicationInfo.hpp"
#include "EndpointInfo.hpp"
#include <atomic>

/** @file container for keeping the set of different interfaces information for a federate
*/
namespace helics
{
    class InterfaceInfo
    {
    public:
        InterfaceInfo() = default;
        const SubscriptionInfo *getSubscription(const std::string &subName) const;
        const SubscriptionInfo *getSubscription(handle_id_t handle_) const;
        SubscriptionInfo *getSubscription(const std::string &subName);
        SubscriptionInfo *getSubscription(handle_id_t handle_);
        const PublicationInfo *getPublication(const std::string &pubName) const;
        const PublicationInfo *getPublication(handle_id_t handle_) const;
        PublicationInfo *getPublication(const std::string &pubName);
        PublicationInfo *getPublication(handle_id_t handle_);
        const EndpointInfo *getEndpoint(const std::string &endpointName) const;
        const EndpointInfo *getEndpoint(handle_id_t handle_) const;
        EndpointInfo *getEndpoint(const std::string &endpointName);
        EndpointInfo *getEndpoint(handle_id_t handle_);

        void createSubscription(handle_id_t handle,
            const std::string &key,
            const std::string &type,
            const std::string &units,
            handle_check_mode check_mode);
        void createPublication(handle_id_t handle,
            const std::string &key,
            const std::string &type,
            const std::string &units);
        void createEndpoint(handle_id_t handle, const std::string &key, const std::string &type);

        auto getEndpoints()
        {
            return endpoints.lock();
        }
        auto getSubscriptions()
        {
            return subscriptions.lock();
        }
        auto getPublications()
        {
            return publications.lock();
        }
        auto getEndpoints() const
        {
            return endpoints.lock_shared();
        }
        auto getSubscriptions() const
        {
            return subscriptions.lock_shared();
        }
        auto getPublications() const
        {
            return publications.lock_shared();
        }
        auto cgetEndpoints() const
        {
            return endpoints.lock_shared();
        }
        auto cgetSubscriptions() const
        {
            return subscriptions.lock_shared();
        }
        auto cgetPublications() const
        {
            return publications.lock_shared();
        }
        void setGlobalId(global_federate_id_t newglobalId)
        {
            global_id = newglobalId;
        }
        void setChangeUpdateFlag(bool updateFlag);
    private:
        std::atomic<global_federate_id_t> global_id;
        bool only_update_on_change{ false };  //!< flag indicating that subscriptions values should only be updated on change
        shared_guarded<DualMappedPointerVector<SubscriptionInfo, std::string, handle_id_t>>
            subscriptions;  //!< storage for all the subscriptions
        shared_guarded<DualMappedPointerVector<PublicationInfo, std::string, handle_id_t>>
            publications;  //!< storage for all the publications
        shared_guarded<DualMappedPointerVector<EndpointInfo, std::string, handle_id_t>>
            endpoints;  //!< storage for all the endpoints
    };
} // namespace helics

