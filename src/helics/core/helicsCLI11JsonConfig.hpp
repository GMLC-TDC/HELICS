/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include "helicsCLI11.hpp"

#include "json/forwards.h"
#include <string>
#include <utility>
#include <vector>

namespace helics {
/// Define a JSON parser for the config files for CLI11
class HelicsConfigJSON: public CLI::ConfigBase {
  public:
    std::vector<CLI::ConfigItem> from_config(std::istream& input) const override final;

    /// skip checking the JSON and go directory to the TOML processing
    void skipJson(bool skj = true) { skip_json_ = skj; }
    /// if the specified section isn't available use the root section
    void fallbackToDefault(bool ftd = true) { fallback_to_default_ = ftd; }

  private:
    bool skip_json_{false};
    bool fallback_to_default_{false};
    /// Internal parser for the configuration
    std::vector<CLI::ConfigItem>
        fromConfigInternal(Json::Value j,
                           const std::string& name = {},
                           const std::vector<std::string>& prefix = {}) const;
};
/// Add the HELICS JSON configuration processor to the app
HelicsConfigJSON* addJsonConfig(CLI::App* app);

}  // namespace helics
