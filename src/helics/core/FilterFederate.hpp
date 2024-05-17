/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "../common/JsonBuilder.hpp"
#include "Core.hpp"
#include "FederateIdExtra.hpp"
#include "FilterCoordinator.hpp"
#include "FilterInfo.hpp"
#include "TimeCoordinator.hpp"
#include "gmlc/containers/AirLock.hpp"
#include "gmlc/containers/MappedPointerVector.hpp"

#include <any>
#include <deque>
#include <functional>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace helics {
class HandleManager;
class ActionMessage;
class BasicHandleInfo;

class FilterFederate {
  private:
    GlobalFederateId mFedID;
    GlobalBrokerId mCoreID;
    const std::string mName;
    // Core* mCore{nullptr};
    TimeCoordinator mCoord;
    HandleManager* mHandles{nullptr};
    FederateStates current_state{FederateStates::CREATED};
    /// map of all local filters
    std::map<InterfaceHandle, std::unique_ptr<FilterCoordinator>> filterCoord;
    // The interface_handle used is here is usually referencing an endpoint

    std::function<void(const ActionMessage&)> mQueueMessage;
    std::function<void(ActionMessage&&)> mQueueMessageMove;
    std::function<void(const ActionMessage&)> mSendMessage;
    std::function<void(ActionMessage&&)> mSendMessageMove;

    std::function<void(ActionMessage&)> mDeliverMessage;

    std::function<void(int, std::string_view, std::string_view)> mLogger;
    std::function<gmlc::containers::AirLock<std::any>&(int)> mGetAirLock;
    std::deque<std::pair<int32_t, Time>> timeBlockProcesses;
    Time minReturnTime{Time::maxVal()};
    /// sets of ongoing filtered messages
    std::map<int32_t, std::set<int32_t>> ongoingFilterProcesses;
    /// sets of ongoing destination filter processing
    std::map<int32_t, std::set<int32_t>> ongoingDestFilterProcesses;
    /** counter for the number of messages that have been sent, nothing magical about 54 just a
     * number bigger than 1 to prevent confusion */
    std::atomic<int32_t> messageCounter{54};
    /// indicator to use a globalTimeCoordinator
    bool usingGlobalTime{false};
    /// storage for all the filters
    gmlc::containers::MappedPointerVector<FilterInfo, GlobalHandle> filters;
    // bool hasTiming{false};

  public:
    FilterFederate(GlobalFederateId fedID, std::string name, GlobalBrokerId coreID, Core* core);
    ~FilterFederate();
    /** process any filter or route the message*/
    void processMessageFilter(ActionMessage& cmd);
    /** process a filter message return*/
    void processFilterReturn(ActionMessage& cmd);
    /** process a destination filter message return*/
    void processDestFilterReturn(ActionMessage& command);
    /** create a filter */
    FilterInfo* createFilter(GlobalBrokerId dest,
                             InterfaceHandle handle,
                             std::string_view key,
                             std::string_view type_in,
                             std::string_view type_out,
                             bool cloning);

    void setCallbacks(std::function<void(const ActionMessage&)> queueMessage,
                      std::function<void(ActionMessage&&)> queueMessageMove,
                      std::function<void(const ActionMessage&)> sendMessage,
                      std::function<void(ActionMessage&&)> sendMessageMove)
    {
        mQueueMessage = std::move(queueMessage);
        mQueueMessageMove = std::move(queueMessageMove);
        mSendMessage = std::move(sendMessage);
        mSendMessageMove = std::move(sendMessageMove);
    }

    void setLogger(std::function<void(int, std::string_view, std::string_view)> logger)
    {
        mLogger = std::move(logger);
    }

    void setDeliver(std::function<void(ActionMessage&)> deliverMessage)
    {
        mDeliverMessage = std::move(deliverMessage);
    }

    void setAirLockFunction(std::function<gmlc::containers::AirLock<std::any>&(int)> getAirLock)
    {
        mGetAirLock = std::move(getAirLock);
    }
    void organizeFilterOperations();

    void handleMessage(ActionMessage& command);

    void processFilterInfo(ActionMessage& command);

    ActionMessage& processMessage(ActionMessage& command, const BasicHandleInfo* handle);
    /** process destination filters on the message and return true if the original command should be
     * delivered to a federate*/
    bool destinationProcessMessage(ActionMessage& command, const BasicHandleInfo* handle);

    void addFilteredEndpoint(nlohmann::json& block, GlobalFederateId fed) const;

    void setHandleManager(HandleManager* handles) { mHandles = handles; }

    std::string query(std::string_view queryStr) const;
    /** check if the filter federate has active time dependencies other than parent*/
    bool hasActiveTimeDependencies() const;

    void useGlobalTimeCoordinator(bool value)
    {
        usingGlobalTime = value;
        mCoord.globalTime = value;
    }

  private:
    void routeMessage(const ActionMessage& msg);
    /** get a filtering function object*/
    FilterCoordinator* getFilterCoordinator(InterfaceHandle handle);

    FilterInfo* getFilterInfo(GlobalHandle id);
    FilterInfo* getFilterInfo(GlobalFederateId fed, InterfaceHandle handle);
    const FilterInfo* getFilterInfo(GlobalFederateId fed, InterfaceHandle handle) const;
    /** run the destination filters associated with an endpoint*/
    void runCloningDestinationFilters(const FilterCoordinator* filt,
                                      const BasicHandleInfo* handle,
                                      const ActionMessage& command) const;
    void addTimeReturn(int32_t id, Time TimeVal);
    void clearTimeReturn(int32_t id);

    std::pair<ActionMessage&, bool> executeFilter(ActionMessage& command, FilterInfo* filt);
    void generateProcessMarker(GlobalFederateId fid, uint32_t pid, Time returnTime);
    void acceptProcessReturn(GlobalFederateId fid, uint32_t pid);

    void generateDestProcessMarker(GlobalFederateId fid, uint32_t pid, Time returnTime);
    void acceptDestProcessReturn(GlobalFederateId fid, uint32_t pid);
};
}  // namespace helics
