/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#ifndef HELICS_CPP98_MESSAGE_FEDERATE_HPP_
#define HELICS_CPP98_MESSAGE_FEDERATE_HPP_
#pragma once

#include "Federate.hpp"
#include "Endpoint.hpp"
#include "../shared_api_library/MessageFederate.h"

namespace helics
{
class MessageFederate : public virtual Federate
{
  public:
    explicit MessageFederate (FederateInfo &fi)
    {
        fed = helicsCreateMessageFederate (fi.getInfo ());
    }

    explicit MessageFederate (const std::string &jsonString)
    {
        fed = helicsCreateMessageFederateFromJson (jsonString.c_str());
    }

    // Default constructor, not meant to be used
    MessageFederate () {}

    /** Methods for registering endpoints **/
    Endpoint registerEndpoint (const std::string &name, const std::string &type = "")
    {
        helics_endpoint ep = helicsFederateRegisterEndpoint (fed, name.c_str(), type.c_str());
        local_endpoints.push_back(ep);
        return Endpoint(ep);
    }

    Endpoint registerGlobalEndpoint (const std::string &name, const std::string &type = "")
    {
        helics_endpoint ep = helicsFederateRegisterGlobalEndpoint (fed, name.c_str(), type.c_str());
        local_endpoints.push_back(ep);
        return Endpoint(ep);
    }

    /** Checks if federate has any messages **/
    bool hasMessage () const
    {
        // returns int, 1 = true, 0 = false
        return helicsFederateHasMessage (fed) > 0;
    }

    /** Returns the number of pending receives for all endpoints. **/
    uint64_t receiveCount () const
    {
        return helicsFederateReceiveCount (fed);
    }

    /** Get a packet for any endpoints in the federate **/
    message_t getMessage ()
    {
        return helicsFederateGetMessage (fed);
    }

  private:
    std::vector<helics_endpoint> local_endpoints;

};
} //namespace helics
#endif
