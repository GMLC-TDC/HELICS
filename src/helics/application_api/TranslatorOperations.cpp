/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "TranslatorOperations.hpp"

#include "../core/Core.hpp"
#include "../core/core-exceptions.hpp"
#include "../utilities/timeStringOps.hpp"

#include <algorithm>
#include <iostream>
#include <map>
#include <memory>
#include <random>
#include <regex>
#include <thread>

namespace helics {
void TranslatorOperations::set(const std::string& /*property*/, double /*val*/) {}
void TranslatorOperations::setString(const std::string& /*property*/, const std::string& /*val*/) {}

JsonTranslatorOperation::JsonTranslatorOperation()
{
    
}


std::shared_ptr<TranslatorOperator> JsonTranslatorOperation::getOperator()
{
    return std::static_pointer_cast<TranslatorOperator>(to);
}


}  // namespace helics
