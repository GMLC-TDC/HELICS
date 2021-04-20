/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "JsonProcessingFunctions.hpp"
#include "TomlProcessingFunctions.hpp"

#include <string>

/** check if a string looks like a recognized file or a command line of some kind
Does not check actual file existence or do any other verification other than do a preliminary check
if the string could be a json|toml|ini file or looks like a json string, and if it does return true
otherwise return false
*/
bool looksLikeFile(const std::string& configString);
