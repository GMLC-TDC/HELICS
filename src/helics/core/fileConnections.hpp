/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "../common/JsonProcessingFunctions.hpp"
#include "../common/TomlProcessingFunctions.hpp"
#include "../common/addTargets.hpp"
#include "Broker.hpp"
#include "Core.hpp"
#include "core-exceptions.hpp"

#include <string>
#include <type_traits>

namespace helics {
template<class brkX>
void makeConnectionsToml(brkX* brk, const std::string& file)
{
    toml::value uVal;
    static_assert(std::is_base_of<Broker, brkX>::value || std::is_base_of<Core, brkX>::value,
                  "broker must be Core or broker");
    toml::value doc;
    try {
        doc = loadToml(file);
    }
    catch (const std::invalid_argument& ia) {
        throw(helics::InvalidParameter(ia.what()));
    }

    auto conns = toml::find_or(doc, "connections", uVal);
    if (!conns.is_uninitialized()) {
        auto& connArray = conns.as_array();
        for (const auto& conn : connArray) {
            if (conn.is_array()) {
                auto& connAct = conn.as_array();
                brk->dataLink(connAct[0].as_string(), connAct[1].as_string());
            } else {
                std::string pub = getOrDefault(conn, "publication", std::string());
                if (!pub.empty()) {
                    addTargets(conn, "targets", [brk, &pub](const std::string& target) {
                        brk->dataLink(pub, target);
                    });
                } else {
                    std::string ipt = getOrDefault(conn, "input", std::string());
                    addTargets(conn, "targets", [brk, &ipt](const std::string& target) {
                        brk->dataLink(target, ipt);
                    });
                }
            }
        }
    }
    auto filts = toml::find_or(doc, "filters", uVal);
    if (!filts.is_uninitialized()) {
        auto& filtArray = filts.as_array();
        for (const auto& filt : filtArray) {
            if (filt.is_array()) {
                auto& filtAct = filt.as_array();
                brk->addSourceFilterToEndpoint(filtAct[0].as_string(), filtAct[1].as_string());
            } else {
                std::string fname = getOrDefault(filt, "filter", std::string());
                if (!fname.empty()) {
                    auto asrc = [brk, &fname](const std::string& ept) {
                        brk->addSourceFilterToEndpoint(fname, ept);
                    };
                    addTargets(filt, "endpoints", asrc);
                    addTargets(filt, "source_endpoints", asrc);
                    addTargets(filt, "sourceEndpoints", asrc);
                    auto adst = [brk, &fname](const std::string& ept) {
                        brk->addDestinationFilterToEndpoint(fname, ept);
                    };
                    addTargets(filt, "dest_endpoints", adst);
                    addTargets(filt, "destEndpoints", adst);
                }
            }
        }
    }
    auto globals = toml::find_or(doc, "globals", uVal);
    if (!globals.is_uninitialized()) {
        if (globals.is_array()) {
            for (auto& val : globals.as_array()) {
                brk->setGlobal(val.as_array()[0].as_string(), val.as_array()[1].as_string());
            }
        } else {
            for (const auto& val : globals.as_table()) {
                brk->setGlobal(val.first, val.second.as_string());
            }
        }
    }
}

template<class brkX>
void makeConnectionsJson(brkX* brk, const std::string& file)
{
    static_assert(std::is_base_of<Broker, brkX>::value || std::is_base_of<Core, brkX>::value,
                  "broker must be Core or broker");
    Json::Value doc;
    try {
        doc = loadJson(file);
    }
    catch (const std::invalid_argument& ia) {
        throw(helics::InvalidParameter(ia.what()));
    }

    if (doc.isMember("connections")) {
        for (const auto& conn : doc["connections"]) {
            if (conn.isArray()) {
                brk->dataLink(conn[0].asString(), conn[1].asString());
            } else {
                std::string pub = getOrDefault(conn, "publication", std::string());
                if (!pub.empty()) {
                    addTargets(conn, "targets", [brk, &pub](const std::string& target) {
                        brk->dataLink(pub, target);
                    });
                } else {
                    std::string ipt = getOrDefault(conn, "input", std::string());
                    addTargets(conn, "targets", [brk, &ipt](const std::string& target) {
                        brk->dataLink(target, ipt);
                    });
                }
            }
        }
    }
    if (doc.isMember("filters")) {
        for (const auto& filt : doc["filters"]) {
            if (filt.isArray()) {
                brk->addSourceFilterToEndpoint(filt[0].asString(), filt[1].asString());
            } else {
                std::string fname = getOrDefault(filt, "filter", std::string());
                if (!fname.empty()) {
                    auto asrc = [brk, &fname](const std::string& ept) {
                        brk->addSourceFilterToEndpoint(fname, ept);
                    };
                    addTargets(filt, "endpoints", asrc);
                    addTargets(filt, "source_endpoints", asrc);
                    addTargets(filt, "sourceEndpoints", asrc);
                    auto adst = [brk, &fname](const std::string& ept) {
                        brk->addDestinationFilterToEndpoint(fname, ept);
                    };
                    addTargets(filt, "dest_endpoints", adst);
                    addTargets(filt, "destEndpoints", adst);
                }
            }
        }
    }
    if (doc.isMember("globals")) {
        if (doc["globals"].isArray()) {
            for (auto& val : doc["globals"]) {
                brk->setGlobal(val[0].asString(), val[1].asString());
            }
        } else {
            auto members = doc["globals"].getMemberNames();
            for (auto& val : members) {
                brk->setGlobal(val, doc["globals"][val].asString());
            }
        }
    }
}

}  // namespace helics
