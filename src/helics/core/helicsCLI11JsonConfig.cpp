/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "helicsCLI11JsonConfig.hpp"

#include "../common/JsonProcessingFunctions.hpp"

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace helics {

static nlohmann::json
    getSection(nlohmann::json& base, const std::string& subSection, int16_t configIndex)
{
    if (!subSection.empty()) {
        auto cfg = base[subSection];
        if (cfg.is_object()) {
            return cfg;
        }
        if (cfg.is_array()) {
            return cfg[configIndex];
        }
        if (cfg.is_null() && subSection.find_first_of('.') != std::string::npos) {
            auto dotloc = subSection.find_first_of('.');
            auto sub1 = base[subSection.substr(0, dotloc)];
            if (!sub1.is_null()) {
                return getSection(sub1, subSection.substr(dotloc + 1), configIndex);
            }
        }
        return nlohmann::json::object();
    }
    return base;
}

std::vector<CLI::ConfigItem> HelicsConfigJSON::from_config(std::istream& input) const
{
    std::string errs;
    if (!mSkipJson) {
        nlohmann::json config;
        try {
            config = nlohmann::json::parse(input, nullptr, true, true);
            config = getSection(config, section(), configIndex);
            if (config.is_null()) {
                if (mFallbackToDefault) {
                    return ConfigBase::from_config(input);
                }
                return {};
            }
            return fromConfigInternal(config);
        }
        catch (const nlohmann::json::parse_error& err) {
            errs = err.what();
        }
        if (mThrowJsonErrors && !errs.empty()) {
            throw(CLI::FileError(errs));
        }
    }
    return ConfigBase::from_config(input);
}

std::vector<CLI::ConfigItem>
    HelicsConfigJSON::fromConfigInternal(const nlohmann::json& json,
                                         const std::string& name,
                                         const std::vector<std::string>& prefix) const
{
    std::vector<CLI::ConfigItem> results;

    if (json.is_object()) {
        if (prefix.size() > maximumLayers) {
            return results;
        }
        for (auto& fld : json.items()) {
            auto copy_prefix = prefix;
            if (!name.empty()) {
                if (name != mPromoteSection) {
                    copy_prefix.push_back(name);
                }
            }
            auto sub_results = fromConfigInternal(fld.value(), fld.key(), copy_prefix);
            results.insert(results.end(), sub_results.begin(), sub_results.end());
        }
    } else if (!name.empty()) {
        results.emplace_back();
        CLI::ConfigItem& res = results.back();
        res.name = name;
        res.parents = prefix;
        if (json.is_boolean()) {
            res.inputs = {json.get<bool>() ? "true" : "false"};
        } else if (json.is_number()) {
            std::stringstream outstring;
            outstring << json.get<double>();
            res.inputs = {outstring.str()};
        } else if (json.is_string()) {
            res.inputs = {json.get<std::string>()};
        } else if (json.is_array()) {
            for (const auto& obj : json) {
                if (obj.is_string()) {
                    res.inputs.push_back(obj.get<std::string>());
                } else {
                    break;
                }
            }
        } else {
            throw CLI::ConversionError("Failed to convert " + name);
        }
    } else {
        throw CLI::ConversionError("You must make all top level values objects in json!");
    }

    return results;
}

HelicsConfigJSON* addJsonConfig(CLI::App* app)
{
    auto fmtr = std::make_shared<HelicsConfigJSON>();
    auto* fmtrRet = fmtr.get();
    app->allow_config_extras(CLI::config_extras_mode::ignore_all);
    app->add_option("--config_section",
                    fmtr->sectionRef(),
                    "specify the section of the config file to use")
        ->configurable(false)
        ->trigger_on_parse();
    app->add_option("--config_index",
                    fmtr->indexRef(),
                    "specify the section index of the config file to use for configuration arrays")
        ->configurable(false)
        ->trigger_on_parse();
    app->get_config_ptr()->check([fmtr](const std::string& filename) {
        if (CLI::ExistingFile(filename).empty()) {
            fmtr->skipJson(!fileops::hasJsonExtension(filename));
        }
        return std::string{};
    });

    app->config_formatter(std::move(fmtr));
    return fmtrRet;
}
}  // namespace helics
