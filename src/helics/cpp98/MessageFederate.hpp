/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#ifndef HELICS_CPP98_MESSAGE_FEDERATE_HPP_
#define HELICS_CPP98_MESSAGE_FEDERATE_HPP_
#pragma once

#include "../shared_api_library/MessageFederate.h"
#include "Endpoint.hpp"
#include "Federate.hpp"

namespace helicscpp
{
class MessageFederate : public virtual Federate
{
  public:
    explicit MessageFederate (const std::string &fedName, FederateInfo &fi)
    {
        fed = helicsCreateMessageFederate (fedName.c_str (), fi.getInfo (), hThrowOnError ());
    }

    explicit MessageFederate (const std::string &configString)
    {
        fed = helicsCreateMessageFederateFromConfig (configString.c_str (), hThrowOnError ());
    }

    // Default constructor, not meant to be used
    MessageFederate () {}

    /** Methods for registering endpoints **/
    Endpoint registerEndpoint (const std::string &name, const std::string &type = std::string ())
    {
        helics_endpoint ep = helicsFederateRegisterEndpoint (fed, name.c_str (), type.c_str (), hThrowOnError ());
        local_endpoints.push_back (ep);
        return Endpoint (ep);
    }

    Endpoint registerGlobalEndpoint (const std::string &name, const std::string &type = std::string ())
    {
        helics_endpoint ep =
          helicsFederateRegisterGlobalEndpoint (fed, name.c_str (), type.c_str (), hThrowOnError ());
        local_endpoints.push_back (ep);
        return Endpoint (ep);
    }

    Endpoint getEndpoint (const std::string &name)
    {
        return Endpoint (helicsFederateGetEndpoint (fed, name.c_str (), hThrowOnError ()));
    }
    Endpoint getEndpoint (int index)
    {
        return Endpoint (helicsFederateGetEndpointByIndex (fed, index, hThrowOnError ()));
    }

    /** Checks if federate has any messages **/
    bool hasMessage () const
    {
        // returns int, 1 = true, 0 = false
        return (helicsFederateHasMessage (fed) > 0);
    }

    /** Returns the number of pending receives for all endpoints. **/
    int pendingMessages () const { return helicsFederatePendingMessages (fed); }

    /** Get a packet for any endpoints in the federate **/
    helics_message getMessage () { return helicsFederateGetMessage (fed); }

  private:
    std::vector<helics_endpoint> local_endpoints;
};
}  // namespace helicscpp
#endif
