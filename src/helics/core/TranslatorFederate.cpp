/*
Copyright (c) 2017-2022,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "TranslatorFederate.hpp"

#include "../common/JsonProcessingFunctions.hpp"
#include "BasicHandleInfo.hpp"
#include "HandleManager.hpp"
#include "TimeCoordinatorProcessing.hpp"
#include "coreTypeOperations.hpp"
#include "flagOperations.hpp"
#include "helicsVersion.hpp"
#include "helics_definitions.hpp"
#include "queryHelpers.hpp"

#include <cassert>

namespace helics {

TranslatorFederate::TranslatorFederate(GlobalFederateId fedID,
                               std::string name,
                               GlobalBrokerId coreID,
                               Core* /*core*/):
    mFedID(fedID),
    mCoreID(coreID), mName(std::move(name)), /*mCore(core),*/
    mCoord([this](const ActionMessage& msg) { routeMessage(msg); })
{
    mCoord.source_id = fedID;
    mCoord.setOptionFlag(helics::defs::Flags::EVENT_TRIGGERED, true);
    mCoord.specifyNonGranting(true);
    mCoord.specifyNonGranting(true);
}

TranslatorFederate::~TranslatorFederate()
{
    mHandles = {nullptr};
    current_state = {HELICS_CREATED};
    // The interface_handle used is here is usually referencing an endpoint

    mQueueMessage = nullptr;
    mQueueMessageMove = nullptr;
    mSendMessage = nullptr;
    mSendMessageMove = nullptr;

    mDeliverMessage = nullptr;

    mLogger = nullptr;
    mGetAirLock = nullptr;


    translators.clear();
}

void TranslatorFederate::routeMessage(const ActionMessage& msg)
{
    if (mSendMessage) {
        mQueueMessage(msg);
    }
}

/** process any filter or route the message*/
void TranslatorFederate::processMessageFilter(ActionMessage& cmd)
{
    
}

void TranslatorFederate::generateProcessMarker(GlobalFederateId fid, uint32_t pid, Time returnTime)
{
    // nothing further to process
    auto fid_index = fid.baseValue();
    
}

void TranslatorFederate::acceptProcessReturn(GlobalFederateId fid, uint32_t pid)
{
    // nothing further to process
    auto fid_index = fid.baseValue();
    
}

void TranslatorFederate::generateDestProcessMarker(GlobalFederateId fid, uint32_t pid, Time returnTime)
{
    // nothing further to process
    auto fid_index = fid.baseValue();
}

void TranslatorFederate::acceptDestProcessReturn(GlobalFederateId fid, uint32_t pid)
{
    // nothing further to process
    auto fid_index = fid.baseValue();
    
}

/** process a filter message return*/
void TranslatorFederate::processFilterReturn(ActionMessage& cmd)
{
    auto* handle = mHandles->getEndpoint(cmd.dest_handle);
    if (handle == nullptr) {
        return;
    }

    auto mid = cmd.sequenceID;
    auto fid = handle->getFederateId();
    auto fid_index = fid.baseValue();

    
}


std::pair<ActionMessage&, bool> TranslatorFederate::executeTranslator(ActionMessage& command,
                                                              TranslatorInfo* filt)
{
    
    return {command, true};
}







