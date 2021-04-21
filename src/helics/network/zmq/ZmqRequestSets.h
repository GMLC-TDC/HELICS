/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "../../core/ActionMessage.hpp"
#include "ZmqContextManager.h"
#include "cppzmq/zmq.hpp"
#include "gmlc/containers/extra/optional.hpp"

#include <deque>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace helics {
namespace zeromq {
    /**helper class to manage REQ sockets that are awaiting a response*/
    class WaitingResponse {
      public:
        int route = 0;  //!< the route identifier for the socket
        std::uint16_t loops = 0;  //!< the number of loops
        bool waiting = false;  //!< whether the response is waiting
        ActionMessage txmsg;  //!< the most recently sent message
    };

    /** class for dealing with the priority message paths from a ZMQ comm object
@details it manages a set of routes to different priority queues and handles the responses
THIS CLASS IS NOT THREAD SAFE- ZMQ sockets cannot be transferred between threads without special
care so it would be VERY problematic to use this where multiple threads will interact with it,  thus
no reason to make it thread safe
*/
    class ZmqRequestSets {
      public:
        /** constructor*/
        ZmqRequestSets();
        /** add a route to the request set*/
        void addRoutes(int routeNumber, const std::string& routeInfo);
        /** transmit a command to a specific route number*/
        bool transmit(int routeNumber, const ActionMessage& command);
        /** check if the request set is waiting on any on responses*/
        bool waiting() const;
        /** check for messages with a 0 second timeout
    @return the number of message waiting to be received*/
        int checkForMessages();
        /** check for messages with an explicit timeout
    @return the number of message waiting to be received*/
        int checkForMessages(std::chrono::milliseconds timeout);
        /** check if there are any waiting message without scanning the sockets*/
        bool hasMessages() const;
        /** get any messages that have been received*/
        stx::optional<ActionMessage> getMessage();
        /** close all the sockets*/
        void close();

      private:
        /** check to send any messages that have been delayed waiting for a previous send*/
        void SendDelayedMessages();

      private:
        std::map<int, std::unique_ptr<zmq::socket_t>> routes;  //!< map of all the routes
        std::map<int, bool> routes_waiting;  //!< routes that are waiting to be sent
        std::vector<zmq::pollitem_t> active_routes;  //!< active routes for ZMQ poller
        std::vector<WaitingResponse> active_messages;  //!< more information about waiting messages
        std::vector<std::pair<int, ActionMessage>>
            waiting_messages;  //!< messages that are queued up to send
        std::deque<ActionMessage> Responses;  //!< message that have been received and are waiting
                                              //!< to be sent to the holder
        std::shared_ptr<ZmqContextManager> ctx;  //!< the ZMQ context manager
    };
}  // namespace zeromq
}  // namespace helics
