/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "TimeCoordinatorProcessing.hpp"

#include "TimeCoordinator.hpp"
#include "flagOperations.hpp"

#include <tuple>

namespace helics {
std::tuple<federate_state, message_processing_result, bool>
    processCoordinatorMessage(ActionMessage& cmd,
                              TimeCoordinator* timeCoord,
                              const federate_state state,
                              const bool timeGranted_mode,
                              const global_federate_id localID)
{
    federate_state newState{state};
    bool newMode{timeGranted_mode};
    message_processing_result proc{message_processing_result::continue_processing};
    bool returnable{false};
    switch (cmd.action()) {
        case CMD_IGNORE:
        default:
            break;
        case CMD_TIME_BLOCK:
        case CMD_TIME_BARRIER:
        case CMD_TIME_BARRIER_CLEAR:
        case CMD_TIME_UNBLOCK: {
            auto processed = timeCoord->processTimeMessage(cmd);
            if (processed == message_process_result::processed) {
                if (!timeGranted_mode) {
                    if (state == HELICS_INITIALIZING) {
                        cmd.setAction(CMD_EXEC_CHECK);
                        proc = message_processing_result::reprocess_message;
                    }
                    if (state == HELICS_EXECUTING) {
                        cmd.setAction(CMD_TIME_CHECK);
                        proc = message_processing_result::reprocess_message;
                    }
                }
            }
            break;
        }
        case CMD_INIT_GRANT:
            if (state == HELICS_CREATED) {
                newState = HELICS_INITIALIZING;
                newMode = true;
                proc = message_processing_result::next_step;
            }
            break;
        case CMD_EXEC_REQUEST:
            if ((cmd.source_id == localID) &&
                checkActionFlag(cmd, indicator_flag)) {  // this sets up a time request
                iteration_request iterate = iteration_request::no_iterations;
                if (checkActionFlag(cmd, iteration_requested_flag)) {
                    iterate = (checkActionFlag(cmd, required_flag)) ?
                        iteration_request::force_iteration :
                        iteration_request::iterate_if_needed;
                }
                timeCoord->enteringExecMode(iterate);
                newMode = false;
                break;
            }
            FALLTHROUGH
            /* FALLTHROUGH */
        case CMD_EXEC_GRANT: {
            bool processed = false;
            switch (timeCoord->processTimeMessage(cmd)) {
                case message_process_result::delay_processing:
                    proc = message_processing_result::delay_message;
                    processed = true;
                    break;
                case message_process_result::no_effect:
                    proc = message_processing_result::continue_processing;
                    processed = true;
                    break;
                default:
                    break;
            }
            if (processed) {
                break;
            }
        }
            FALLTHROUGH
            /* FALLTHROUGH */
        case CMD_EXEC_CHECK:  // just check the time for entry
        {
            if (state != HELICS_INITIALIZING) {
                break;
            }
            if (!timeGranted_mode) {
                auto grant = timeCoord->checkExecEntry();
                switch (grant) {
                    case message_processing_result::iterating:
                        newMode = true;
                        returnable = true;
                        proc = grant;
                        break;
                    case message_processing_result::next_step:
                        newState = HELICS_EXECUTING;
                        newMode = true;
                        returnable = true;
                        proc = grant;
                        break;
                    case message_processing_result::continue_processing:
                        break;
                    default:
                        newMode = true;
                        returnable = true;
                        proc = grant;
                        break;
                }
            }
        } break;
        case CMD_TERMINATE_IMMEDIATELY:
            newState = HELICS_FINISHED;
            proc = message_processing_result::halted;
            break;
        case CMD_STOP:
            newState = HELICS_FINISHED;
            timeCoord->disconnect();
            proc = message_processing_result::halted;
            break;
        case CMD_DISCONNECT_FED_ACK:
            if ((cmd.dest_id == localID) && (cmd.source_id == parent_broker_id)) {
                if ((state != HELICS_FINISHED) && (state != HELICS_TERMINATING)) {
                    timeCoord->disconnect();
                }
                newState = HELICS_FINISHED;
                proc = message_processing_result::halted;
            }
            break;
        case CMD_DISCONNECT_FED:
        case CMD_DISCONNECT:
            if (cmd.source_id == localID) {
                if ((state != HELICS_FINISHED) && (state != HELICS_TERMINATING)) {
                    timeCoord->disconnect();
                    cmd.dest_id = parent_broker_id;
                    newState = HELICS_TERMINATING;
                    proc = message_processing_result::reprocess_message;
                }
            } else {
                returnable = false;
                switch (timeCoord->processTimeMessage(cmd)) {
                    case message_process_result::delay_processing:
                        returnable = true;
                        proc = message_processing_result::delay_message;
                        break;
                    case message_process_result::no_effect:
                        returnable = true;
                        proc = message_processing_result::continue_processing;
                    default:
                        break;
                }
                if (state != HELICS_EXECUTING) {
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
                case message_process_result::delay_processing:
                    returnable = true;
                    proc = message_processing_result::delay_message;
                    break;
                case message_process_result::no_effect:
                    proc = message_processing_result::continue_processing;
                    returnable = true;
                    break;
                default:
                    break;
            }
            if (returnable) {
                break;
            }
            if (state != HELICS_EXECUTING) {
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
                case message_process_result::delay_processing:
                    proc = message_processing_result::delay_message;
                    returnable = true;
                    break;
                case message_process_result::no_effect:
                    proc = message_processing_result::continue_processing;
                    returnable = true;
                    break;
                default:
                    break;
            }
            if (returnable) {
                break;
            }
            FALLTHROUGH
            /* FALLTHROUGH */
        case CMD_TIME_CHECK:
            if (state != HELICS_EXECUTING) {
                break;
            }
            if (!timeGranted_mode) {
                proc = timeCoord->checkTimeGrant();
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
            proc = message_processing_result::next_step;
            break;
        }

        case CMD_ERROR:
        case CMD_LOCAL_ERROR:
        case CMD_GLOBAL_ERROR:
            if (cmd.action() == CMD_GLOBAL_ERROR || cmd.source_id == localID ||
                cmd.source_id == parent_broker_id || cmd.source_id == root_broker_id ||
                cmd.dest_id != localID) {
                if ((state != HELICS_FINISHED) && (state != HELICS_TERMINATING)) {
                    if (cmd.action() != CMD_GLOBAL_ERROR) {
                        timeCoord->localError();
                    }
                    newState = HELICS_ERROR;
                    proc = message_processing_result::error;
                }
            } else {
                switch (timeCoord->processTimeMessage(cmd)) {
                    case message_process_result::delay_processing:
                        returnable = true;
                        proc = message_processing_result::delay_message;
                        break;
                    case message_process_result::no_effect:
                        returnable = true;
                        proc = message_processing_result::continue_processing;
                    default:
                        break;
                }
                if (state != HELICS_EXECUTING) {
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
