/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "TimeCoordinatorProcessing.hpp"

#include "TimeCoordinator.hpp"
#include "flagOperations.hpp"

#include <thread>
#include <tuple>

namespace helics {
std::tuple<FederateStates, MessageProcessingResult, bool>
    processCoordinatorMessage(ActionMessage& cmd,
                              TimeCoordinator* timeCoord,
                              const FederateStates state,
                              const bool timeGranted_mode,
                              const GlobalFederateId localID)
{
    FederateStates newState{state};
    bool newMode{timeGranted_mode};
    MessageProcessingResult proc{MessageProcessingResult::CONTINUE_PROCESSING};
    bool returnable{false};
    switch (cmd.action()) {
        case CMD_IGNORE:
        default:
            break;
        case CMD_USER_RETURN: {
            auto tidHash = std::hash<std::thread::id>{}(std::this_thread::get_id());
            auto tid = static_cast<std::int32_t>(tidHash);
            if (tid == cmd.messageID) {
                proc = MessageProcessingResult::USER_RETURN;
            }

        } break;
        case CMD_TIME_BLOCK:
        case CMD_TIME_BARRIER:
        case CMD_TIME_BARRIER_CLEAR:
        case CMD_TIME_UNBLOCK: {
            auto processed = timeCoord->processTimeMessage(cmd);
            if (processed != TimeProcessingResult::NOT_PROCESSED) {
                if (!timeGranted_mode) {
                    if (state == FederateStates::INITIALIZING) {
                        cmd.setAction(CMD_EXEC_CHECK);
                        proc = MessageProcessingResult::REPROCESS_MESSAGE;
                    }
                    if (state == FederateStates::EXECUTING) {
                        cmd.setAction(CMD_TIME_CHECK);
                        proc = MessageProcessingResult::REPROCESS_MESSAGE;
                    }
                }
            }
            break;
        }
        case CMD_INIT_GRANT:
            if (state == FederateStates::CREATED) {
                if (checkActionFlag(cmd, observer_flag) ||
                    checkActionFlag(cmd, dynamic_join_flag)) {
                    timeCoord->setDynamicJoining();
                }
                if (checkActionFlag(cmd, iteration_requested_flag)) {
                    newState = FederateStates::CREATED;
                    newMode = false;
                    proc = MessageProcessingResult::ITERATING;
                } else {
                    newState = FederateStates::INITIALIZING;
                    newMode = true;
                    proc = MessageProcessingResult::NEXT_STEP;
                }
            }
            break;
        case CMD_EXEC_REQUEST:
            if ((cmd.source_id == localID) &&
                checkActionFlag(cmd, indicator_flag)) {  // this sets up a time request
                IterationRequest iterate = IterationRequest::NO_ITERATIONS;
                if (checkActionFlag(cmd, iteration_requested_flag)) {
                    iterate = (checkActionFlag(cmd, required_flag)) ?
                        IterationRequest::FORCE_ITERATION :
                        IterationRequest::ITERATE_IF_NEEDED;
                }
                timeCoord->enteringExecMode(iterate);
                newMode = false;
                break;
            }
            [[fallthrough]];
        case CMD_EXEC_GRANT: {
            bool processed = false;
            switch (timeCoord->processTimeMessage(cmd)) {
                case TimeProcessingResult::DELAY_PROCESSING:
                    proc = MessageProcessingResult::DELAY_MESSAGE;
                    processed = true;
                    break;
                case TimeProcessingResult::NOT_PROCESSED:
                    proc = MessageProcessingResult::CONTINUE_PROCESSING;
                    processed = true;
                    break;
                default:
                    break;
            }
            if (processed) {
                break;
            }
        }
            [[fallthrough]];
        case CMD_EXEC_CHECK:  // just check the time for entry
        {
            if (state != FederateStates::INITIALIZING) {
                break;
            }
            if (!timeGranted_mode) {
                auto grant = timeCoord->checkExecEntry(
                    cmd.action() == CMD_EXEC_REQUEST ? cmd.source_id : GlobalFederateId{});
                switch (grant) {
                    case MessageProcessingResult::ITERATING:
                        newMode = true;
                        // returnable = true;
                        proc = grant;
                        break;
                    case MessageProcessingResult::NEXT_STEP:
                        newState = FederateStates::EXECUTING;
                        newMode = true;
                        // returnable = true;
                        proc = grant;
                        break;
                    case MessageProcessingResult::CONTINUE_PROCESSING:
                        if (cmd.action() == CMD_EXEC_GRANT &&
                            !checkActionFlag(cmd, iteration_requested_flag)) {
                            timeCoord->sendUpdatedExecRequest();
                        }
                        break;
                    default:
                        newMode = true;
                        // returnable = true;
                        proc = grant;
                        break;
                }
            }
        } break;
        case CMD_TERMINATE_IMMEDIATELY:
            newState = FederateStates::FINISHED;
            proc = MessageProcessingResult::HALTED;
            break;
        case CMD_STOP:
            newState = FederateStates::FINISHED;
            timeCoord->disconnect();
            proc = MessageProcessingResult::HALTED;
            break;
        case CMD_DISCONNECT_FED_ACK:
            if ((cmd.dest_id == localID) && (cmd.source_id == parent_broker_id)) {
                if ((state != FederateStates::FINISHED) && (state != FederateStates::TERMINATING)) {
                    timeCoord->disconnect();
                }
                newState = FederateStates::FINISHED;
                proc = MessageProcessingResult::HALTED;
            }
            break;
        case CMD_REQUEST_CURRENT_TIME:
        case CMD_TIMING_INFO:
            timeCoord->processTimeMessage(cmd);
            break;
        case CMD_DISCONNECT_FED:
        case CMD_DISCONNECT:
            if (cmd.source_id == localID) {
                if ((state != FederateStates::FINISHED) && (state != FederateStates::TERMINATING)) {
                    timeCoord->disconnect();
                    cmd.dest_id = parent_broker_id;
                    newState = FederateStates::TERMINATING;
                    proc = MessageProcessingResult::REPROCESS_MESSAGE;
                }
            } else {
                // returnable = false;
                switch (timeCoord->processTimeMessage(cmd)) {
                    case TimeProcessingResult::DELAY_PROCESSING:
                        // returnable = true;
                        proc = MessageProcessingResult::DELAY_MESSAGE;
                        break;
                    case TimeProcessingResult::NOT_PROCESSED:
                        // returnable = true;
                        proc = MessageProcessingResult::CONTINUE_PROCESSING;
                        break;
                    default:
                        break;
                }
                if (state != FederateStates::EXECUTING) {
                    break;
                }
                if (!timeGranted_mode) {
                    proc = timeCoord->checkTimeGrant();
                    if (returnableResult(proc)) {
                        newMode = true;
                    }
                }
            }
            break;
        case CMD_BROADCAST_DISCONNECT:
        case CMD_DISCONNECT_BROKER:
        case CMD_DISCONNECT_CORE:
            returnable = false;
            switch (timeCoord->processTimeMessage(cmd)) {
                case TimeProcessingResult::DELAY_PROCESSING:
                    returnable = true;
                    proc = MessageProcessingResult::DELAY_MESSAGE;
                    break;
                case TimeProcessingResult::NOT_PROCESSED:
                    proc = MessageProcessingResult::CONTINUE_PROCESSING;
                    returnable = true;
                    break;
                default:
                    break;
            }
            if (returnable) {
                break;
            }
            if (state != FederateStates::EXECUTING) {
                break;
            }
            if (!timeGranted_mode) {
                proc = timeCoord->checkTimeGrant();
                if (returnableResult(proc)) {
                    newMode = true;
                }
            }
            break;

        case CMD_TIME_REQUEST:
        case CMD_TIME_GRANT:
            returnable = false;
            switch (timeCoord->processTimeMessage(cmd)) {
                case TimeProcessingResult::DELAY_PROCESSING:
                    proc = MessageProcessingResult::DELAY_MESSAGE;
                    returnable = true;
                    break;
                case TimeProcessingResult::NOT_PROCESSED:
                    proc = MessageProcessingResult::CONTINUE_PROCESSING;
                    returnable = true;
                    break;
                default:
                    break;
            }
            if (returnable) {
                break;
            }
            [[fallthrough]];
        case CMD_TIME_CHECK:
            if (state != FederateStates::EXECUTING) {
                if (state == FederateStates::INITIALIZING) {
                    cmd.setAction(CMD_EXEC_CHECK);
                    proc = MessageProcessingResult::REPROCESS_MESSAGE;
                }

                break;
            }
            if (!timeGranted_mode) {
                proc = timeCoord->checkTimeGrant(
                    cmd.action() == CMD_TIME_REQUEST ? cmd.source_id : GlobalFederateId{});
                if (returnableResult(proc)) {
                    newMode = true;
                }
            }
            break;
        case CMD_FORCE_TIME_GRANT: {
            if (cmd.actionTime < timeCoord->getGrantedTime()) {
                break;
            }
            timeCoord->processTimeMessage(cmd);
            newMode = true;
            proc = MessageProcessingResult::NEXT_STEP;
            break;
        }

        case CMD_ERROR:
        case CMD_LOCAL_ERROR:
        case CMD_GLOBAL_ERROR:
            if (cmd.action() == CMD_GLOBAL_ERROR || cmd.source_id == localID ||
                cmd.source_id == parent_broker_id || cmd.source_id == gRootBrokerID ||
                cmd.dest_id != localID) {
                if ((state != FederateStates::FINISHED) && (state != FederateStates::TERMINATING)) {
                    if (cmd.action() != CMD_GLOBAL_ERROR) {
                        timeCoord->localError();
                    }
                    newState = FederateStates::ERRORED;
                    proc = MessageProcessingResult::ERROR_RESULT;
                }
            } else {
                switch (timeCoord->processTimeMessage(cmd)) {
                    case TimeProcessingResult::DELAY_PROCESSING:
                        returnable = true;
                        proc = MessageProcessingResult::DELAY_MESSAGE;
                        break;
                    case TimeProcessingResult::NOT_PROCESSED:
                        returnable = true;
                        proc = MessageProcessingResult::CONTINUE_PROCESSING;
                        break;
                    default:
                        break;
                }
                if (state != FederateStates::EXECUTING) {
                    break;
                }
                if (returnable) {
                    break;
                }
                if (!timeGranted_mode) {
                    proc = timeCoord->checkTimeGrant();
                    if (returnableResult(proc)) {
                        newMode = true;
                    }
                }
            }
            break;
        case CMD_ADD_DEPENDENCY:
        case CMD_REMOVE_DEPENDENCY:
        case CMD_ADD_DEPENDENT:
        case CMD_REMOVE_DEPENDENT:
        case CMD_ADD_INTERDEPENDENCY:
        case CMD_REMOVE_INTERDEPENDENCY:
            if (cmd.dest_id == localID) {
                timeCoord->processDependencyUpdateMessage(cmd);
            }

            break;
    }
    return {newState, proc, newMode};
}

}  // namespace helics
