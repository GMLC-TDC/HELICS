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
#include <vector>

namespace helics {

static Json::Value getSection(Json::Value& base, const std::string& subSection, int16_t configIndex)
{
    if (!subSection.empty()) {
        auto cfg = base[subSection];
        if (cfg.isObject()) {
            return cfg;
        }
        if (cfg.isArray()) {
            return cfg[configIndex];
        }
        if (cfg.isNull() && subSection.find_first_of('.') != std::string::npos) {
            auto dotloc = subSection.find_first_of('.');
            auto sub1 = base[subSection.substr(0, dotloc)];
            if (!sub1.isNull()) {
                return getSection(sub1, subSection.substr(dotloc + 1), configIndex);
            }
        }
        return Json::nullValue;
    }
    return base;
}

std::vector<CLI::ConfigItem> HelicsConfigJSON::from_config(std::istream& input) const
{
    Json::CharReaderBuilder rbuilder;
    rbuilder["collectComments"] = false;
    std::string errs;
    if (!mSkipJson) {
        Json::Value config;
        if (Json::parseFromStream(rbuilder, input, &config, &errs)) {
            config = getSection(config, section(), configIndex);
            if (config.isNull()) {
                if (mFallbackToDefault) {
                    return ConfigBase::from_config(input);
                }
                return {};
            }
            return fromConfigInternal(config);
        }
        if (mThrowJsonErrors && !errs.empty()) {
            throw(CLI::FileError(errs));
        }
    }
    return ConfigBase::from_config(input);
}

std::vector<CLI::ConfigItem>
    HelicsConfigJSON::fromConfigInternal(Json::Value json,
                                         const std::string& name,
                                         const std::vector<std::string>& prefix) const
{
    std::vector<CLI::ConfigItem> results;

    if (json.isObject()) {
        if (prefix.size() > maximumLayers) {
            return results;
        }
        auto fields = json.getMemberNames();
        for (auto& fld : fields) {
            auto copy_prefix = prefix;
            if (!name.empty()) {
                if (name != mPromoteSection) {
                    copy_prefix.push_back(name);
                }
            }
            auto sub_results = fromConfigInternal(json[fld], fld, copy_prefix);
            results.insert(results.end(), sub_results.begin(), sub_results.end());
        }
    } else if (!name.empty()) {
        results.emplace_back();
        CLI::ConfigItem& res = results.back();
        res.name = name;
        res.parents = prefix;
        if (json.isBool()) {
            res.inputs = {json.asBool() ? "true" : "false"};
        } else if (json.isNumeric()) {
            std::stringstream outstring;
            outstring << json.asDouble();
            res.inputs = {outstring.str()};
        } else if (json.isString()) {
            res.inputs = {json.asString()};
        } else if (json.isArray()) {
            for (const auto& obj : json) {
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
