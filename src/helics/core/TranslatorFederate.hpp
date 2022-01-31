/*
Copyright (c) 2017-2022,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "../common/JsonBuilder.hpp"
#include "Core.hpp"
#include "GlobalFederateId.hpp"
#include "TimeCoordinator.hpp"
#include "TranslatorInfo.hpp"
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

class TranslatorFederate {
  private:
    GlobalFederateId mFedID;
    GlobalBrokerId mCoreID;
    const std::string mName;
    // Core* mCore{nullptr};
    TimeCoordinator mCoord;
    HandleManager* mHandles{nullptr};
    FederateStates current_state{HELICS_CREATED};

    std::function<void(const ActionMessage&)> mQueueMessage;
    std::function<void(ActionMessage&&)> mQueueMessageMove;
    std::function<void(const ActionMessage&)> mSendMessage;
    std::function<void(ActionMessage&&)> mSendMessageMove;

    std::function<void(ActionMessage&)> mDeliverMessage;

    std::function<void(int, std::string_view, std::string_view)> mLogger;
    std::function<gmlc::containers::AirLock<std::any>&(int)> mGetAirLock;

    Time minReturnTime{Time::maxVal()};

    /// storage for all the filters
    gmlc::containers::MappedPointerVector<TranslatorInfo, GlobalHandle> translators;
    // bool hasTiming{false};

  public:
    TranslatorFederate(GlobalFederateId fedID, std::string name, GlobalBrokerId coreID, Core* core);
    ~TranslatorFederate();
    /** create a translator */
    TranslatorInfo* createTranslator(GlobalBrokerId dest,
                                     InterfaceHandle handle,
                                     const std::string& key,
                                     const std::string& endpointType,
                                     const std::string& units);

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

    void handleMessage(ActionMessage& command);

    void setHandleManager(HandleManager* handles) { mHandles = handles; }

    std::string query(const std::string& queryStr) const;
    /** check if the filter federate has active time dependencies other than parent*/
    bool hasActiveTimeDependencies() const;

  private:
    void routeMessage(const ActionMessage& msg);
    /** get a filtering function object*/

    TranslatorInfo* getTranslatorInfo(GlobalHandle id);
    TranslatorInfo* getTranslatorInfo(GlobalFederateId fed, InterfaceHandle handle);
    const TranslatorInfo* getTranslatorInfo(GlobalFederateId fed, InterfaceHandle handle) const;

    void executeTranslator(ActionMessage& command, TranslatorInfo* trans);
};
}  // namespace helics