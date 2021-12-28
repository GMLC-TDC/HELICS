/*
Copyright (c) 2017-2021,
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


    filters.clear();
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
    if (cmd.dest_id != mFedID) {
        mSendMessage(cmd);
    } else {
        // deal with local source filters
        auto* FiltI = getFilterInfo(cmd.getDest());
        if (FiltI != nullptr) {
            if ((!checkActionFlag(*FiltI, disconnected_flag)) && (FiltI->filterOp)) {
                if (FiltI->cloning) {
                    auto new_messages =
                        FiltI->filterOp->processVector(createMessageFromCommand(std::move(cmd)));
                    for (auto& msg : new_messages) {
                        if (msg) {
                            cmd = ActionMessage(std::move(msg));
                            mDeliverMessage(cmd);
                        }
                    }
                } else {
                    bool destFilter = (cmd.action() == CMD_SEND_FOR_DEST_FILTER_AND_RETURN);
                    bool returnToSender =
                        ((cmd.action() == CMD_SEND_FOR_FILTER_AND_RETURN) || destFilter);
                    auto source = cmd.getSource();
                    auto filterCounter = cmd.counter;
                    auto seqID = cmd.sequenceID;
                    if (FiltI->filterOp) {
                        auto tempMessage = createMessageFromCommand(std::move(cmd));
                        auto dest = tempMessage->dest;
                        tempMessage = FiltI->filterOp->process(std::move(tempMessage));

                        if (tempMessage) {
                            if (tempMessage->dest != dest && destFilter) {
                                // the destination was altered we need to start the process over
                                cmd = ActionMessage(std::move(tempMessage));
                                cmd.dest_id = parent_broker_id;
                                cmd.dest_handle = InterfaceHandle{};
                                mDeliverMessage(cmd);
                                cmd = CMD_IGNORE;
                            } else {
                                cmd = ActionMessage(std::move(tempMessage));
                            }
                        } else {
                            cmd = CMD_IGNORE;
                        }
                    }

                    if (!returnToSender) {
                        if (cmd.action() == CMD_IGNORE) {
                            return;
                        }
                        cmd.setSource(source);
                        cmd.dest_id = parent_broker_id;
                        cmd.dest_handle = InterfaceHandle();
                        mDeliverMessage(cmd);
                    } else {
                        cmd.setDestination(source);
                        cmd.counter = filterCounter;
                        cmd.sequenceID = seqID;
                        cmd.source_handle = FiltI->handle;
                        cmd.source_id = mFedID;
                        if (cmd.action() == CMD_IGNORE) {
                            cmd.setAction(destFilter ? CMD_NULL_DEST_MESSAGE : CMD_NULL_MESSAGE);

                            mDeliverMessage(cmd);
                            return;
                        }
                        cmd.setAction(destFilter ? CMD_DEST_FILTER_RESULT : CMD_FILTER_RESULT);

                        mDeliverMessage(cmd);
                    }
                }
            } else {
                // the filter didn't have a function or was deactivated but still was requested to
                // process
                bool destFilter = (cmd.action() == CMD_SEND_FOR_DEST_FILTER_AND_RETURN);
                bool returnToSender =
                    ((cmd.action() == CMD_SEND_FOR_FILTER_AND_RETURN) || destFilter);
                auto source = cmd.getSource();
                if (!returnToSender) {
                    cmd.setAction(CMD_SEND_MESSAGE);
                    cmd.dest_id = parent_broker_id;
                    cmd.dest_handle = InterfaceHandle();
                    mDeliverMessage(cmd);
                } else {
                    cmd.setDestination(source);
                    cmd.setAction(destFilter ? CMD_DEST_FILTER_RESULT : CMD_FILTER_RESULT);

                    cmd.source_handle = FiltI->handle;
                    cmd.source_id = mFedID;
                    mDeliverMessage(cmd);
                }
            }
        } else {
            assert(false);
            // this is an odd condition (not sure what to do yet)
            /*    m.dest_id = filtFunc->sourceOperators[ii].fed_id;
                m.dest_handle = filtFunc->sourceOperators[ii].handle;
                if ((ii < static_cast<int> (filtFunc->sourceOperators.size() - 1)) ||
                    (filtFunc->finalSourceFilter.fed_id != invalid_fed_id))
                {
                    m.setAction(CMD_SEND_FOR_FILTER_OPERATION);
                }
                else
                {
                    m.setAction(CMD_SEND_FOR_FILTER);
                }
                return m;
                */
        }
    }
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


