/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#ifndef HELICS_CPP98_COMBINATION_FEDERATE_HPP_
#define HELICS_CPP98_COMBINATION_FEDERATE_HPP_
#pragma once

#include "ValueFederate.hpp"
#include "MessageFederate.hpp"

namespace helics98
{
class CombinationFederate : public ValueFederate, public MessageFederate
{
  public:
    explicit CombinationFederate (const std::string &name, FederateInfo &fi)
    {
        fed = helicsCreateCombinationFederate (name.c_str(),fi.getInfo());
    }
    
    explicit CombinationFederate (const std::string &configString)
    {
        fed = helicsCreateCombinationFederateFromConfig (configString.c_str());
    }
};
} //namespace helics
#endif

