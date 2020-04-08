/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "helicsCLI11JsonConfig.hpp"
#include "../common/JsonProcessingFunctions.hpp"

namespace helics
{

    std::vector<CLI::ConfigItem> HelicsConfigJSON::from_config(std::istream &input) const {
        Json::CharReaderBuilder rbuilder;
        rbuilder["collectComments"] = false;
        std::string errs;
        if (!skip_json_)
        {
            Json::Value config;
            if (Json::parseFromStream(rbuilder, input, &config, &errs))
            {
                if (!keyPaths.empty())
                {
                    for (auto &kp : keyPaths)
                    {
                        config = config[kp];
                    }
                }
                return _from_config(config);
            }
        }
        return ConfigBase::from_config(input);
        
    }

    std::vector<CLI::ConfigItem>
        HelicsConfigJSON::_from_config(Json::Value j, std::string name, std::vector<std::string> prefix) const {
        std::vector<CLI::ConfigItem> results;

        if (j.isObject()) {
            if (prefix.size() > maxLayers_)
            {
                return results;
            }
            auto fields = j.getMemberNames();
            for (auto &fld:fields) {
                auto copy_prefix = prefix;
                if (!name.empty()) {
                    copy_prefix.push_back(name);
                }
                auto sub_results = _from_config(j[fld], fld, copy_prefix);
                results.insert(results.end(), sub_results.begin(), sub_results.end());
            }
        }
        else if (!name.empty()) {
            results.emplace_back();
            CLI::ConfigItem &res = results.back();
            res.name = name;
            res.parents = prefix;
            if (j.isBool()) {
                res.inputs = { j.asBool() ? "true" : "false" };
            }
            else if (j.isNumeric()) {
                std::stringstream ss;
                ss << j.asDouble();
                res.inputs = { ss.str() };
            }
            else if (j.isString()) {
                res.inputs = { j.asString() };
            }
            else if (j.isArray()) {
                for (Json::ArrayIndex ii = 0; ii < j.size(); ++ii)
                {
                    if (j[ii].isString())
                    {
                        res.inputs.push_back(j[ii].asString());
                    }
                    else
                    {
                        break;
                    }
                }
            }
            else {
                throw CLI::ConversionError("Failed to convert " + name);
            }
        }
        else {
            throw CLI::ConversionError("You must make all top level values objects in json!");
        }

        return results;
    }

}
