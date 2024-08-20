/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include "helicsCLI11.hpp"

#include <nlohmann/json_fwd.hpp>
#include <string>
#include <utility>
#include <vector>

namespace helics {
/// Define a JSON parser for the config files for CLI11
class HelicsConfigJSON: public CLI::ConfigBase {
  public:
    std::vector<CLI::ConfigItem> from_config(std::istream& input) const override final;

    /// skip checking the JSON and go directly to the TOML processing
    void skipJson(bool skj = true) { mSkipJson = skj; }
    /// if the specified section isn't available use the root section
    void fallbackToDefault(bool ftd = true) { mFallbackToDefault = ftd; }
    /// throw if the json processing produces errors
    void throwJsonErrors(bool the = true) { mThrowJsonErrors = the; }
    void promoteSection(std::string sectionName) { mPromoteSection = std::move(sectionName); }

  private:
    bool mSkipJson{false};
    bool mFallbackToDefault{false};
    bool mThrowJsonErrors{true};
    std::string mPromoteSection;
    /// Internal parser for the configuration
    std::vector<CLI::ConfigItem>
        fromConfigInternal(const nlohmann::json& json,
                           const std::string& name = {},
                           const std::vector<std::string>& prefix = {}) const;
};
/// Add the HELICS JSON configuration processor to the app
HelicsConfigJSON* addJsonConfig(CLI::App* app);

}  // namespace helics
