/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#ifndef HELICS_CPP98_COMBINATION_FEDERATE_HPP_
#define HELICS_CPP98_COMBINATION_FEDERATE_HPP_
#pragma once

#include "MessageFederate.hpp"
#include "ValueFederate.hpp"

namespace helicscpp
{
/** combination federate object in the C++98 API*/
class CombinationFederate : public ValueFederate, public MessageFederate
{
  public:
    explicit CombinationFederate (const std::string &name, FederateInfo &fi)
    {
        fed = helicsCreateCombinationFederate (name.c_str (), fi.getInfo (), hThrowOnError ());
    }

    explicit CombinationFederate (const std::string &configString)
    {
        fed = helicsCreateCombinationFederateFromConfig (configString.c_str (), hThrowOnError ());
    }
};
}  // namespace helicscpp
#endif
