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
                    auto &targetArray = conn.get<toml::Array>("targets");
                    for (const auto &target : targetArray)
                    {
                        brk->dataLink(pub, target.as<std::string>());
                    }
                }
            }
        }
    }
    auto filts = doc.find("filters");
    if (filts != nullptr)
    {
        auto &filtArray = filts->as<toml::Array>();
        for (const auto &filt : filtArray)
        {
            if (filt.is<toml::Array>())
            {
                auto &filtAct = filt.as<toml::Array>();
                brk->addSourceFilterToEndpoint(filtAct[0].as<std::string>(), filtAct[1].as<std::string>());
            }
            else
            {
                std::string fname = tomlGetOrDefault(filt, "filter", std::string());
                if (filt.has("endpoints"))
                {
                    auto &targetArray = filt.get<toml::Array>("endpoints");
                    for (const auto &target : targetArray)
                    {
                        brk->addSourceFilterToEndpoint(fname, target.as<std::string>());
                    }
                }
                if (filt.has("source_endpoints"))
                {
                    auto &targetArray = filt.get<toml::Array>("source_endpoints");
                    for (const auto &target : targetArray)
                    {
                        brk->addSourceFilterToEndpoint(fname, target.as<std::string>());
                    }
                }
                if (filt.has("dest_endpoints"))
                {
                    auto &targetArray = filt.get<toml::Array>("dest_endpoints");
                    for (const auto &target : targetArray)
                    {
                        brk->addDestinationFilterToEndpoint(fname, target.as<std::string>());
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
    if (doc.isMember("filters"))
    {
        for (const auto &filt : doc["filters"])
        {
            if (filt.isArray())
            {
                brk->addSourceFilterToEndpoint(filt[0].asString(), filt[1].asString());
            }
            else
            {
                std::string fname = jsonGetOrDefault(filt, "filter", std::string());
                if (filt.isMember("endpoints"))
                {
                    for (const auto &target : filt["endpoints"])
                    {
                        brk->addSourceFilterToEndpoint(fname, target.asString());
                    }
                }
                if (filt.isMember("source_endpoints"))
                {
                    for (const auto &target : filt["source_endpoints"])
                    {
                        brk->addSourceFilterToEndpoint(fname, target.asString());
                    }
                }
                if (filt.isMember("dest_endpoints"))
                {
                    for (const auto &target : filt["dest_endpoints"])
                    {
                        brk->addDestinationFilterToEndpoint(fname, target.asString());
                    }
                }
            }
        }
    }
}

} //namespace helics
