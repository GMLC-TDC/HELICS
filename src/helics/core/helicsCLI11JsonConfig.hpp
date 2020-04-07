/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include "helicsCLI11.hpp"
#include "json/forwards.h"

namespace helics
{
    /// Define a json parser for the config files for CLI11
    class HelicsConfigJSON : public CLI::ConfigBase {
    public:


        std::vector<CLI::ConfigItem> from_config(std::istream &input) const override final;

        /// Internal parser for the configuration
        std::vector<CLI::ConfigItem>
            _from_config(Json::Value j, std::string name = "", std::vector<std::string> prefix = {}) const;
        /// Set the maximum recursive levels to process
        void set_max_level(std::size_t max_levels) { max_levels = max_levels; }
    private:
        std::size_t max_levels_{ 100 };
    };

}
