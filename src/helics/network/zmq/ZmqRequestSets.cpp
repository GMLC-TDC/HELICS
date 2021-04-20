/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "ZmqRequestSets.h"

#include <algorithm>
#include <iostream>
#include <memory>
#include <utility>

namespace helics {
namespace zeromq {
    ZmqRequestSets::ZmqRequestSets(): ctx(ZmqContextManager::getContextPointer()) {}
    void ZmqRequestSets::addRoutes(int routeNumber, const std::string& routeInfo)
    {
        auto zsock = std::make_unique<zmq::socket_t>(ctx->getContext(), ZMQ_REQ);
        try {
            zsock->connect(routeInfo);
        }
        catch (const zmq::error_t& ze) {
            std::cerr << "error connecting to " << routeInfo << " " << ze.what() << std::endl;
            return;
        }
        zsock->setsockopt(ZMQ_LINGER, 200);
        auto fnd = routes_waiting.find(routeNumber);
        if (fnd != routes_waiting.end()) {
            routes[routeNumber] = std::move(zsock);
        } else {
            routes.emplace(routeNumber, std::move(zsock));
        }

        routes_waiting[routeNumber] = false;
    }

    bool ZmqRequestSets::transmit(int routeNumber, const ActionMessage& command)
    {
        // check if we are waiting on the route
        if (routes_waiting.at(routeNumber)) {
            waiting_messages.emplace_back(routeNumber, command);
            return true;
        }
        routes_waiting[routeNumber] = true;
        routes[routeNumber]->send(command.to_string());
        active_routes.emplace_back(zmq::pollitem_t());
        active_routes.back().events = ZMQ_POLLIN;
        active_routes.back().socket = static_cast<void*>(*routes[routeNumber]);

        active_messages.resize(active_messages.size() + 1);
        active_messages.back().route = routeNumber;
        active_messages.back().waiting = true;
        active_messages.back().txmsg = command;
        return true;
    }

    bool ZmqRequestSets::waiting() const { return (!active_routes.empty()); }

    bool ZmqRequestSets::hasMessages() const { return (!Responses.empty()); }

    int ZmqRequestSets::checkForMessages()
    {
        checkForMessages(std::chrono::milliseconds(0));
        return static_cast<int>(Responses.size());
    }

    int ZmqRequestSets::checkForMessages(std::chrono::milliseconds timeout)
    {
        if (active_routes.empty()) {
            return 0;
        }
        auto rc = zmq::poll(active_routes, timeout);
        if (rc == 0) {
            return 0;
        }
        zmq::message_t msg;
        // scan the active_routes
        for (size_t ii = 0; ii < active_routes.size(); ++ii) {
            if ((active_routes[ii].revents & ZMQ_POLLIN) > 0) {
                routes[active_messages[ii].route]->recv(msg);
                active_routes[ii].events = 0;
                Responses.emplace_back(static_cast<char*>(msg.data()),
                                       msg.size());  // convert to an ActionMessage here
                routes_waiting[active_messages[ii].route] = false;
                active_messages[ii].waiting = false;
            }
        }
        auto res = std::remove_if(active_routes.begin(), active_routes.end(), [](const auto& ar) {
            return (ar.events == 0);
        });
        if (res != active_routes.end()) {
            active_routes.erase(res, active_routes.end());
        }
        auto res2 = std::remove_if(active_messages.begin(),
                                   active_messages.end(),
                                   [](const auto& am) { return (!am.waiting); });
        if (res2 != active_messages.end()) {
            active_messages.erase(res2, active_messages.end());
        }
        if (!waiting_messages.empty()) {
            SendDelayedMessages();
        }
        return static_cast<int>(Responses.size());
    }

    void ZmqRequestSets::SendDelayedMessages()
    {
        bool still_waiting = false;
        for (auto& delM : waiting_messages) {
            if (!routes_waiting[delM.first]) {
                transmit(delM.first, delM.second);
                delM.first = 0;
            } else {
                still_waiting = true;
            }
        }
        if (still_waiting) {
            auto rem = std::remove_if(waiting_messages.begin(),
                                      waiting_messages.end(),
                                      [](const auto& wm) { return (wm.first == 0); });
            if (rem != waiting_messages.end()) {
                waiting_messages.erase(rem, waiting_messages.end());
            }
        } else {
            waiting_messages.clear();
        }
    }

    void ZmqRequestSets::close()
    {
        waiting_messages.clear();
        for (auto& rt : routes) {
            rt.second->close();
        }
        routes.clear();
        routes_waiting.clear();
        active_routes.clear();
    }

    stx::optional<ActionMessage> ZmqRequestSets::getMessage()
    {
        if (!Responses.empty()) {
            auto resp = Responses.front();
            Responses.pop_front();
            return resp;
        }
        return stx::nullopt;
    }

    /*
private:
    std::map<int, zmq::socket_t> routes;
    std::vector<zmq::pollitem_t> active_routes;
    std::vector<WaitingResponse> active_messages;
    std::queue<std::pair<int, ActionMessage>> waiting_messages;
    std::queue<ActionMessage> Responses;
    */
}  // namespace zeromq
}  // namespace helics
