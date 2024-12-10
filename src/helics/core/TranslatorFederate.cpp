/*
Copyright (c) 2017-2024,
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
#include <memory>
#include <string>
#include <utility>

namespace helics {

TranslatorFederate::TranslatorFederate(GlobalFederateId fedID,
                                       std::string name,
                                       GlobalBrokerId coreID,
                                       Core* /*core*/):
    mFedID(fedID), mCoreID(coreID), mName(std::move(name)), /*mCore(core),*/
    mCoord([this](const ActionMessage& msg) { routeMessage(msg); })
{
    mCoord.setSourceId(fedID);
    mCoord.setOptionFlag(helics::defs::Flags::EVENT_TRIGGERED, true);
    mCoord.specifyNonGranting(true);
    mCoord.setProperty(HELICS_PROPERTY_TIME_OUTPUT_DELAY, Time::epsilon());
}

TranslatorFederate::~TranslatorFederate()
{
    mHandles = nullptr;
    current_state = FederateStates::CREATED;

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

void TranslatorFederate::executeTranslator(ActionMessage& command, TranslatorInfo* trans)
{
    if (!trans->tranOp) {
        return;
    }
    switch (command.action()) {
        case CMD_SEND_MESSAGE: {
            const auto& targets = trans->getPubInfo()->subscribers;
            if (targets.empty()) {
                break;
            }
            auto val = trans->tranOp->convertToValue(createMessageFromCommand(command));
            if (!val.empty()) {
                if (targets.size() == 1) {
                    ActionMessage sendM(CMD_PUB);
                    sendM.setDestination(targets.front().id);
                    sendM.setSource(trans->id);
                    sendM.actionTime = trans->tranOp->computeNewValueTime(command.actionTime);
                    sendM.payload = std::move(val);
                    mSendMessageMove(std::move(sendM));
                } else {
                    ActionMessage sendM(CMD_PUB);

                    sendM.setSource(trans->id);
                    sendM.actionTime = trans->tranOp->computeNewValueTime(command.actionTime);
                    sendM.payload = std::move(val);
                    for (const auto& target : targets) {
                        sendM.setDestination(target.id);
                        mSendMessage(sendM);
                    }
                }
            }
        } break;
        case CMD_PUB: {
            auto message = trans->tranOp->convertToMessage(command.payload);
            if (message) {
                auto targets = trans->getEndpointInfo()->getTargets();
                if (targets.empty()) {
                    break;
                }
                const auto& source = trans->getInputInfo()->getSourceName(command.getSource());
                message->source = trans->key;
                message->original_source = source;
                message->time = trans->tranOp->computeNewMessageTime(command.actionTime);
                ActionMessage sendM(std::move(message));
                sendM.setSource(command.getSource());

                if (targets.size() == 1) {
                    sendM.setString(targetStringLoc, targets.front().second);
                    sendM.setDestination(targets.front().first);
                    mDeliverMessage(sendM);
                } else {
                    for (const auto& target : targets) {
                        auto messageCopy(sendM);
                        messageCopy.setString(targetStringLoc, target.second);
                        messageCopy.setDestination(target.first);
                        mDeliverMessage(messageCopy);
                    }
                }
            }
        }

        break;
        default:
            break;
    }
}

void TranslatorFederate::handleMessage(ActionMessage& command)
{
    auto proc_result = processCoordinatorMessage(command, &mCoord, current_state, false, mFedID);

    if (std::get<2>(proc_result) && current_state == FederateStates::EXECUTING) {
        mCoord.disconnect();
        ActionMessage disconnect(CMD_DISCONNECT);
        disconnect.source_id = mFedID;
        disconnect.dest_id = parent_broker_id;
        mQueueMessage(disconnect);
    }
    if (current_state != std::get<0>(proc_result)) {
        current_state = std::get<0>(proc_result);
        switch (current_state) {
            case FederateStates::INITIALIZING:
                mCoord.enteringExecMode(IterationRequest::NO_ITERATIONS);
                {
                    ActionMessage echeck{CMD_EXEC_CHECK};
                    echeck.dest_id = mFedID;
                    echeck.source_id = mFedID;
                    handleMessage(echeck);
                }
                break;
            case FederateStates::EXECUTING:
                mCoord.timeRequest(Time::maxVal(),
                                   IterationRequest::NO_ITERATIONS,
                                   Time::maxVal(),
                                   Time::maxVal());
                break;
            case FederateStates::FINISHED:
                break;
            case FederateStates::ERRORED: {
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
                // TODO(PT) actually close the interface
            }
        } break;

        case CMD_REMOVE_ENDPOINT: {
            auto* tranI = getTranslatorInfo(command.getDest());
            if (tranI != nullptr) {
                tranI->getEndpointInfo()->removeTarget(command.getSource());
            }
        } break;
        case CMD_REMOVE_PUBLICATION: {
            auto* tranI = getTranslatorInfo(command.getDest());
            if (tranI != nullptr) {
                tranI->getInputInfo()->removeSource(command.getSource(), timeZero);
            }

        } break;
        case CMD_REMOVE_SUBSCRIBER: {
            auto* tranI = getTranslatorInfo(command.getDest());
            if (tranI != nullptr) {
                tranI->getPubInfo()->removeSubscriber(command.getSource());
            }

        } break;
        case CMD_ADD_PUBLISHER: {
            auto* tranI = getTranslatorInfo(mFedID, command.dest_handle);
            if (tranI != nullptr) {
                tranI->getInputInfo()->addSource(command.getSource(),
                                                 command.name(),
                                                 command.getString(typeStringLoc),
                                                 command.getString(unitStringLoc));

                if (!checkActionFlag(command, error_flag)) {
                    mCoord.addDependency(command.source_id);
                }
            }
        } break;
        case CMD_ADD_SUBSCRIBER: {
            auto* tranI = getTranslatorInfo(mFedID, command.dest_handle);
            if (tranI != nullptr) {
                tranI->getPubInfo()->addSubscriber(command.getSource(), command.name());

                if (!checkActionFlag(command, error_flag)) {
                    mCoord.addDependent(command.source_id);
                }
            }
        } break;
        case CMD_ADD_ENDPOINT: {
            auto* tranI = getTranslatorInfo(mFedID, command.dest_handle);
            if (tranI != nullptr) {
                if (checkActionFlag(command, destination_target)) {
                    tranI->getEndpointInfo()->addDestination(command.getSource(),
                                                             command.name(),
                                                             command.getString(typeStringLoc));
                    if (!checkActionFlag(command, error_flag)) {
                        mCoord.addDependent(command.source_id);
                    }
                } else {
                    tranI->getEndpointInfo()->addSource(command.getSource(),
                                                        command.name(),
                                                        command.getString(typeStringLoc));
                    if (!checkActionFlag(command, error_flag)) {
                        mCoord.addDependency(command.source_id);
                    }
                }
            }

        } break;
        case CMD_CORE_CONFIGURE:
            if (command.messageID == UPDATE_TRANSLATOR_OPERATOR) {
                auto* tranI = getTranslatorInfo(mFedID, command.source_handle);
                if (tranI != nullptr) {
                    auto locker = mGetAirLock(command.counter).try_unload();
                    if (locker) {
                        auto operation =
                            std::any_cast<std::shared_ptr<TranslatorOperator>>(std::move(*locker));
                        tranI->tranOp = std::move(operation);
                    }
                }
            }
            break;
        case CMD_SEND_MESSAGE:
        case CMD_PUB: {
            auto* tranI = getTranslatorInfo(mFedID, command.dest_handle);
            if (tranI != nullptr) {
                executeTranslator(command, tranI);
            }
        } break;
        default:
            break;
    }
}

TranslatorInfo* TranslatorFederate::createTranslator(GlobalBrokerId dest,
                                                     InterfaceHandle handle,
                                                     std::string_view key,
                                                     std::string_view endpointType,
                                                     std::string_view units)
{
    auto tran = std::make_unique<TranslatorInfo>(
        GlobalHandle{(dest == parent_broker_id || dest == mCoreID) ? GlobalBrokerId{mFedID} : dest,
                     handle},
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

TranslatorInfo* TranslatorFederate::getTranslatorInfo(GlobalHandle gid)
{
    return translators.find(gid);
}

TranslatorInfo* TranslatorFederate::getTranslatorInfo(GlobalFederateId fed, InterfaceHandle handle)
{
    if (fed == parent_broker_id || fed == mCoreID) {
        fed = mFedID;
    }
    return translators.find(GlobalHandle{fed, handle});
}

const TranslatorInfo* TranslatorFederate::getTranslatorInfo(GlobalFederateId fed,
                                                            InterfaceHandle handle) const
{
    if (fed == parent_broker_id || fed == mCoreID) {
        fed = mFedID;
    }
    return translators.find(GlobalHandle{fed, handle});
}

std::string TranslatorFederate::query(std::string_view queryStr) const
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
        nlohmann::json base;
        base["attributes"] = nlohmann::json::object();
        base["attributes"]["name"] = mName;
        base["attributes"]["id"] = mFedID.baseValue();
        base["attributes"]["parent"] = mCoreID.baseValue();
        base["state"] = fedStateString(current_state);
        base["publications"] = 0;
        base["input"] = 0;
        base["endpoints"] = 0;
        base["granted_time"] = static_cast<double>(mCoord.getGrantedTime());
        return fileops::generateJsonString(base);
    }
    if (queryStr == "global_state") {
        nlohmann::json base;
        base["attributes"] = nlohmann::json::object();
        base["attributes"]["name"] = mName;
        base["attributes"]["id"] = mFedID.baseValue();
        base["attributes"]["parent"] = mCoreID.baseValue();
        base["state"] = fedStateString(current_state);
        return fileops::generateJsonString(base);
    }
    if (queryStr == "global_time_debugging") {
        nlohmann::json base;
        base["attributes"] = nlohmann::json::object();
        base["attributes"]["name"] = mName;
        base["attributes"]["id"] = mFedID.baseValue();
        base["attributes"]["parent"] = mCoreID.baseValue();
        base["state"] = fedStateString(current_state);
        mCoord.generateDebuggingTimeInfo(base);
        return fileops::generateJsonString(base);
    }
    if (queryStr == "timeconfig") {
        nlohmann::json base;
        mCoord.generateConfig(base);
        return fileops::generateJsonString(base);
    }
    if (queryStr == "config") {
        nlohmann::json base;
        mCoord.generateConfig(base);
        return fileops::generateJsonString(base);
    }
    if (queryStr == "dependents") {
        return generateStringVector(mCoord.getDependents(),
                                    [](auto& dep) { return std::to_string(dep.baseValue()); });
    }
    if (queryStr == "data_flow_graph") {
        nlohmann::json base;
        base["attributes"] = nlohmann::json::object();
        base["attributes"]["name"] = mName;
        base["attributes"]["id"] = mFedID.baseValue();
        base["attributes"]["parent"] = mCoreID.baseValue();
        if (translators.size() > 0) {
            base["translators"] = nlohmann::json::array();
            for (const auto& trans : translators) {
                nlohmann::json tran;
                tran["id"] = trans->id.handle.baseValue();
                tran["name"] = trans->key;

                tran["source_endpoints"] = trans->getEndpointInfo()->getSourceTargets();
                tran["destination_endpoints"] = trans->getEndpointInfo()->getDestinationTargets();
                tran["source_publications"] = trans->getPubInfo()->getTargets();
                tran["destination_inputs"] = trans->getInputInfo()->getTargets();
                base["translators"].push_back(std::move(tran));
            }
        }
        return fileops::generateJsonString(base);
    }
    if (queryStr == "global_time") {
        nlohmann::json base;
        base["attributes"] = nlohmann::json::object();
        base["attributes"]["name"] = mName;
        base["attributes"]["id"] = mFedID.baseValue();
        base["attributes"]["parent"] = mCoreID.baseValue();
        base["granted_time"] = static_cast<double>(mCoord.getGrantedTime());
        base["send_time"] = static_cast<double>(mCoord.allowedSendTime());
        return fileops::generateJsonString(base);
    }
    if (queryStr == "dependency_graph") {
        nlohmann::json base;
        base["attributes"] = nlohmann::json::object();
        base["attributes"]["name"] = mName;
        base["attributes"]["id"] = mFedID.baseValue();
        base["attributes"]["parent"] = mCoreID.baseValue();
        base["dependents"] = nlohmann::json::array();
        for (auto& dep : mCoord.getDependents()) {
            base["dependents"].push_back(dep.baseValue());
        }
        base["dependencies"] = nlohmann::json::array();
        for (auto& dep : mCoord.getDependencies()) {
            base["dependencies"].push_back(dep.baseValue());
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
