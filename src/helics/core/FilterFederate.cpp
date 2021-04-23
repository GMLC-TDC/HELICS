/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "FilterFederate.hpp"

#include "../common/JsonProcessingFunctions.hpp"
#include "BasicHandleInfo.hpp"
#include "HandleManager.hpp"
#include "TimeCoordinatorProcessing.hpp"
#include "coreTypeOperations.hpp"
#include "flagOperations.hpp"
#include "helicsVersion.hpp"
#include "helics_definitions.hpp"
#include "queryHelpers.hpp"

namespace helics {

FilterFederate::FilterFederate(global_federate_id fedID,
                               std::string name,
                               global_broker_id coreID,
                               Core* /*core*/):
    mFedID(fedID),
    mCoreID(coreID), mName(std::move(name)), /*mCore(core),*/
    mCoord([this](const ActionMessage& msg) { routeMessage(msg); })
{
    mCoord.source_id = fedID;
    mCoord.setOptionFlag(helics::defs::flags::event_triggered, true);
    mCoord.specifyNonGranting(true);
}

FilterFederate::~FilterFederate()
{
    mHandles = {nullptr};
    current_state = {HELICS_CREATED};
    /// map of all local filters
    filterCoord.clear();
    // The interface_handle used is here is usually referencing an endpoint

    mQueueMessage = nullptr;
    mQueueMessageMove = nullptr;
    mSendMessage = nullptr;
    mSendMessageMove = nullptr;

    mDeliverMessage = nullptr;

    mLogger = nullptr;
    mGetAirLock = nullptr;

    /// sets of ongoing filtered messages
    ongoingFilterProcesses.clear();
    /// sets of ongoing destination filter processing
    ongoingDestFilterProcesses.clear();

    filters.clear();
}

void FilterFederate::routeMessage(const ActionMessage& msg)
{
    if (mSendMessage) {
        mQueueMessage(msg);
    }
}

/** process any filter or route the message*/
void FilterFederate::processMessageFilter(ActionMessage& cmd)
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
                                cmd.dest_handle = interface_handle();
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
                        cmd.dest_handle = interface_handle();
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
                    cmd.dest_handle = interface_handle();
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

void FilterFederate::generateProcessMarker(global_federate_id fid, uint32_t pid, Time returnTime)
{
    // nothing further to process
    auto fid_index = fid.baseValue();
    if (ongoingFilterProcesses[fid_index].empty()) {
        ActionMessage block(CMD_TIME_BLOCK);
        block.dest_id = mCoreID;
        block.source_id = fid;
        mSendMessage(block);
    }
    ongoingFilterProcesses[fid_index].insert(pid);
    addTimeReturn(pid, returnTime);
}

void FilterFederate::acceptProcessReturn(global_federate_id fid, uint32_t pid)
{
    // nothing further to process
    auto fid_index = fid.baseValue();
    ongoingFilterProcesses[fid_index].erase(pid);
    if (ongoingFilterProcesses[fid_index].empty()) {
        ActionMessage unblock(CMD_TIME_UNBLOCK);
        unblock.dest_id = mCoreID;
        unblock.source_id = fid;
        unblock.sequenceID = pid;
        mSendMessage(unblock);
    }
    clearTimeReturn(pid);
}

void FilterFederate::generateDestProcessMarker(global_federate_id fid,
                                               uint32_t pid,
                                               Time returnTime)
{
    // nothing further to process
    auto fid_index = fid.baseValue();
    if (ongoingDestFilterProcesses[fid_index].empty()) {
        ActionMessage block(CMD_TIME_BLOCK);
        block.dest_id = fid;
        block.source_id = mFedID;
        block.sequenceID = pid;
        mSendMessage(block);
    }
    ongoingDestFilterProcesses[fid_index].insert(pid);
    addTimeReturn(pid, returnTime);
}

void FilterFederate::acceptDestProcessReturn(global_federate_id fid, uint32_t pid)
{
    // nothing further to process
    auto fid_index = fid.baseValue();
    ongoingDestFilterProcesses[fid_index].erase(pid);
    if (ongoingDestFilterProcesses[fid_index].empty()) {
        ActionMessage unblock(CMD_TIME_UNBLOCK);
        unblock.dest_id = fid;
        unblock.source_id = mFedID;
        unblock.sequenceID = pid;
        mSendMessage(unblock);
    }
    clearTimeReturn(pid);
}

/** process a filter message return*/
void FilterFederate::processFilterReturn(ActionMessage& cmd)
{
    auto* handle = mHandles->getEndpoint(cmd.dest_handle);
    if (handle == nullptr) {
        return;
    }

    auto mid = cmd.sequenceID;
    auto fid = handle->getFederateId();
    auto fid_index = fid.baseValue();

    if (ongoingFilterProcesses[fid_index].find(mid) != ongoingFilterProcesses[fid_index].end()) {
        if (cmd.action() == CMD_NULL_MESSAGE) {
            acceptProcessReturn(fid, mid);
            return;
        }
        auto* filtFunc = getFilterCoordinator(handle->getInterfaceHandle());
        cmd.setAction(CMD_SEND_MESSAGE);
        bool needToSendMessage{true};
        for (auto ii = static_cast<size_t>(cmd.counter) + 1; ii < filtFunc->sourceFilters.size();
             ++ii) {
            auto* filt = filtFunc->sourceFilters[ii];
            if (checkActionFlag(*filt, disconnected_flag)) {
                continue;
            }

            auto press = executeFilter(cmd, filt);
            if (!press.second) {
                if (cmd.action() == CMD_IGNORE) {
                    needToSendMessage = false;
                    break;
                }

                if (ii < filtFunc->sourceFilters.size() - 1) {
                    cmd.counter = static_cast<uint16_t>(ii);
                    cmd.setAction(CMD_SEND_FOR_FILTER_AND_RETURN);
                    cmd.sequenceID = messageCounter++;
                    cmd.setSource(handle->handle);
                    generateProcessMarker(handle->getFederateId(), cmd.sequenceID, cmd.actionTime);
                } else {
                    cmd.setAction(CMD_SEND_FOR_FILTER);
                }
                break;
            }
        }
        acceptProcessReturn(fid, mid);
        if (needToSendMessage) {
            mDeliverMessage(cmd);
        }
    }
}

/** process a destination filter message return*/
void FilterFederate::processDestFilterReturn(ActionMessage& command)
{
    {
        auto* handle = mHandles->getEndpoint(command.dest_handle);
        if (handle == nullptr) {
            return;
        }
        auto mid = command.sequenceID;
        auto fid = handle->getFederateId();

        auto& ongoingDestProcess = ongoingDestFilterProcesses[fid.baseValue()];
        if (ongoingDestProcess.find(mid) != ongoingDestProcess.end()) {
            if (command.action() == CMD_NULL_DEST_MESSAGE) {
                acceptDestProcessReturn(fid, mid);
                return;
            }
            auto* filtFunc = getFilterCoordinator(handle->getInterfaceHandle());

            if (!filtFunc->cloningDestFilters.empty()) {
                runCloningDestinationFilters(filtFunc, handle, command);
            }

            // mCoord->processTimeMessage(command);
            command.setAction(CMD_SEND_MESSAGE);
            mSendMessageMove(std::move(command));
            acceptDestProcessReturn(fid, mid);
        }
    }
}

std::pair<ActionMessage&, bool> FilterFederate::executeFilter(ActionMessage& command,
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

ActionMessage& FilterFederate::processMessage(ActionMessage& command, const BasicHandleInfo* handle)
{
    auto* filtFunc = getFilterCoordinator(handle->getInterfaceHandle());
    if (filtFunc == nullptr) {
        return command;
    }
    if (filtFunc->hasSourceFilters) {
        //   for (int ii = 0; ii < static_cast<int> (filtFunc->sourceFilters.size ()); ++ii)
        size_t ii = 0;
        for (auto& filt : filtFunc->sourceFilters) {
            if (checkActionFlag(*filt, disconnected_flag)) {
                continue;
            }
            auto press = executeFilter(command, filt);
            if (!press.second) {
                if (command.action() == CMD_IGNORE) {
                    return command;
                }
                command.counter = static_cast<uint16_t>(ii);
                if (ii < filtFunc->sourceFilters.size() - 1) {
                    command.setAction(CMD_SEND_FOR_FILTER_AND_RETURN);
                    command.sequenceID = messageCounter++;
                    generateProcessMarker(handle->getFederateId(),
                                          command.sequenceID,
                                          command.actionTime);
                } else {
                    command.setAction(CMD_SEND_FOR_FILTER);
                }
                return command;
            }
            ++ii;
        }
    }
    return command;
}

bool FilterFederate::destinationProcessMessage(ActionMessage& command,
                                               const BasicHandleInfo* handle)
{
    auto* ffunc = getFilterCoordinator(handle->getInterfaceHandle());
    if (ffunc != nullptr) {
        if (ffunc->destFilter != nullptr) {
            if (!checkActionFlag(*(ffunc->destFilter), disconnected_flag)) {
                if (ffunc->destFilter->core_id != mFedID) {  // now we have deal with non-local
                                                             // processing destination filter
                    // first block the federate time advancement until the return is
                    // received
                    auto mid = ++messageCounter;
                    auto fed_id = handle->getFederateId();
                    generateDestProcessMarker(handle->getFederateId(), mid, command.actionTime);

                    // now send a message to get filtered
                    command.setAction(CMD_SEND_FOR_DEST_FILTER_AND_RETURN);
                    command.sequenceID = mid;
                    command.source_id = fed_id;
                    command.source_handle = handle->getInterfaceHandle();
                    command.dest_id = ffunc->destFilter->core_id;
                    command.dest_handle = ffunc->destFilter->handle;

                    mSendMessageMove(std::move(command));
                    return false;
                }
                // the filter is part of this core

                if (ffunc->destFilter->filterOp) {
                    auto tempMessage = createMessageFromCommand(std::move(command));
                    auto odest = tempMessage->dest;
                    auto nmessage = ffunc->destFilter->filterOp->process(std::move(tempMessage));
                    if (odest == nmessage->dest) {
                        command = std::move(nmessage);
                    } else {
                        // handle destination reroute filters
                        command = std::move(nmessage);
                        mDeliverMessage(command);
                        return false;
                    }
                }
            }
        }
        if (!ffunc->cloningDestFilters.empty()) {
            runCloningDestinationFilters(ffunc, handle, command);
        }
    }
    return true;
}

void FilterFederate::runCloningDestinationFilters(const FilterCoordinator* fcoord,
                                                  const BasicHandleInfo* handle,
                                                  const ActionMessage& command) const
{
    // now go to the cloning filters
    for (auto* clFilter : fcoord->cloningDestFilters) {
        if (checkActionFlag(*clFilter, disconnected_flag)) {
            continue;
        }
        if (clFilter->core_id == mFedID) {
            const auto* FiltI = getFilterInfo(mFedID, clFilter->handle);
            if (FiltI != nullptr) {
                if (FiltI->filterOp != nullptr) {
                    // this is a cloning filter so it generates a bunch(?) of new
                    // messages
                    auto new_messages =
                        FiltI->filterOp->processVector(createMessageFromCommand(command));
                    for (auto& msg : new_messages) {
                        if (msg) {
                            if (msg->dest == handle->key) {
                                // in case the clone filter send to itself.
                                ActionMessage cmd(std::move(msg));
                                cmd.dest_id = handle->handle.fed_id;
                                cmd.dest_handle = handle->handle.handle;
                                mSendMessageMove(std::move(cmd));
                            } else {
                                ActionMessage cmd(std::move(msg));
                                mDeliverMessage(cmd);
                            }
                        }
                    }
                }
            }
        } else {
            ActionMessage clone(command);
            clone.setAction(CMD_SEND_FOR_FILTER);
            clone.dest_id = clFilter->core_id;
            clone.dest_handle = clFilter->handle;
            mSendMessage(clone);
        }
    }
}

void FilterFederate::addTimeReturn(int32_t id, Time TimeVal)
{
    timeBlockProcesses.emplace_back(id, TimeVal);
    if (TimeVal < minReturnTime) {
        minReturnTime = TimeVal;
        mCoord.updateMessageTime(minReturnTime, current_state == HELICS_EXECUTING);
    }
}

void FilterFederate::clearTimeReturn(int32_t id)
{
    if (timeBlockProcesses.empty()) {
        return;
    }
    bool recheckTime = false;
    if (timeBlockProcesses.front().first == id) {
        if (timeBlockProcesses.front().second == minReturnTime) {
            recheckTime = true;
        }
        timeBlockProcesses.pop_front();
    } else {
    }
    if (recheckTime) {
        minReturnTime = Time::maxVal();
        for (const auto& tBP : timeBlockProcesses) {
            if (tBP.second < minReturnTime) {
                minReturnTime = tBP.second;
            }
        }
        mCoord.updateMessageTime(minReturnTime, current_state == HELICS_EXECUTING);
    }
}

void FilterFederate::handleMessage(ActionMessage& command)
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
                mCoord.enteringExecMode(iteration_request::no_iterations);
                {
                    ActionMessage echeck{CMD_EXEC_CHECK};
                    echeck.dest_id = mFedID;
                    echeck.source_id = mFedID;
                    handleMessage(echeck);
                }
                break;
            case HELICS_EXECUTING:
                mCoord.timeRequest(Time::maxVal(),
                                   iteration_request::no_iterations,
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
                    errorString = command.payload;
                }
                if (mLogger) {
                    mLogger(helics_log_level_error, mName, errorString);
                }
            } break;
            default:
                break;
        }
    }

    switch (std::get<1>(proc_result)) {
        case message_processing_result::continue_processing:
            break;
        case message_processing_result::reprocess_message:
            if (command.dest_id != mFedID) {
                mSendMessage(command);
                return;
            }
            return handleMessage(command);
        case message_processing_result::delay_message:
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
        case CMD_REMOVE_FILTER: {
            auto* filterC = getFilterCoordinator(command.dest_handle);
            if (filterC == nullptr) {
                return;
            }
            filterC->closeFilter(command.getSource());
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
        case CMD_REG_FILTER:
            processFilterInfo(command);
            break;
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
                        auto M = stx::any_cast<std::shared_ptr<FilterOperator>>(std::move(*op));
                        filtI->filterOp = std::move(M);
                    }
                }
            }
            break;
        default:
            break;
    }
}

