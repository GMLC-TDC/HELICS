/*
Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef HELICS_CPP98_COMBINATION_FEDERATE_HPP_
#define HELICS_CPP98_COMBINATION_FEDERATE_HPP_
#pragma once

#include "ValueFederate.hpp"
#include "MessageFederate.hpp"

namespace helics
{
class CombinationFederate : public ValueFederate, public MessageFederate
{
  public:
    explicit CombinationFederate (FederateInfo &fi)
    {
        fed = helicsCreateCombinationFederate (fi.getInfo());
    }

    explicit CombinationFederate (const std::string &jsonString)
    {
        fed = helicsCreateCombinationFederateFromJson (jsonString.c_str());
    }
};
} //namespace helics
#endif
