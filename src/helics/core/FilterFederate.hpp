/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "FilterCoordinator.hpp"
#include "FilterInfo.hpp"
#include "TimeCoordinator.hpp"
#include "global_federate_id.hpp"
#include "gmlc/containers/DualMappedPointerVector.hpp"
#include "Core.hpp"
#include <functional>
#include <map>
#include <set>
#include <memory>
#include "../common/JsonBuilder.hpp"

namespace helics {
class HandleManager;
class ActionMessage;
class BasicHandleInfo;

class FilterFederate {
  private:
    global_federate_id mFedID;
    global_broker_id mCoreID;
    const std::string mName;
    Core* mCore;
    HandleManager* mHandles;
    /// map of all local filters
    std::map<interface_handle, std::unique_ptr<FilterCoordinator>> filterCoord;
    // The interface_handle used is here is usually referencing an endpoint
    /// storage for all the filters
    gmlc::containers::DualMappedPointerVector<FilterInfo, std::string, global_handle> filters;

    TimeCoordinator mCoord;

    std::function<void(ActionMessage&)> mQueueMessage;
    std::function<void(ActionMessage&&)> mQueueMessageMove;
    std::function<void(ActionMessage&)> mSendMessage;
    std::function<void(ActionMessage&&)> mSendMessageMove;

    std::function<void(ActionMessage&)> mDeliverMessage;

    std::function<void(int, const std::string&, const std::string&)> mLogger;
    /// sets of ongoing filtered messages
    std::map<int32_t, std::set<int32_t>>
        ongoingFilterProcesses;
    /// sets of ongoing destination filter processing
    std::map<int32_t, std::set<int32_t>>
        ongoingDestFilterProcesses;
    /** counter for the number of messages that have been sent, nothing magical about 54 just a number bigger than 1 to prevent confusion */
    std::atomic<int32_t> messageCounter{
        54};  
  public:
    FilterFederate(global_federate_id fedID, const std::string &name, global_broker_id coreID, Core *core):
        mFedID(fedID), mCoreID(coreID), mName(name),mCore(core){};
    /** process any filter or route the message*/
    void processMessageFilter(ActionMessage& cmd);
    /** process a filter message return*/
    void processFilterReturn(ActionMessage& cmd);
    /** process a destination filter message return*/
    void processDestFilterReturn(ActionMessage& command);
    /** create a filter */
    FilterInfo* createFilter(global_broker_id dest,
                             interface_handle handle,
                             const std::string& key,
                             const std::string& type_in,
                             const std::string& type_out,
                             bool cloning);

    void setCallbacks(std::function<void(ActionMessage&)> queueMessage,
                      std::function<void(ActionMessage&&)> queueMessageMove,
                      std::function<void(ActionMessage&)> sendMessage,
                      std::function<void(ActionMessage&&)> sendMessageMove)
    {
        mQueueMessage = std::move(queueMessage);
        mQueueMessageMove = std::move(queueMessageMove);
        mSendMessage = std::move(sendMessage);
        mSendMessageMove = std::move(sendMessageMove);
    }

    void setLogger(std::function<void(int, const std::string&, const std::string&)> logger)
    {
        mLogger = std::move(logger);
    }

    void setDeliver(std::function<void(ActionMessage&)> deliverMessage)
    {
        mDeliverMessage = std::move(deliverMessage);
    }
    void organizeFilterOperations();

    void handleMessage(ActionMessage& command);

    void processFilterInfo(ActionMessage& command);

    ActionMessage& processMessage(ActionMessage& command, const BasicHandleInfo *handle);

    void destinationProcessMessage(ActionMessage& command, const BasicHandleInfo* handle);

    void addFilteredEndpoint(Json::Value& block, global_federate_id fed) const;

    void setHandleManager(HandleManager* handles)
    {
        mHandles = handles;}

      private:
    /** get a filtering function object*/
    FilterCoordinator* getFilterCoordinator(interface_handle handle);
};
}  // namespace helics
