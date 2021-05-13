/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "helicsCLI11JsonConfig.hpp"

#include "../common/JsonProcessingFunctions.hpp"

#include <memory>
#include <string>
#include <vector>

namespace helics {

std::vector<CLI::ConfigItem> HelicsConfigJSON::from_config(std::istream& input) const
{
    Json::CharReaderBuilder rbuilder;
    rbuilder["collectComments"] = false;
    std::string errs;
    if (!skip_json_) {
        Json::Value config;
        if (Json::parseFromStream(rbuilder, input, &config, &errs)) {
            if (!section().empty()) {
                auto cfg = config[section()];
                if (cfg.isObject()) {
                    config = std::move(cfg);
                } else if (cfg.isArray()) {
                    config = cfg[configIndex];
                    if (config.isNull()) {
                        return {};
                    }
                } else if (!fallback_to_default_) {
                    return {};
                }
            }
            return fromConfigInternal(config);
        }
    }
    return ConfigBase::from_config(input);
}

std::vector<CLI::ConfigItem>
    HelicsConfigJSON::fromConfigInternal(Json::Value j,
                                         const std::string& name,
                                         const std::vector<std::string>& prefix) const
{
    std::vector<CLI::ConfigItem> results;

    if (j.isObject()) {
        if (prefix.size() > maxLayers_) {
            return results;
        }
        auto fields = j.getMemberNames();
        for (auto& fld : fields) {
            auto copy_prefix = prefix;
            if (!name.empty()) {
                copy_prefix.push_back(name);
            }
            auto sub_results = fromConfigInternal(j[fld], fld, copy_prefix);
            results.insert(results.end(), sub_results.begin(), sub_results.end());
        }
    } else if (!name.empty()) {
        results.emplace_back();
        CLI::ConfigItem& res = results.back();
        res.name = name;
        res.parents = prefix;
        if (j.isBool()) {
            res.inputs = {j.asBool() ? "true" : "false"};
        } else if (j.isNumeric()) {
            std::stringstream ss;
            ss << j.asDouble();
            res.inputs = {ss.str()};
        } else if (j.isString()) {
            res.inputs = {j.asString()};
        } else if (j.isArray()) {
            for (const auto& obj : j) {
                if (obj.isString()) {
                    res.inputs.push_back(obj.asString());
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
    app->config_formatter(std::move(fmtr));
    return fmtrRet;
}
}  // namespace helics