std::pair<ActionMessage&, bool> TranslatorFederate::executeFilter(ActionMessage& command,
                                                              FilterInfo* filt)
{
    if (filt->core_id == mFedID) {
        if (filt->cloning) {
            // cloning filter returns a vector
            auto new_messages = filt->filterOp->processVector(createMessageFromCommand(command));
            for (auto& msg : new_messages) {
                if (msg) {
                    ActionMessage cmd(std::move(msg));
                    mDeliverMessage(cmd);
                }
            }
        } else {
            // deal with local source filters
            auto tempMessage = createMessageFromCommand(std::move(command));
            tempMessage = filt->filterOp->process(std::move(tempMessage));
            if (tempMessage) {
                command = ActionMessage(std::move(tempMessage));
            } else {
                // the filter dropped the message;
                command = CMD_IGNORE;
                return {command, false};
            }
        }
    } else if (filt->cloning) {
        ActionMessage cloneMessage(command);
        cloneMessage.setAction(CMD_SEND_FOR_FILTER);
        setActionFlag(cloneMessage, clone_flag);
        cloneMessage.dest_id = filt->core_id;
        cloneMessage.dest_handle = filt->handle;
        mSendMessage(cloneMessage);
    } else {
        command.dest_id = filt->core_id;
        command.dest_handle = filt->handle;

        return {command, false};
    }
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
            auto* filt = filters.find(command.getSource());
            if (filt != nullptr) {
                ActionMessage rem(CMD_REMOVE_FILTER);
                rem.source_handle = filt->handle;
                rem.source_id = filt->core_id;
                for (auto& target : filt->sourceTargets) {
                    rem.setDestination(target);
                    mSendMessage(rem);
                }
                for (auto& target : filt->destTargets) {
                    if (std::find(filt->sourceTargets.begin(), filt->sourceTargets.end(), target) !=
                        filt->sourceTargets.end()) {
                        rem.setDestination(target);
                        mSendMessage(rem);
                    }
                }
                filt->sourceTargets.clear();
                filt->destTargets.clear();
                setActionFlag(*filt, disconnected_flag);
            }
        } break;
        
        case CMD_REMOVE_ENDPOINT: {
            auto* filtI = getFilterInfo(command.getDest());
            if (filtI != nullptr) {
                filtI->removeTarget(command.getSource());
            }
        } break;
        case CMD_REG_ENDPOINT: {
            auto* filtI = getFilterInfo(mFedID, command.dest_handle);
            if (filtI != nullptr) {
                filtI->sourceTargets.emplace_back(command.source_id, command.source_handle);
                mCoord.addDependency(command.source_id);
            }
            auto* filthandle = mHandles->getFilter(command.dest_handle);
            if (filthandle != nullptr) {
                filthandle->used = true;
            }
        } break;

        case CMD_ADD_ENDPOINT: {
            auto* filtI = getFilterInfo(mFedID, command.dest_handle);
            if (filtI != nullptr) {
                if (checkActionFlag(command, destination_target)) {
                    filtI->destTargets.emplace_back(command.getSource());
                } else {
                    filtI->sourceTargets.emplace_back(command.getSource());
                }
                if (!checkActionFlag(command, error_flag)) {
                    mCoord.addDependency(command.source_id);
                }
            }

            auto* filthandle = mHandles->getFilter(command.dest_handle);
            if (filthandle != nullptr) {
                filthandle->used = true;
            }
        } break;
        case CMD_CORE_CONFIGURE:
            if (command.messageID == UPDATE_FILTER_OPERATOR) {
                auto* filtI = getFilterInfo(mFedID, command.source_handle);
                if (filtI != nullptr) {
                    auto op = mGetAirLock(command.counter).try_unload();
                    if (op) {
                        auto M = std::any_cast<std::shared_ptr<FilterOperator>>(std::move(*op));
                        filtI->filterOp = std::move(M);
                    }
                }
            }
            break;
        default:
            break;
    }
}

FilterInfo* TranslatorFederate::createFilter(GlobalBrokerId dest,
                                         InterfaceHandle handle,
                                         const std::string& key,
                                         const std::string& type_in,
                                         const std::string& type_out,
                                         bool cloning)
{
    auto filt = std::make_unique<FilterInfo>((dest == parent_broker_id || dest == mCoreID) ?
                                                 GlobalBrokerId(mFedID) :
                                                 dest,
                                             handle,
                                             key,
                                             type_in,
                                             type_out,
                                             false);

    auto cid = filt->core_id;
    auto* retTarget = filt.get();
    // auto actualKey = key;
    retTarget->cloning = cloning;
    // if (actualKey.empty()) {
    // actualKey = "sFilter_";
    // actualKey.append(std::to_string(handle.baseValue()));
    // }

    // if (filt->core_id == mFedID) {
    filters.insert({cid, handle}, std::move(filt));
    //} else {
    // actualKey.push_back('_');
    //  actualKey.append(std::to_string(cid.baseValue()));
    //    filters.insert({cid, handle}, std::move(filt));
    //}

    return retTarget;
}


FilterInfo* TranslatorFederate::getFilterInfo(GlobalHandle id)
{
    return filters.find(id);
}

FilterInfo* TranslatorFederate::getFilterInfo(GlobalFederateId fed, InterfaceHandle handle)
{
    if (fed == parent_broker_id || fed == mCoreID) {
        fed = mFedID;
    }
    return filters.find(GlobalHandle{fed, handle});
}

const FilterInfo* TranslatorFederate::getFilterInfo(GlobalFederateId fed, InterfaceHandle handle) const
{
    if (fed == parent_broker_id || fed == mCoreID) {
        fed = mFedID;
    }
    return filters.find(GlobalHandle{fed, handle});
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
        if (filters.size() > 0) {
            base["filters"] = Json::arrayValue;
            for (const auto& filt : filters) {
                Json::Value filter;
                filter["id"] = filt->handle.baseValue();
                filter["name"] = filt->key;
                filter["cloning"] = filt->cloning;
                filter["source_targets"] = generateStringVector(filt->sourceTargets, [](auto& dep) {
                    return std::to_string(dep.fed_id.baseValue()) +
                        "::" + std::to_string(dep.handle.baseValue());
                });
                filter["dest_targets"] = generateStringVector(filt->destTargets, [](auto& dep) {
                    return std::to_string(dep.fed_id.baseValue()) +
                        "::" + std::to_string(dep.handle.baseValue());
                });
                base["filters"].append(std::move(filter));
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