void TranslatorFederate::handleMessage(ActionMessage& command)
{
    auto proc_result = processCoordinatorMessage(command, &mCoord, current_state, false, mFedID);

    if (std::get<2>(proc_result) && current_state == HELICS_EXECUTING) {
        mCoord.disconnect();
        ActionMessage disconnect(CMD_DISCONNECT);
        disconnect.source_id = mFedID;
        disconnect.dest_id = parent_broker_id;
        mQueueMessage(disconnect);
    }
    if (current_state != std::get<0>(proc_result)) {
        current_state = std::get<0>(proc_result);
        switch (current_state) {
            case HELICS_INITIALIZING:
                mCoord.enteringExecMode(IterationRequest::NO_ITERATIONS);
                {
                    ActionMessage echeck{CMD_EXEC_CHECK};
                    echeck.dest_id = mFedID;
                    echeck.source_id = mFedID;
                    handleMessage(echeck);
                }
                break;
            case HELICS_EXECUTING:
                mCoord.timeRequest(Time::maxVal(),
                                   IterationRequest::NO_ITERATIONS,
                                   Time::maxVal(),
                                   Time::maxVal());
                break;
            case HELICS_FINISHED:
                break;
            case HELICS_ERROR: {
                std::string errorString;
                if (command.payload.empty()) {
                    errorString = commandErrorString(command.messageID);
                    if (errorString == "unknown") {
                        errorString += " code:" + std::to_string(command.messageID);
                    }
                } else {
                    errorString = command.payload.to_string();
                }
                if (mLogger) {
                    mLogger(HELICS_LOG_LEVEL_ERROR, mName, errorString);
                }
            } break;
            default:
                break;
        }
    }

    switch (std::get<1>(proc_result)) {
        case MessageProcessingResult::CONTINUE_PROCESSING:
            break;
        case MessageProcessingResult::REPROCESS_MESSAGE:
            if (command.dest_id != mFedID) {
                mSendMessage(command);
                return;
            }
            return handleMessage(command);
        case MessageProcessingResult::DELAY_MESSAGE:
        default:
            return;
    }
    switch (command.action()) {
        case CMD_CLOSE_INTERFACE: {
            auto* tran = translators.find(command.getSource());
            if (tran != nullptr) {
                
            }
        } break;
        
        case CMD_REMOVE_ENDPOINT: {
            auto* tranI = getTranslatorInfo(command.getDest());
            if (tranI != nullptr) {
                tranI->getEndpointInfo()->removeTarget(command.getSource());
            }
        } break;
        case CMD_REG_ENDPOINT: {
           
        } break;

        case CMD_ADD_ENDPOINT: {
            auto* tranI = getTranslatorInfo(mFedID, command.dest_handle);
            if (tranI != nullptr) {
                if (checkActionFlag(command, destination_target)) {
                    tranI->getEndpointInfo()->addDestinationTarget(command.getSource(),command.payload.to_string(),command.getString(typeStringLoc));
                } else {
                    tranI->getEndpointInfo()->addSourceTarget(command.getSource(),
                                                              command.payload.to_string(),
                                                              command.getString(typeStringLoc));
                }
                if (!checkActionFlag(command, error_flag)) {
                    mCoord.addDependency(command.source_id);
                }
            }

            
        } break;
        case CMD_CORE_CONFIGURE:
            if (command.messageID == UPDATE_TRANSLATOR_OPERATOR) {
                auto* tranI = getTranslatorInfo(mFedID, command.source_handle);
                if (tranI != nullptr) {
                    auto op = mGetAirLock(command.counter).try_unload();
                    if (op) {
                        auto M = std::any_cast<std::shared_ptr<TranslatorOperator>>(std::move(*op));
                        tranI->tranOp = std::move(M);
                    }
                }
            }
            break;
        default:
            break;
    }
}

TranslatorInfo* TranslatorFederate::createTranslator(GlobalBrokerId dest,
                                         InterfaceHandle handle,
                                         const std::string& key,
                                         const std::string& endpointType,
                                         const std::string& units)
{
    auto tran = std::make_unique<TranslatorInfo>(
        GlobalHandle{(dest == parent_broker_id || dest == mCoreID) ? GlobalBrokerId(mFedID) : dest, handle},
                                             key,
                                             endpointType,
                                             units);

    auto cid = tran->id;
    auto* retTarget = tran.get();
    // auto actualKey = key;

    // if (actualKey.empty()) {
    // actualKey = "sFilter_";
    // actualKey.append(std::to_string(handle.baseValue()));
    // }

    // if (filt->core_id == mFedID) {
    translators.insert(cid, std::move(tran));
    //} else {
    // actualKey.push_back('_');
    //  actualKey.append(std::to_string(cid.baseValue()));
    //    filters.insert({cid, handle}, std::move(filt));
    //}

    return retTarget;
}


TranslatorInfo* TranslatorFederate::getTranslatorInfo(GlobalHandle id)
{
    return translators.find(id);
}

TranslatorInfo* TranslatorFederate::getTranslatorInfo(GlobalFederateId fed, InterfaceHandle handle)
{
    if (fed == parent_broker_id || fed == mCoreID) {
        fed = mFedID;
    }
    return translators.find(GlobalHandle{fed, handle});
}

const TranslatorInfo* TranslatorFederate::getTranslatorInfo(GlobalFederateId fed, InterfaceHandle handle) const
{
    if (fed == parent_broker_id || fed == mCoreID) {
        fed = mFedID;
    }
    return translators.find(GlobalHandle{fed, handle});
}

