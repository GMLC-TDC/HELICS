/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#ifndef HELICS_CPP98_COMBINATION_FEDERATE_HPP_
#define HELICS_CPP98_COMBINATION_FEDERATE_HPP_
#pragma once

#include "MessageFederate.hpp"
#include "ValueFederate.hpp"

#include <string>

namespace helicscpp {
/** combination federate object in the C++98 API
@details a combination federate merges a \ref ValueFederate and a \ref MessageFederate*/
class CombinationFederate: public ValueFederate, public MessageFederate {
  public:
    /** construct a combination federate
    @param name the name of the federate
    @param fi the federateInfo object to use for the construction information*/
    explicit CombinationFederate(const std::string& name, FederateInfo& fi)
    {
        fed = helicsCreateCombinationFederate(name.c_str(), fi.getInfo(), hThrowOnError());
    }
    /** construct a combination Federate from a configuration string either a file or json/toml
     * string*/
    explicit CombinationFederate(const std::string& configString)
    {
        fed = helicsCreateCombinationFederateFromConfig(configString.c_str(), hThrowOnError());
    }
};
}  // namespace helicscpp
#endif
