/*
Copyright (c) 2017-2024,
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

namespace helics::fileops {

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
                brk->dataLink(connAct[0].as_string().str, connAct[1].as_string().str);
            } else {
                std::string pub = getOrDefault(conn, "publication", std::string());
                if (!pub.empty()) {
                    addTargets(conn, "targets", [brk, &pub](const std::string& target) {
                        brk->dataLink(pub, target);
                    });
                } else {
                    std::string ipt = getOrDefault(conn, "input", std::string());
                    if (!ipt.empty()) {
                        addTargets(conn, "targets", [brk, &ipt](const std::string& target) {
                            brk->dataLink(target, ipt);
                        });
                        addTargets(conn, "sources", [brk, &ipt](const std::string& source) {
                            brk->dataLink(source, ipt);
                        });
                    } else {
                        std::string ept = getOrDefault(conn, "endpoint", std::string());
                        if (!ept.empty()) {
                            addTargets(conn, "targets", [brk, &ept](const std::string& target) {
                                brk->linkEndpoints(ept, target);
                            });
                            addTargets(conn, "sources", [brk, &ept](const std::string& source) {
                                brk->linkEndpoints(source, ept);
                            });
                        }
                    }
                }
            }
        }
    }
    auto lnks = toml::find_or(doc, "links", uVal);
    if (!lnks.is_uninitialized()) {
        auto& connArray = lnks.as_array();
        for (const auto& conn : connArray) {
            if (conn.is_array()) {
                auto& connAct = conn.as_array();
                brk->linkEndpoints(connAct[0].as_string().str, connAct[1].as_string().str);
            } else {
                std::string pub = getOrDefault(conn, "publication", std::string());
                if (!pub.empty()) {
                    addTargets(conn, "targets", [brk, &pub](const std::string& target) {
                        brk->dataLink(pub, target);
                    });
                } else {
                    std::string ipt = getOrDefault(conn, "input", std::string());
                    if (!ipt.empty()) {
                        addTargets(conn, "targets", [brk, &ipt](const std::string& target) {
                            brk->dataLink(target, ipt);
                        });
                        addTargets(conn, "sources", [brk, &ipt](const std::string& source) {
                            brk->dataLink(source, ipt);
                        });
                    } else {
                        std::string ept = getOrDefault(conn, "endpoint", std::string());
                        if (!ept.empty()) {
                            addTargets(conn, "targets", [brk, &ept](const std::string& target) {
                                brk->linkEndpoints(ept, target);
                            });
                            addTargets(conn, "sources", [brk, &ept](const std::string& source) {
                                brk->linkEndpoints(source, ept);
                            });
                        }
                    }
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
                brk->addSourceFilterToEndpoint(filtAct[0].as_string().str,
                                               filtAct[1].as_string().str);
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
                brk->setGlobal(val.as_array()[0].as_string().str,
                               val.as_array()[1].as_string().str);
            }
        } else {
            for (const auto& val : globals.as_table()) {
                brk->setGlobal(val.first, val.second.as_string().str);
            }
        }
    }
    auto aliases = toml::find_or(doc, "aliases", uVal);
    if (!aliases.is_uninitialized()) {
        if (aliases.is_array()) {
            for (auto& val : aliases.as_array()) {
                brk->addAlias(val.as_array()[0].as_string().str, val.as_array()[1].as_string().str);
            }
        } else {
            for (const auto& val : aliases.as_table()) {
                brk->addAlias(val.first, val.second.as_string().str);
            }
        }
    }
}

template<class brkX>
void makeConnectionsJson(brkX* brk, const std::string& file)
{
    static_assert(std::is_base_of<Broker, brkX>::value || std::is_base_of<Core, brkX>::value,
                  "input must be Core or Broker");
    Json::Value doc;
    try {
        doc = loadJson(file);
    }
    catch (const std::invalid_argument& ia) {
        throw(helics::InvalidParameter(ia.what()));
    }

    if (doc.isMember("connections")) {
        for (const auto& conn : doc["connections"]) {
            if (conn.isArray() && conn.size() >= 2) {
                brk->dataLink(conn[0].asString(), conn[1].asString());
            } else {
                std::string pub = fileops::getOrDefault(conn, "publication", std::string_view());
                if (!pub.empty()) {
                    addTargets(conn, "targets", [brk, &pub](std::string_view target) {
                        brk->dataLink(pub, target);
                    });
                } else {
                    std::string ipt = fileops::getOrDefault(conn, "input", std::string_view());
                    if (!ipt.empty()) {
                        addTargets(conn, "targets", [brk, &ipt](std::string_view target) {
                            brk->dataLink(target, ipt);
                        });
                        addTargets(conn, "sources", [brk, &ipt](std::string_view source) {
                            brk->dataLink(source, ipt);
                        });
                    } else {
                        std::string ept =
                            fileops::getOrDefault(conn, "endpoint", std::string_view());
                        if (!ept.empty()) {
                            addTargets(conn, "targets", [brk, &ept](std::string_view target) {
                                brk->linkEndpoints(ept, target);
                            });
                            addTargets(conn, "sources", [brk, &ept](std::string_view source) {
                                brk->linkEndpoints(source, ept);
                            });
                        }
                    }
                }
            }
        }
    }
    if (doc.isMember("links")) {
        for (const auto& conn : doc["links"]) {
            if (conn.isArray() && conn.size() >= 2) {
                brk->linkEndpoints(conn[0].asString(), conn[1].asString());
            } else {
                std::string pub = fileops::getOrDefault(conn, "publication", std::string_view());
                if (!pub.empty()) {
                    addTargets(conn, "targets", [brk, &pub](std::string_view target) {
                        brk->dataLink(pub, target);
                    });
                } else {
                    std::string ipt = fileops::getOrDefault(conn, "input", std::string());
                    if (!ipt.empty()) {
                        addTargets(conn, "targets", [brk, &ipt](std::string_view target) {
                            brk->dataLink(target, ipt);
                        });
                        addTargets(conn, "sources", [brk, &ipt](std::string_view source) {
                            brk->dataLink(source, ipt);
                        });
                    } else {
                        std::string ept = fileops::getOrDefault(conn, "endpoint", std::string());
                        if (!ept.empty()) {
                            addTargets(conn, "targets", [brk, &ept](std::string_view target) {
                                brk->linkEndpoints(ept, target);
                            });
                            addTargets(conn, "sources", [brk, &ept](std::string_view source) {
                                brk->linkEndpoints(source, ept);
                            });
                        }
                    }
                }
            }
        }
    }
    if (doc.isMember("filters")) {
        for (const auto& filt : doc["filters"]) {
            if (filt.isArray()) {
                brk->addSourceFilterToEndpoint(filt[0].asString(), filt[1].asString());
            } else {
                std::string fname = fileops::getOrDefault(filt, "filter", std::string_view());
                if (!fname.empty()) {
                    auto asrc = [brk, &fname](std::string_view ept) {
                        brk->addSourceFilterToEndpoint(fname, ept);
                    };
                    addTargets(filt, "endpoints", asrc);
                    addTargets(filt, "source_endpoints", asrc);
                    addTargets(filt, "sourceEndpoints", asrc);
                    auto adst = [brk, &fname](std::string_view ept) {
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

    if (doc.isMember("aliases")) {
        if (doc["aliases"].isArray()) {
            for (auto& val : doc["aliases"]) {
                brk->addAlias(val[0].asString(), val[1].asString());
            }
        } else {
            auto members = doc["aliases"].getMemberNames();
            for (auto& val : members) {
                brk->addAlias(val, doc["aliases"][val].asString());
            }
        }
    }

    if constexpr (std::is_base_of<Core, brkX>::value) {
        loadTags(doc, [brk](std::string_view tagname, std::string_view tagvalue) {
            brk->setFederateTag(gLocalCoreId, tagname, tagvalue);
        });
    }
}
}  // namespace helics::fileops
