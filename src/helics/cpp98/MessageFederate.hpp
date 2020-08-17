/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#ifndef HELICS_CPP98_MESSAGE_FEDERATE_HPP_
#define HELICS_CPP98_MESSAGE_FEDERATE_HPP_
#pragma once

#include "../shared_api_library/MessageFederate.h"
#include "Endpoint.hpp"
#include "Federate.hpp"

#include <string>
#include <vector>

namespace helicscpp {
/** class defining the block communication based interface */

class MessageFederate: public virtual Federate {
  public:
    /**constructor taking a federate information structure and using the default core
  @param fedName the name of the messageFederate, can be left empty to use a default or one from fi
  @param fi  a federate information structure
  */
    MessageFederate(const std::string& fedName, FederateInfo& fi)
    {
        fed = helicsCreateMessageFederate(fedName.c_str(), fi.getInfo(), hThrowOnError());
    }

    /**constructor taking a string with the required information
   @param configString can be either a JSON file, TOML file or a string containing JSON code
   */
    explicit MessageFederate(const std::string& configString)
    {
        fed = helicsCreateMessageFederateFromConfig(configString.c_str(), hThrowOnError());
    }

    /** Default constructor, not meant to be used*/
    MessageFederate() HELICS_NOTHROW {}

    /** Methods for registering endpoints **/

    /** register an endpoint
    @details call is only valid in startup mode
    @param name the name of the endpoint
    @param type the defined type of the interface for endpoint checking if requested
    @return an Endpoint Object
    */
    Endpoint registerEndpoint(const std::string& name, const std::string& type = std::string())
    {
        helics_endpoint ep =
            helicsFederateRegisterEndpoint(fed, name.c_str(), type.c_str(), hThrowOnError());
        local_endpoints.push_back(ep);
        return Endpoint(ep);
    }

    /** register an endpoint directly without prepending the federate name
    @param name the name of the endpoint
    @param type the defined type of the interface for endpoint checking if requested
     @return an Endpoint Object
    */
    Endpoint registerGlobalEndpoint(const std::string& name,
                                    const std::string& type = std::string())
    {
        helics_endpoint ep =
            helicsFederateRegisterGlobalEndpoint(fed, name.c_str(), type.c_str(), hThrowOnError());
        local_endpoints.push_back(ep);
        return Endpoint(ep);
    }

    /** get an Endpoint from its name
    @param name the name of the endpoint to retrieve
    @return an Endpoint*/
    Endpoint getEndpoint(const std::string& name)
    {
        return Endpoint(helicsFederateGetEndpoint(fed, name.c_str(), hThrowOnError()));
    }
    /** get an Endpoint from an index
    @param index the index of the endpoint to retrieve index is 0 based
    @return an Endpoint*/
    Endpoint getEndpoint(int index)
    {
        return Endpoint(helicsFederateGetEndpointByIndex(fed, index, hThrowOnError()));
    }

    /** Checks if federate has any messages **/
    bool hasMessage() const
    {
        // returns int, 1 = true, 0 = false
        return (helicsFederateHasMessage(fed) > 0);
    }

    /** Returns the number of pending receives for all endpoints. **/
    int pendingMessages() const { return helicsFederatePendingMessages(fed); }

    /** Get a packet for any endpoints in the federate **/
    Message getMessage() { return Message(helicsFederateGetMessageObject(fed)); }

    /** create a message object */
    Message createMessage()
    {
        return Message(helicsFederateCreateMessageObject(fed, hThrowOnError()));
    }
    /**get the number of registered endpoints*/
    int getEndpointCount() const { return helicsFederateGetEndpointCount(fed); }

  private:
    std::vector<helics_endpoint> local_endpoints;
};
// this code needs the definition of federate before it can de defined
inline Message::Message(const Federate& fed):
    mo(helicsFederateCreateMessageObject(fed.getObject(), hThrowOnError()))
{
}

inline Message& Message::newMessageObject(const Federate& fed)
{
    helics_message_object newmo =
        helicsFederateCreateMessageObject(fed.getObject(), hThrowOnError());
    if (mo != HELICS_NULL_POINTER) {
        helicsMessageFree(mo);
    }
    mo = newmo;
    return *this;
}
}  // namespace helicscpp
#endif
