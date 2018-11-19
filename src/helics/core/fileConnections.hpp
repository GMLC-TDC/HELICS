/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once

#include "../common/JsonProcessingFunctions.hpp"
#include "../common/TomlProcessingFunctions.hpp"
#include "core-exceptions.hpp"
#include <string>
#include <type_traits>
#include "Broker.hpp"
#include "Core.hpp"

namespace helics
{

template<class brkX>
void makeConnectionsToml(brkX *brk, const std::string &file)
{
    static_assert(std::is_base_of <Broker, brkX>::value || std::is_base_of <Core, brkX>::value, "broker must be Core or broker");
    toml::Value doc;
    try
    {
        doc = loadToml(file);
    }
    catch (const std::invalid_argument &ia)
    {
        throw (helics::InvalidParameter(ia.what()));
    }

    auto conns = doc.find("connections");
    if (conns != nullptr)
    {
        auto &connArray = conns->as<toml::Array>();
        for (const auto &conn : connArray)
        {
            if (conn.is<toml::Array>())
            {
                auto &connAct = conn.as<toml::Array>();
                brk->dataLink(connAct[0].as<std::string>(), connAct[1].as<std::string>());
            }
            else
            {
                std::string pub = tomlGetOrDefault(conn, "publication", std::string());
                if (conn.has("targets"))
                {
                    auto &targetArray = conn["targets"].as<toml::Array>();
                    for (const auto &target : targetArray)
                    {
                        brk->dataLink(pub, target.as<std::string>());
                    }
                }
            }
        }
    }
}

template<class brkX>
void makeConnectionsJson(brkX *brk, const std::string &file)
{
    static_assert(std::is_base_of <Broker, brkX>::value || std::is_base_of <Core, brkX>::value, "broker must be Core or broker");
    auto doc = loadJson(file);

    if (doc.isMember("connections"))
    {
        for (const auto &conn : doc["connections"])
        {
            if (conn.isArray())
            {
                brk->dataLink(conn[0].asString(), conn[1].asString());
            }
            else
            {
                std::string pub = jsonGetOrDefault(conn, "publication", std::string());
                if (conn.isMember("targets"))
                {
                    for (const auto &target : conn["targets"])
                    {
                        brk->dataLink(pub, target.asString());
                    }
                }
            }
        }
    }
}

} //namespace helics
