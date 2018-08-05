/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#include "Federate.hpp"

#include "../common/JsonProcessingFunctions.hpp"
#include "../common/TomlProcessingFunctions.hpp"
#include "../core/core-exceptions.hpp"
#include "../core/helicsVersion.hpp"
#include <fstream>
#include <iostream>
#include <set>
#include <boost/filesystem.hpp>

#include "../common/argParser.h"
#include "../common/stringOps.h"

namespace filesystem = boost::filesystem;

namespace helics
{
using namespace std::string_literals;
static const ArgDescriptors InfoArgs{
  {"broker,b"s, "address of the broker to connect"s},
  {"name,n"s, "name of the federate"s},
  {"corename"s, "the name of the core to create or find"s},
  {"core,c"s, "type of the core to connect to"s},
  {"offset"s, "the offset of the time steps"s},
  {"period"s, "the period of the federate"s},
  {"timedelta"s, "the time delta of the federate"s},
  {"rttolerance"s, "the time tolerance of the real time mode"s},
  {"rt_lag"s, "the amount of the time the federate is allowed to lag realtime before corrective action is taken"s},
  {"rt_lead"s,
   "the amount of the time the federate is allowed to lead realtime before corrective action is taken"s},
  {"coreinit,i"s, "the core initialization string"s},
  {"maxiterations"s, ArgDescriptor::arg_type_t::int_type,
   "the maximum number of iterations a federate is allowed to take"s},
  {"log_level"s, ArgDescriptor::arg_type_t::int_type, "the logging level of a federate"s},
  {"separator"s, "separator character for local federates"s},
  {"inputdelay"s, "the input delay on incoming communication of the federate"s},
  {"outputdelay"s, "the output delay for outgoing communication of the federate"s},
  {"flags,f"s, ArgDescriptor::arg_type_t::vector_string, "named flag for the federate"s}};

FederateInfo::FederateInfo (int argc, const char *const *argv) { loadInfoFromArgs (argc, argv); }

static const std::map<std::string, int> propStringsTranslations{
  {"period", PERIOD_PROPERTY},
  {"timedelta", TIME_DELTA_PROPERTY},
  {"time_delta", TIME_DELTA_PROPERTY},
  {"offset", OFFSET_PROPERTY},
  {"rtlead", RT_LEAD_PROPERTY},
  {"rtlag", RT_LEAD_PROPERTY},
  {"rttolerance", RT_TOLERANCE_PROPERTY},
  {"rt_lead", RT_LEAD_PROPERTY},
  {"rt_lag", RT_LEAD_PROPERTY},
  {"rt_tolerance", RT_TOLERANCE_PROPERTY},
  {"inputdelay", INPUT_DELAY_PROPERTY},
  {"outputdelay", RT_LEAD_PROPERTY},
  {"input_delay", INPUT_DELAY_PROPERTY},
  {"output_delay", RT_LEAD_PROPERTY},
  {"max_iterations", MAX_ITERATIONS_PROPERTY},
  {"loglevel", LOG_LEVEL_PROPERTY},
  {"log_level", LOG_LEVEL_PROPERTY},
  {"maxiterations", MAX_ITERATIONS_PROPERTY},
  {"interruptible", INTERRUPTIBLE_FLAG},
  {"uninterruptible", UNINTERRUPTIBLE_FLAG},
  {"observer", OBSERVER_FLAG},
  {"source_only", SOURCE_ONLY_FLAG},
  {"sourceonly", SOURCE_ONLY_FLAG},
  {"only_update_on_change", ONLY_UPDATE_ON_CHANGE_FLAG},
  {"only_transmit_on_change", ONLY_TRANSMIT_ON_CHANGE_FLAG},
  {"forward_compute", FORWARD_COMPUTE_FLAG},
  {"realtime", REALTIME_FLAG},
  {"delayed_update", WAIT_FOR_CURRENT_TIME_UPDATE_FLAG},
  {"wait_for_current_time", WAIT_FOR_CURRENT_TIME_UPDATE_FLAG}};

static const std::set<std::string> validTimeProperties{"period",      "timedelta",    "time_delta",  "offset",
                                                       "rtlead",      "rtlag",        "rttolerance", "rt_lead",
                                                       "rt_lag",      "rt_tolerance", "inputdelay",  "outputdelay",
                                                       "input_delay", "output_delay"};

static const std::set<std::string> validIntProperties{"max_iterations", "loglevel", "log_level", "maxiterations"};

static const std::set<std::string> validFlagOptions{
  "interruptible",         "uninterruptible",         "observer",        "source_only", "sourceonly",
  "only_update_on_change", "only_transmit_on_change", "forward_compute", "realtime",    "delayed_update",
  "wait_for_current_time"};

static void loadFlags(FederateInfo &fi, const std::string &flags)
{
    auto sflgs = stringOps::splitline (flags);
    for (auto &flg : sflgs)
    {
        auto loc = validFlagOptions.find (flg);
        if (loc != validFlagOptions.end ())
        {
            fi.setFlagOption (propStringsTranslations.at (flg), true);
        }
        else
        {
            try
            {
                auto val = std::stoi (flg);
                fi.setFlagOption (val, (val > 0));
            }
            catch (const std::invalid_argument &)
            {
                std::cerr << "unrecognized flag " << flg << std::endl;
            }
        }
    }
}

