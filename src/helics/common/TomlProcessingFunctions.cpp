/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#include "TomlProcessingFunctions.hpp"
#include <fstream>

bool hasTomlExtension (const std::string &tomlString)
{
    auto ext = tomlString.substr (tomlString.length () - 4);
    return ((ext == "toml") || (ext == "TOML") || (ext == ".ini") || (ext == ".INI"));
}

toml::Value loadToml (const std::string &tomlString)
{
    if (tomlString.size () > 128)
    {
        try
        {
            return loadTomlStr (tomlString);
        }
        catch (const std::invalid_argument &)
        {
            // just pass through this was an assumption
        }
    }
    std::ifstream file (tomlString);

    if (file.is_open ())
    {
        toml::ParseResult pr = toml::parse (file);
        if (!pr.valid ())
        {
            throw (std::invalid_argument (pr.errorReason));
        }

        return pr.value;
    }
    return loadTomlStr (tomlString);
}

toml::Value loadTomlStr (const std::string &tomlString)
{
    std::istringstream tstring (tomlString);
    toml::ParseResult pr = toml::parse (tstring);
    if (pr.valid ())
    {
        return pr.value;
    }
    throw (std::invalid_argument (pr.errorReason));
}

/** read a time from a JSON value element*/
helics::Time loadTomlTime (const toml::Value &timeElement, time_units defaultUnits)
{
    if (timeElement.is<toml::Table> ())
    {
        auto units = timeElement.find ("units");
        if (units != nullptr)
        {
            defaultUnits = helics::timeUnitsFromString (units->as<std::string> ());
        }
        auto val = timeElement.find ("value");
        if (val != nullptr)
        {
            if (val->is<int64_t> ())
            {
                return helics::Time (val->as<int64_t> (), defaultUnits);
            }
            return helics::Time (val->as<double> () * toSecondMultiplier (defaultUnits));
        }
    }
    else if (timeElement.is<int64_t> ())
    {
        return helics::Time (timeElement.as<int64_t> (), defaultUnits);
    }
    else if (timeElement.is<double> ())
    {
        return helics::Time (timeElement.as<double> () * toSecondMultiplier (defaultUnits));
    }
    else if (timeElement.is<toml::Time> ())
    {
        return helics::Time (
          static_cast<std::chrono::nanoseconds> (timeElement.as<toml::Time> ().time_since_epoch ()));
    }
    else
    {
        return helics::loadTimeFromString (timeElement.as<std::string> ());
    }
    return helics::Time::minVal ();
}

std::string getKey (const toml::Value &element)
{
    std::string retval;
    auto mem = element.find ("key");
    if (mem != nullptr)
    {
        retval = mem->as<std::string> ();
    }
    else
    {
        auto name = element.find ("name");
        if (name != nullptr)
        {
            retval = name->as<std::string> ();
        }
    }
    return retval;
}
