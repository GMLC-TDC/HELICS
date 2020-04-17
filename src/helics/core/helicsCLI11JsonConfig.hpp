/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include "helicsCLI11.hpp"

#include "json/forwards.h"
#include <string>
#include <utility>
#include <vector>

namespace helics {
/// Define a json parser for the config files for CLI11
class HelicsConfigJSON: public CLI::ConfigBase {
  public:
    std::vector<CLI::ConfigItem> from_config(std::istream& input) const override final;

    /// Internal parser for the configuration
    std::vector<CLI::ConfigItem>
        _from_config(Json::Value j, std::string name = "", std::vector<std::string> prefix = {})
            const;
    /// skip checking the JSON and go directory to the TOML processing
    void skipJson(bool skj = true) { skip_json_ = skj; }
    /// set a specific path to check and ignore others
    void setKeyPath(std::vector<std::string> paths) { keyPaths = std::move(paths); }

  private:
    bool skip_json_{false};
    std::vector<std::string> keyPaths;
};

} // namespace helics
