/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/

#include "ZmqRequestSets.h"
#include "helics_includes/optional.h"
#include <algorithm>
namespace helics
{
ZmqRequestSets::ZmqRequestSets () { ctx = zmqContextManager::getContextPointer (); }
void ZmqRequestSets::addRoutes (int routeNumber, const std::string &routeInfo)
{
    auto zsock = std::make_unique<zmq::socket_t> (ctx->getContext (), ZMQ_REQ);
    zsock->connect (routeInfo);
    routes.emplace (routeNumber, std::move (zsock));
}

bool ZmqRequestSets::transmit (int routeNumber, const ActionMessage &command)
{
	//check if we are waiting on the route
    auto fnd = routes_waiting.find (routeNumber);
    if (fnd != routes_waiting.end ())
    {
		if (fnd->second)
		{
			waiting_messages.emplace_back(routeNumber, command);
			return true;
		}
    }
    
    routes[routeNumber]->send (command.to_string ());
    active_routes.emplace_back (zmq::pollitem_t ());
    active_routes.back ().events = ZMQ_POLLIN;
    active_routes.back ().socket = static_cast<void *> (*routes[routeNumber]);

    active_messages.resize (active_messages.size () + 1);
    active_messages.back ().route = routeNumber;
    active_messages.back ().waiting = true;
    active_messages.back ().txmsg = command;
    return true;
}

bool ZmqRequestSets::waiting () const { return (!active_routes.empty ()); }

bool ZmqRequestSets::checkForMessages ()
{
    if (Responses.empty ())
    {
        return checkForMessages (std::chrono::milliseconds (0));
    }
    return true;
}

bool ZmqRequestSets::checkForMessages (std::chrono::milliseconds timeout)
{
    if (active_routes.empty ())
    {
        return false;
    }
    auto rc = zmq::poll (active_routes, timeout);
    if (rc == 0)
    {
        return false;
    }
    zmq::message_t msg;
    for (size_t ii = 0; ii < active_routes.size (); ++ii)
    {
        if (active_routes[ii].revents & ZMQ_POLLIN)
        {
            routes[active_messages[ii].route]->recv (&msg);
            active_routes[ii].events = 0;
            Responses.emplace_back (static_cast<char *> (msg.data ()), msg.size ());
            routes_waiting[active_messages[ii].route] = false;
            active_messages[ii].waiting = false;
        }
    }
    auto res = std::remove_if (active_routes.begin (), active_routes.end (),
                               [](const auto &ar) { return (ar.events == 0); });
    if (res != active_routes.end ())
    {
        active_routes.erase (res, active_routes.end ());
    }
    auto res2 = std::remove_if (active_messages.begin (), active_messages.end (),
                                [](const auto &am) { return (!am.waiting); });
    if (res2 != active_messages.end ())
    {
        active_messages.erase (res2, active_messages.end ());
    }
    if (!waiting_messages.empty ())
    {
        checkDelayedSend ();
    }
    return (!Responses.empty ());
}

void ZmqRequestSets::checkDelayedSend ()
{
    bool still_waiting = false;
    for (auto &delM : waiting_messages)
    {
        if (!routes_waiting[delM.first])
        {
            transmit (delM.first, delM.second);
            delM.first = 0;
        }
        else
        {
            still_waiting = true;
        }
    }
    if (still_waiting)
    {
        auto rem = std::remove_if (waiting_messages.begin (), waiting_messages.end (),
                                   [](const auto &wm) { return (wm.first == 0); });
        if (rem != waiting_messages.end ())
        {
            waiting_messages.erase (rem, waiting_messages.end ());
        }
    }
    else
    {
        waiting_messages.clear ();
    }
}

stx::optional<ActionMessage> ZmqRequestSets::getMessage ()
{
    if (!Responses.empty ())
    {
        auto resp = Responses.front ();
        Responses.pop_front ();
        return resp;
    }
    return {};
}

/*
private:
    std::map<int, zmq::socket_t> routes;
    std::vector<zmq::pollitem_t> active_routes;
    std::vector<waitingResponse> active_messages;
    std::queue<std::pair<int, ActionMessage>> waiting_messages;
    std::queue<ActionMessage> Responses;
    */
} //namespace helics