FilterInfo* FilterFederate::createFilter(global_broker_id dest,
                                         interface_handle handle,
                                         const std::string& key,
                                         const std::string& type_in,
                                         const std::string& type_out,
                                         bool cloning)
{
    auto filt = std::make_unique<FilterInfo>((dest == parent_broker_id || dest == mCoreID) ?
                                                 global_broker_id(mFedID) :
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

FilterCoordinator* FilterFederate::getFilterCoordinator(interface_handle handle)
{
    auto fnd = filterCoord.find(handle);
    if (fnd == filterCoord.end()) {
        // just make a dummy filterFunction so we have something to return
        auto ff = std::make_unique<FilterCoordinator>();
        auto* ffp = ff.get();
        filterCoord.emplace(handle, std::move(ff));
        return ffp;
    }
    return fnd->second.get();
}

FilterInfo* FilterFederate::getFilterInfo(global_handle id)
{
    return filters.find(id);
}

FilterInfo* FilterFederate::getFilterInfo(global_federate_id fed, interface_handle handle)
{
    if (fed == parent_broker_id || fed == mCoreID) {
        fed = mFedID;
    }
    return filters.find(global_handle{fed, handle});
}

const FilterInfo* FilterFederate::getFilterInfo(global_federate_id fed,
                                                interface_handle handle) const
{
    if (fed == parent_broker_id || fed == mCoreID) {
        fed = mFedID;
    }
    return filters.find(global_handle{fed, handle});
}

void FilterFederate::processFilterInfo(ActionMessage& command)
{
    auto* filterC = getFilterCoordinator(command.dest_handle);
    if (filterC == nullptr) {
        return;
    }
    bool FilterAlreadyPresent = false;
    if (checkActionFlag(command, destination_target)) {
        if (checkActionFlag(command, clone_flag)) {
            for (auto& filt : filterC->cloningDestFilters) {
                if ((filt->core_id == command.source_id) &&
                    (filt->handle == command.source_handle)) {
                    FilterAlreadyPresent = true;
                    break;
                }
            }
        } else {  // there can only be one non-cloning destination filter
            if (filterC->destFilter != nullptr) {
                if ((filterC->destFilter->core_id == command.source_id) &&
                    (filterC->destFilter->handle == command.source_handle)) {
                    FilterAlreadyPresent = true;
                }
            }
        }

        if (!FilterAlreadyPresent) {
            auto* endhandle = mHandles->getEndpoint(command.dest_handle);
            if (endhandle != nullptr) {
                setActionFlag(*endhandle, has_dest_filter_flag);
                if ((!checkActionFlag(command, clone_flag)) && (filterC->hasDestFilters)) {
                    // duplicate non cloning destination filters are not allowed
                    ActionMessage err(CMD_ERROR);
                    err.dest_id = command.source_id;
                    err.setSource(command.getDest());
                    err.messageID = defs::errors::registration_failure;
                    err.payload =
                        "Endpoint " + endhandle->key + " already has a destination filter";
                    mSendMessageMove(std::move(err));
                    return;
                }
            }
            auto* newFilter = getFilterInfo(command.getSource());
            if (newFilter == nullptr) {
                newFilter = createFilter(global_broker_id(command.source_id),
                                         command.source_handle,
                                         command.name,
                                         command.getString(typeStringLoc),
                                         command.getString(typeOutStringLoc),
                                         checkActionFlag(command, clone_flag));
            }

            filterC->hasDestFilters = true;
            if (checkActionFlag(command, clone_flag)) {
                filterC->cloningDestFilters.push_back(newFilter);
            } else {
                if (endhandle != nullptr) {
                    setActionFlag(*endhandle, has_non_cloning_dest_filter_flag);
                }
                filterC->destFilter = newFilter;
            }
        }
    } else {
        for (auto& filt : filterC->allSourceFilters) {
            if ((filt->core_id == command.source_id) && (filt->handle == command.source_handle)) {
                FilterAlreadyPresent = true;
                break;
            }
        }
        if (!FilterAlreadyPresent) {
            auto* newFilter = getFilterInfo(command.getSource());
            if (newFilter == nullptr) {
                newFilter = createFilter(global_broker_id(command.source_id),
                                         command.source_handle,
                                         command.name,
                                         command.getString(typeStringLoc),
                                         command.getString(typeOutStringLoc),
                                         checkActionFlag(command, clone_flag));
            }
            filterC->allSourceFilters.push_back(newFilter);
            filterC->hasSourceFilters = true;
            auto* endhandle = mHandles->getEndpoint(command.dest_handle);
            if (endhandle != nullptr) {
                setActionFlag(*endhandle, has_source_filter_flag);
            }
        }
    }
}

void FilterFederate::organizeFilterOperations()
{
    for (auto& fc : filterCoord) {
        auto* fi = fc.second.get();
        const auto* handle = mHandles->getHandleInfo(fc.first);
        if (handle == nullptr) {
            continue;
        }
        std::string endpointType = handle->type;

        if (!fi->allSourceFilters.empty()) {
            fi->sourceFilters.clear();
            fi->sourceFilters.reserve(fi->allSourceFilters.size());
            // Now we have to do some intelligent ordering with types
            std::vector<bool> used(fi->allSourceFilters.size(), false);
            bool someUnused = true;
            bool usedMore = true;
            bool firstPass = true;
            std::string currentType = endpointType;
            while (someUnused && usedMore) {
                someUnused = false;
                usedMore = false;
                for (size_t ii = 0; ii < fi->allSourceFilters.size(); ++ii) {
                    if (used[ii]) {
                        continue;
                    }
                    if (firstPass) {
                        if (fi->allSourceFilters[ii]->cloning) {
                            fi->sourceFilters.push_back(fi->allSourceFilters[ii]);
                            used[ii] = true;
                            usedMore = true;
                        } else {
                            someUnused = true;
                        }
                    } else {
                        // TODO(PT): this will need some work to finish sorting out but should work
                        // for initial tests
                        if (core::matchingTypes(fi->allSourceFilters[ii]->inputType, currentType)) {
                            used[ii] = true;
                            usedMore = true;
                            fi->sourceFilters.push_back(fi->allSourceFilters[ii]);
                            currentType = fi->allSourceFilters[ii]->outputType;
                        } else {
                            someUnused = true;
                        }
                    }
                }
                if (firstPass) {
                    firstPass = false;
                    usedMore = true;
                }
            }
            for (size_t ii = 0; ii < fi->allSourceFilters.size(); ++ii) {
                if (used[ii]) {
                    continue;
                }
                mLogger(helics_log_level_warning,
                        fi->allSourceFilters[ii]->key,
                        "unable to match types on some filters");
            }
        }
    }
}

void FilterFederate::addFilteredEndpoint(Json::Value& block, global_federate_id fed) const
{
    block["endpoints"] = Json::arrayValue;
    for (const auto& filt : filterCoord) {
        auto* fc = filt.second.get();
        const auto* ep = mHandles->getEndpoint(filt.first);
        if (ep->getFederateId() != fed) {
            continue;
        }
        Json::Value eptBlock;

        eptBlock["name"] = ep->key;
        eptBlock["id"] = ep->handle.handle.baseValue();
        if (fc->hasSourceFilters) {
            std::string srcFilters = "[";
            for (auto& fcc : fc->sourceFilters) {
                if (!fcc->key.empty()) {
                    srcFilters.append(fcc->key);
                } else {
                    srcFilters += std::to_string(fcc->core_id.baseValue()) + ':' +
                        std::to_string(fcc->handle.baseValue());
                }
                if (fcc->cloning) {
                    srcFilters.append("(cloning)");
                }
                srcFilters.push_back(',');
            }
            if (srcFilters.back() == ',') {
                srcFilters.pop_back();
            }
            srcFilters.push_back(']');
            eptBlock["srcFilters"] = srcFilters;
        }
        if (fc->hasDestFilters) {
            if (fc->destFilter != nullptr) {
                if (!fc->destFilter->key.empty()) {
                    eptBlock["destFilter"] = fc->destFilter->key;
                } else {
                    eptBlock["destFilter"] = std::to_string(fc->destFilter->core_id.baseValue()) +
                        ':' + std::to_string(fc->destFilter->handle.baseValue());
                }
            }
            if (!fc->cloningDestFilters.empty()) {
                std::string dcloningFilter = "[";
                for (auto& fcc : fc->cloningDestFilters) {
                    if (!fcc->key.empty()) {
                        dcloningFilter.append(fcc->key);
                    } else {
                        dcloningFilter += std::to_string(fcc->core_id.baseValue()) + ':' +
                            std::to_string(fcc->handle.baseValue());
                    }
                    dcloningFilter.push_back(',');
                }
                if (dcloningFilter.back() == ',') {
                    dcloningFilter.pop_back();
                }
                dcloningFilter.push_back(']');
                eptBlock["cloningdestFilter"] = dcloningFilter;
            }
        }
        block["endpoints"].append(eptBlock);
    }
}

std::string FilterFederate::query(const std::string& queryStr) const
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
        return generateJsonString(base);
    }
    if (queryStr == "global_state") {
        Json::Value base;
        base["name"] = mName;
        base["id"] = mFedID.baseValue();
        base["parent"] = mCoreID.baseValue();
        base["state"] = fedStateString(current_state);
        return generateJsonString(base);
    }
    if (queryStr == "global_time_debugging") {
        Json::Value base;
        base["name"] = mName;
        base["id"] = mFedID.baseValue();
        base["parent"] = mCoreID.baseValue();
        base["state"] = fedStateString(current_state);
        mCoord.generateDebuggingTimeInfo(base);
        return generateJsonString(base);
    }
    if (queryStr == "timeconfig") {
        Json::Value base;
        mCoord.generateConfig(base);
        return generateJsonString(base);
    }
    if (queryStr == "config") {
        Json::Value base;
        mCoord.generateConfig(base);
        return generateJsonString(base);
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
        return generateJsonString(base);
    }
    if (queryStr == "global_time") {
        Json::Value base;
        base["name"] = mName;
        base["id"] = mFedID.baseValue();
        base["parent"] = mCoreID.baseValue();
        base["granted_time"] = static_cast<double>(mCoord.getGrantedTime());
        base["send_time"] = static_cast<double>(mCoord.allowedSendTime());
        return generateJsonString(base);
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
        return generateJsonString(base);
    }

    return "#invalid";
}

bool FilterFederate::hasActiveTimeDependencies() const
{
    return mCoord.hasActiveTimeDependencies();
}
}  // namespace helics