	void FederateInfo::loadInfoFromArgs (int argc, const char *const *argv)
{
    variable_map vm;
    auto res = argumentParser (argc, argv, vm, InfoArgs);
    if (res == versionReturn)
    {
        std::cout << helics::versionString << '\n';
    }
    if (res < 0)
    {
        return;
    }
	if (vm.count("name") > 0)
	{
        defName = vm["name"].as<std::string> ();
	}
    if (vm.count ("corename") > 0)
    {
        coreName = vm["corename"].as<std::string> ();
    }
    if (vm.count ("core") > 0)
    {
        coreType = helics::coreTypeFromString (vm["core"].as<std::string> ());
    }

    coreInitString = "1";
    if (vm.count ("coreinit") > 0)
    {
        coreInitString.push_back (' ');
        coreInitString = vm["coreinit"].as<std::string> ();
    }
    if (vm.count ("broker") > 0)
    {
        coreInitString += " --broker=";
        coreInitString += vm["broker"].as<std::string> ();
    }
    for (auto &tprop : validTimeProperties)
    {
        if (vm.count (tprop) > 0)
        {
            setTimeProperty (propStringsTranslations.at (tprop),
                             loadTimeFromString (vm[tprop].as<std::string> ()));
        }
    }
    for (auto &iprop : validIntProperties)
    {
        if (vm.count (iprop) > 0)
        {
            setTimeProperty (propStringsTranslations.at (iprop), vm[iprop].as<int> ());
        }
    }

    if (vm.count ("separator") > 0)
    {
        auto sep = vm["separator"].as<std::string> ();
        if (!sep.empty ())
        {
            separator = sep[0];
        }
    }
    if (vm.count ("flags") > 0)
    {
        auto vflag = vm["flags"].as<std::vector<std::string>> ();
        for (auto &flag : vflag)
        {
            loadFlags (*this, flag);
        }
    }
}

FederateInfo loadFederateInfo (const std::string &configString)
{
    return loadFederateInfo (std::string (), configString);
}

static FederateInfo loadFederateInfoJson (const std::string &name, const std::string &jsonString);
static FederateInfo loadFederateInfoToml (const std::string &name, const std::string &tomlString);

FederateInfo loadFederateInfo (const std::string &name, const std::string &configString)
{
    FederateInfo ret;
    if (hasTomlExtension (configString))
    {
        ret = loadFederateInfoToml (name, configString);
    }
    else
    {
        ret = loadFederateInfoJson (name, configString);
    }
    return ret;
}

FederateInfo loadFederateInfoJson (const std::string &name, const std::string &jsonString)
{
    FederateInfo fi;
    Json_helics::Value doc;
    try
    {
        doc = loadJson (jsonString);
    }
    catch (const std::invalid_argument &ia)
    {
        throw (helics::InvalidParameter (ia.what ()));
    }

    std::function<void(const std::string &, bool)> flagCall = [&fi](const std::string &fname, bool arg) {
        fi.setFlagOption (propStringsTranslations.at (fname), arg);
    };

    std::function<void(const std::string &, Time)> timeCall = [&fi](const std::string &fname, Time arg) {
        fi.setTimeProperty (propStringsTranslations.at (fname), arg);
    };

    std::function<void(const std::string &, int)> intCall = [&fi](const std::string &fname, int arg) {
        fi.setIntegerProperty (propStringsTranslations.at (fname), arg);
    };

	for (auto &prop : validTimeProperties)
	{
        jsonCallIfMember (doc, prop, timeCall);
	}

	for (auto &prop : validIntProperties)
    {
        jsonCallIfMember (doc, prop, intCall);
    }

	for (auto &prop : validFlagOptions)
    {
        jsonCallIfMember (doc, prop, flagCall);
    }
    if (doc.isMember ("flags"))
    {
        loadFlags (fi, doc["flags"].asString());
	}
    
    if (doc.isMember ("separator"))
    {
        auto sep = doc["separator"].asString ();
        if (!sep.empty ())
        {
            fi.separator = sep[0];
        }
    }
    if (doc.isMember ("core"))
    {
        try
        {
            fi.coreType = coreTypeFromString (doc["core"].asString ());
        }
        catch (const std::invalid_argument &ia)
        {
            fi.coreName = doc["core"].asString ();
        }
    }
    if (doc.isMember ("coreType"))
    {
        try
        {
            fi.coreType = coreTypeFromString (doc["coreType"].asString ());
        }
        catch (const std::invalid_argument &ia)
        {
            std::cerr << "Unrecognized core type\n";
        }
    }
    if (doc.isMember ("coretype"))
    {
        try
        {
            fi.coreType = coreTypeFromString (doc["coretype"].asString ());
        }
        catch (const std::invalid_argument &ia)
        {
            std::cerr << "Unrecognized core type\n";
        }
    }
    jsonReplaceIfMember (doc, "name", fi.defName);
    jsonReplaceIfMember (doc, "coreName", fi.coreName);
    jsonReplaceIfMember (doc, "coreInit", fi.coreInitString);
    return fi;
}

FederateInfo loadFederateInfoToml (const std::string &name, const std::string &tomlString)
{
    FederateInfo fi;
    toml::Value doc;
    try
    {
        doc = loadToml (tomlString);
    }
    catch (const std::invalid_argument &ia)
    {
        throw (helics::InvalidParameter (ia.what ()));
    }
    std::function<void(const std::string &, bool)> flagCall = [&fi](const std::string &fname, bool arg) {
        fi.setFlagOption (propStringsTranslations.at (fname), arg);
    };

    std::function<void(const std::string &, Time)> timeCall = [&fi](const std::string &fname, Time arg) {
        fi.setTimeProperty (propStringsTranslations.at (fname), arg);
    };

    std::function<void(const std::string &, int)> intCall = [&fi](const std::string &fname, int arg) {
        fi.setIntegerProperty (propStringsTranslations.at (fname), arg);
    };

    for (auto &prop : validTimeProperties)
    {
        tomlCallIfMember (doc, prop, timeCall);
    }

    for (auto &prop : validIntProperties)
    {
        tomlCallIfMember (doc, prop, intCall);
    }

    for (auto &prop : validFlagOptions)
    {
        tomlCallIfMember (doc, prop, flagCall);
    }
    if (isMember (doc, "flags"))
    {
        loadFlags (fi, doc["flags"].as<std::string> ());
    }
    if (isMember (doc, "separator"))
    {
        auto sep = doc["separator"].as<std::string> ();
        if (!sep.empty ())
        {
            fi.separator = sep[0];
        }
    }
    if (isMember (doc, "core"))
    {
        try
        {
            fi.coreType = coreTypeFromString (doc["core"].as<std::string> ());
        }
        catch (const std::invalid_argument &ia)
        {
            fi.coreName = doc["core"].as<std::string> ();
        }
    }
    if (isMember (doc, "coreType"))
    {
        try
        {
            fi.coreType = coreTypeFromString (doc["coreType"].as<std::string> ());
        }
        catch (const std::invalid_argument &ia)
        {
            std::cerr << "Unrecognized core type\n";
        }
    }
    else if (isMember (doc, "coretype"))
    {
        try
        {
            fi.coreType = coreTypeFromString (doc["coretype"].as<std::string> ());
        }
        catch (const std::invalid_argument &ia)
        {
            std::cerr << "Unrecognized core type\n";
        }
    }
    tomlReplaceIfMember (doc, "name", fi.defName);
    tomlReplaceIfMember (doc, "coreName", fi.coreName);
    tomlReplaceIfMember (doc, "coreInit", fi.coreInitString);
   

    return fi;
}
}  // namespace helics
