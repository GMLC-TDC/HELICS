/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "FilterFederate.hpp"

#include "BasicHandleInfo.hpp"
#include "HandleManager.hpp"
#include "coreTypeOperations.hpp"
#include "flagOperations.hpp"
#include "helics_definitions.hpp"
#include "../common/JsonProcessingFunctions.hpp"

namespace helics {

/** process any filter or route the message*/
void FilterFederate::processMessageFilter(ActionMessage& cmd)
{
    if (cmd.dest_id == parent_broker_id) {
        mSendMessage(cmd);
    } else if (cmd.dest_id == mFedID) {
        // deal with local source filters

        auto* FiltI = filters.find(cmd.getDest());
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
                    auto mid = cmd.messageID;
                    auto tempMessage = createMessageFromCommand(std::move(cmd));
                    tempMessage = FiltI->filterOp->process(std::move(tempMessage));
                    if (tempMessage) {
                        cmd = ActionMessage(std::move(tempMessage));
                    } else {
                        cmd = CMD_IGNORE;
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
                        if (cmd.action() == CMD_IGNORE) {
                            cmd.setAction(destFilter ? CMD_NULL_DEST_MESSAGE : CMD_NULL_MESSAGE);
                            cmd.messageID = mid;
                            mDeliverMessage(cmd);
                            return;
                        }
                        cmd.setAction(destFilter ? CMD_DEST_FILTER_RESULT : CMD_FILTER_RESULT);

                        cmd.source_handle = FiltI->handle;
                        cmd.source_id = mFedID;
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
    } else {
        mSendMessage(cmd);
    }
}
/** process a filter message return*/
void FilterFederate::processFilterReturn(ActionMessage& cmd)
{
    auto* handle = mHandles->getEndpoint(cmd.dest_handle);
    if (handle == nullptr) {
        return;
    }

    auto messID = cmd.messageID;
    auto fid = handle->getFederateId();
    auto fid_index = fid.baseValue();
    if (ongoingFilterProcesses[fid_index].find(messID) != ongoingFilterProcesses[fid_index].end()) {
        if (cmd.action() == CMD_NULL_MESSAGE) {
            ongoingFilterProcesses[fid_index].erase(messID);
            if (ongoingFilterProcesses[fid_index].empty()) {
                ActionMessage unblock(CMD_TIME_UNBLOCK);
                unblock.dest_id = mCoreID;
                unblock.source_id = fid;
                mSendMessage(unblock);
            }
        }
        auto* filtFunc = getFilterCoordinator(handle->getInterfaceHandle());
        if (filtFunc->hasSourceFilters) {
            for (auto ii = static_cast<size_t>(cmd.counter) + 1;
                 ii < filtFunc->sourceFilters.size();
                 ++ii) {
                // cloning filters come first so we don't need to check for them in this code branch
                auto* filt = filtFunc->sourceFilters[ii];
                if (checkActionFlag(*filt, disconnected_flag)) {
                    continue;
                }
                if (filt->core_id == mFedID) {
                    // deal with local source filters
                    auto tempMessage = createMessageFromCommand(std::move(cmd));
                    tempMessage = filt->filterOp->process(std::move(tempMessage));
                    if (tempMessage) {
                        cmd = ActionMessage(std::move(tempMessage));
                    } else {
                        ongoingFilterProcesses[fid_index].erase(messID);
                        if (ongoingFilterProcesses[fid_index].empty()) {
                            ActionMessage unblock(CMD_TIME_UNBLOCK);
                            unblock.dest_id = mCoreID;
                            unblock.source_id = fid;
                            mSendMessage(unblock);
                        }
                        return;
                    }
                } else {
                    cmd.dest_id = filt->core_id;
                    cmd.dest_handle = filt->handle;
                    cmd.counter = static_cast<uint16_t>(ii);
                    if (ii < filtFunc->sourceFilters.size() - 1) {
                        cmd.setAction(CMD_SEND_FOR_FILTER_AND_RETURN);
                    } else {
                        cmd.setAction(CMD_SEND_FOR_FILTER);
                        ongoingFilterProcesses[fid_index].erase(messID);
                    }
                    mSendMessage(cmd);
                    if (ongoingFilterProcesses[fid_index].empty()) {
                        ActionMessage unblock(CMD_TIME_UNBLOCK);
                        unblock.dest_id = mCoreID;
                        unblock.source_id = fid;
                        mSendMessage(unblock);
                    }
                    return;
                }
            }
        }
        ongoingFilterProcesses[fid_index].erase(messID);
        mDeliverMessage(cmd);
        if (ongoingFilterProcesses[fid_index].empty()) {
            ActionMessage unblock(CMD_TIME_UNBLOCK);
            unblock.dest_id = mCoreID;
            unblock.source_id = fid;
            mSendMessage(unblock);
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
        auto messID = command.messageID;
        auto& ongoingDestProcess = ongoingDestFilterProcesses[handle->getFederateId().baseValue()];
        if (ongoingDestProcess.find(messID) != ongoingDestProcess.end()) {
            ongoingDestProcess.erase(messID);
            if (command.action() == CMD_NULL_DEST_MESSAGE) {
                ActionMessage removeTimeBlock(CMD_TIME_UNBLOCK, mCoreID, command.dest_id);
                removeTimeBlock.messageID = messID;
                mSendMessage(removeTimeBlock);
                return;
            }
            auto* filtFunc = getFilterCoordinator(handle->getInterfaceHandle());

            // now go to the cloning filters
            for (auto* clFilter : filtFunc->cloningDestFilters) {
                if (checkActionFlag(*clFilter, disconnected_flag)) {
                    continue;
                }
                if (clFilter->core_id == mFedID) {
                    auto* FiltI = filters.find(global_handle(mFedID, clFilter->handle));
                    if (FiltI != nullptr) {
                        if (FiltI->filterOp != nullptr) {
                            if (FiltI->cloning) {
                                (void)(FiltI->filterOp->process(createMessageFromCommand(command)));
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

            // mCoord->processTimeMessage(command);
            command.setAction(CMD_SEND_MESSAGE);
            mSendMessageMove(std::move(command));
            // now unblock the time
            ActionMessage removeTimeBlock(CMD_TIME_UNBLOCK, mCoreID, handle->getFederateId());
            removeTimeBlock.messageID = messID;
            mSendMessage(removeTimeBlock);
        }
    }
}

ActionMessage& FilterFederate::processMessage(ActionMessage& command, const BasicHandleInfo* handle)
{
    auto* filtFunc = getFilterCoordinator(handle->getInterfaceHandle());
    if (filtFunc->hasSourceFilters) {
        //   for (int ii = 0; ii < static_cast<int> (filtFunc->sourceFilters.size ()); ++ii)
        size_t ii = 0;
        for (auto& filt : filtFunc->sourceFilters) {
            if (checkActionFlag(*filt, disconnected_flag)) {
                continue;
            }
            if (filt->core_id == mFedID) {
                if (filt->cloning) {
                    // cloning filter returns a vector
                    auto new_messages =
                        filt->filterOp->processVector(createMessageFromCommand(command));
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
                        return command;
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
                command.counter = static_cast<uint16_t>(ii);
                if (ii < filtFunc->sourceFilters.size() - 1) {
                    command.setAction(CMD_SEND_FOR_FILTER_AND_RETURN);
                    if (ongoingFilterProcesses[handle->getFederateId().baseValue()].empty()) {
                        ActionMessage block(CMD_TIME_BLOCK);
                        block.dest_id = mCoreID;
                        block.source_id = handle->getFederateId();
                        mSendMessage(block);
                    }
                    ongoingFilterProcesses[handle->getFederateId().baseValue()].insert(
                        command.messageID);
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

void FilterFederate::destinationProcessMessage(ActionMessage& command,
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
                    auto fed_id = handle->getFederateId();
                    ActionMessage tblock(CMD_TIME_BLOCK, mCoreID, fed_id);
                    auto mid = ++messageCounter;
                    tblock.messageID = mid;
                    mSendMessage(tblock);

                    // now send a message to get filtered
                    command.setAction(CMD_SEND_FOR_DEST_FILTER_AND_RETURN);
                    command.messageID = mid;
                    command.source_id = fed_id;
                    command.source_handle = handle->getInterfaceHandle();
                    command.dest_id = ffunc->destFilter->core_id;
                    command.dest_handle = ffunc->destFilter->handle;
                    if (ongoingFilterProcesses[fed_id.baseValue()].empty()) {
                        ActionMessage block(CMD_TIME_BLOCK);
                        block.dest_id = mCoreID;
                        block.source_id = handle->getFederateId();
                        mSendMessage(block);
                    }
                    ongoingDestFilterProcesses[fed_id.baseValue()].emplace(mid);
                    mSendMessageMove(std::move(command));
                    return;
                }
                // the filter is part of this core
                auto tempMessage = createMessageFromCommand(std::move(command));
                if (ffunc->destFilter->filterOp) {
                    auto nmessage = ffunc->destFilter->filterOp->process(std::move(tempMessage));
                    command = std::move(nmessage);
                } else {
                    command = std::move(tempMessage);
                }
            }
        }
        // now go to the cloning filters
        for (auto* clFilter : ffunc->cloningDestFilters) {
            if (checkActionFlag(*clFilter, disconnected_flag)) {
                continue;
            }
            if (clFilter->core_id == mFedID) {
                auto* FiltI = filters.find(global_handle(mFedID, clFilter->handle));
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
}

void FilterFederate::handleMessage(ActionMessage& command)
{
    switch (command.action()) {
        case CMD_REMOVE_FILTER: {
            auto* filterC = getFilterCoordinator(command.dest_handle);
            if (filterC == nullptr) {
                return;
            }
            filterC->closeFilter(command.getSource());
        } break;
        case CMD_REMOVE_ENDPOINT: {
            auto* filtI = filters.find(command.getDest());
            if (filtI != nullptr) {
                filtI->removeTarget(command.getSource());
            }
        } break;
        case CMD_REG_ENDPOINT: {
            auto* filtI = filters.find(global_handle(mFedID, command.dest_handle));
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
            auto* filtI = filters.find(global_handle(mFedID, command.dest_handle));
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
    auto filt = std::make_unique<FilterInfo>(
        (dest == parent_broker_id) ? mFedID : dest, handle, key, type_in, type_out, false);

    auto* retTarget = filt.get();
    auto actualKey = key;
    retTarget->cloning = cloning;
    if (actualKey.empty()) {
        actualKey = "sFilter_";
        actualKey.append(std::to_string(handle.baseValue()));
    }
    if (filt->core_id == mFedID) {
        filters.insert(actualKey, global_handle(dest, filt->handle), std::move(filt));
    } else {
        actualKey.push_back('_');
        actualKey.append(std::to_string(filt->core_id.baseValue()));
        filters.insert(actualKey, {filt->core_id, filt->handle}, std::move(filt));
    }

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
            auto* newFilter = filters.find(command.getSource());
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
            auto* newFilter = filters.find(command.getSource());
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

void FilterFederate::addFilteredEndpoint(Json::Value& block, global_federate_id fed) const {
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
}  // namespace helics