std::string TranslatorFederate::query(const std::string& queryStr) const
{
    if (queryStr == "exists") {
        return "true";
    }
    if (queryStr == "version") {
        return versionString;
    }
    if (queryStr == "isinit") {
        return "true";
    }
    if (queryStr == "state") {
        return fedStateString(current_state);
    }

    if (queryStr == "publications" || queryStr == "inputs" || queryStr == "filtered_endpoints" ||
        queryStr == "endpoints" || queryStr == "subscriptions") {
        return "[]";
    }

    if (queryStr == "interfaces") {
        return "[]";
    }

    if (queryStr == "dependencies") {
        return generateStringVector(mCoord.getDependencies(),
                                    [](auto& dep) { return std::to_string(dep.baseValue()); });
    }
    if (queryStr == "current_time") {
        return mCoord.printTimeStatus();
    }
    if (queryStr == "current_state") {
        Json::Value base;
        base["name"] = mName;
        base["id"] = mFedID.baseValue();
        base["parent"] = mCoreID.baseValue();
        base["state"] = fedStateString(current_state);
        base["publications"] = 0;
        base["input"] = 0;
        base["endpoints"] = 0;
        base["granted_time"] = static_cast<double>(mCoord.getGrantedTime());
        return fileops::generateJsonString(base);
    }
    if (queryStr == "global_state") {
        Json::Value base;
        base["name"] = mName;
        base["id"] = mFedID.baseValue();
        base["parent"] = mCoreID.baseValue();
        base["state"] = fedStateString(current_state);
        return fileops::generateJsonString(base);
    }
    if (queryStr == "global_time_debugging") {
        Json::Value base;
        base["name"] = mName;
        base["id"] = mFedID.baseValue();
        base["parent"] = mCoreID.baseValue();
        base["state"] = fedStateString(current_state);
        mCoord.generateDebuggingTimeInfo(base);
        return fileops::generateJsonString(base);
    }
    if (queryStr == "timeconfig") {
        Json::Value base;
        mCoord.generateConfig(base);
        return fileops::generateJsonString(base);
    }
    if (queryStr == "config") {
        Json::Value base;
        mCoord.generateConfig(base);
        return fileops::generateJsonString(base);
    }
    if (queryStr == "dependents") {
        return generateStringVector(mCoord.getDependents(),
                                    [](auto& dep) { return std::to_string(dep.baseValue()); });
    }
    if (queryStr == "data_flow_graph") {
        Json::Value base;
        base["name"] = mName;
        base["id"] = mFedID.baseValue();
        base["parent"] = mCoreID.baseValue();
        if (translators.size() > 0) {
            base["filters"] = Json::arrayValue;
            for (const auto& trans : translators) {
                Json::Value tran;
                tran["id"] = trans->id.handle.baseValue();
                tran["name"] = trans->key;
                /*
                tran["source_targets"] = generateStringVector(filt->sourceTargets, [](auto& dep) {
                    return std::to_string(dep.fed_id.baseValue()) +
                        "::" + std::to_string(dep.handle.baseValue());
                });
                tran["dest_targets"] = generateStringVector(filt->destTargets, [](auto& dep) {
                    return std::to_string(dep.fed_id.baseValue()) +
                        "::" + std::to_string(dep.handle.baseValue());
                });
                */
                tran["translators"].append(std::move(tran));
            }
        }
        return fileops::generateJsonString(base);
    }
    if (queryStr == "global_time") {
        Json::Value base;
        base["name"] = mName;
        base["id"] = mFedID.baseValue();
        base["parent"] = mCoreID.baseValue();
        base["granted_time"] = static_cast<double>(mCoord.getGrantedTime());
        base["send_time"] = static_cast<double>(mCoord.allowedSendTime());
        return fileops::generateJsonString(base);
    }
    if (queryStr == "dependency_graph") {
        Json::Value base;
        base["name"] = mName;
        base["id"] = mFedID.baseValue();
        base["parent"] = mCoreID.baseValue();
        base["dependents"] = Json::arrayValue;
        for (auto& dep : mCoord.getDependents()) {
            base["dependents"].append(dep.baseValue());
        }
        base["dependencies"] = Json::arrayValue;
        for (auto& dep : mCoord.getDependencies()) {
            base["dependencies"].append(dep.baseValue());
        }
        return fileops::generateJsonString(base);
    }

    return "#invalid";
}

bool TranslatorFederate::hasActiveTimeDependencies() const
{
    return mCoord.hasActiveTimeDependencies();
}
}  // namespace helics
