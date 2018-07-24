/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#include "Federate.hpp"

#include "../common/TomlProcessingFunctions.hpp"
#include "../common/JsonProcessingFunctions.hpp"
#include "../core/core-exceptions.hpp"
#include "../core/helicsVersion.hpp"
#include <fstream>
#include <iostream>
#include <boost/filesystem.hpp>

#include "../common/argParser.h"

namespace filesystem = boost::filesystem;

namespace helics
{
using namespace std::string_literals;
static const ArgDescriptors InfoArgs{{"broker,b"s, "address of the broker to connect"s},
                                     {"name,n"s, "name of the player federate"s},
                                     {"corename"s, "the name of the core to create or find"s},
                                     {"core,c"s, "type of the core to connect to"s},
                                     {"offset"s, "the offset of the time steps"s},
                                     {"period"s, "the period of the federate"s},
                                     {"timedelta"s, "the time delta of the federate"s},
                                     {"rttolerance"s, "the time tolerance of the real time mode"s},
                                     {"coreinit,i"s, "the core initialization string"s},
                                     {"separator"s, "separator character for local federates"s},
                                     {"inputdelay"s, "the input delay on incoming communication of the federate"s},
                                     {"outputdelay"s,
                                      "the output delay for outgoing communication of the federate"s},
                                     {"flags,f"s, ArgDescriptor::arg_type_t::vector_string,
                                      "named flag for the federate"s}};

FederateInfo::FederateInfo (int argc, const char *const *argv) { loadInfoFromArgs (argc, argv); }

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
    if (vm.count ("name") > 0)
    {
        name = vm["name"].as<std::string> ();
    }
    if (vm.count ("corename") > 0)
    {
        coreName = vm["corename"].as<std::string> ();
    }
    std::string coretypename;
    if (vm.count ("core") > 0)
    {
        coretypename = vm["core"].as<std::string> ();
    }

    coreType = helics::coreTypeFromString (coretypename);

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

    if (vm.count ("timedelta") > 0)
    {
        timeDelta = loadTimeFromString (vm["timedelta"].as<std::string> ());
    }
    if (vm.count ("inputdelay") > 0)
    {
        timeDelta = loadTimeFromString (vm["inputdelay"].as<std::string> ());
    }
    if (vm.count ("rttolerance") > 0)
    {
        rt_lead = loadTimeFromString (vm["rttolerance"].as<std::string> ());
        rt_lag = rt_lead;
    }
    if (vm.count ("rtlag") > 0)
    {
        rt_lag = loadTimeFromString (vm["rtlag"].as<std::string> ());
    }
    if (vm.count ("rtlead") > 0)
    {
        rt_lead = loadTimeFromString (vm["rtlead"].as<std::string> ());
    }
    if (vm.count ("outputdelay") > 0)
    {
        timeDelta = loadTimeFromString (vm["outputdelay"].as<std::string> ());
    }

    if (vm.count ("period") > 0)
    {
        period = loadTimeFromString (vm["period"].as<std::string> ());
    }

    if (vm.count ("offset") > 0)
    {
        offset = loadTimeFromString (vm["offset"].as<std::string> ());
    }
    if (vm.count ("maxiterations") > 0)
    {
        maxIterations = static_cast<int16_t> (vm["maxiterations"].as<int> ());
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
        for (auto flag : vflag)
        {
            if (flag == "observer")
            {
                observer = true;
            }
            else if (flag == "rollback")
            {
                rollback = true;
            }
            else if (flag == "only_update_on_change")
            {
                only_update_on_change = true;
            }
            else if (flag == "only_transmit_on_change")
            {
                only_transmit_on_change = true;
            }
            else if (flag == "source_only")
            {
                source_only = true;
            }
            else if (flag == "uninterruptible")
            {
                uninterruptible = true;
            }
            else if (flag == "interruptible")  // can use either flag
            {
                uninterruptible = false;
            }
            else if (flag == "forward_compute")
            {
                forwardCompute = true;
            }
            else if (flag == "realtime")
            {
                realtime = true;
            }
            else if (flag == "delayed_update")
            {
                wait_for_current_time_updates = true;
            }
            else
            {
                std::cerr << "unrecognized flag " << flag << std::endl;
            }
        }
    }
}

FederateInfo loadFederateInfo (const std::string &configString)
{
    return loadFederateInfo (std::string (), configString);
}

static FederateInfo loadFederateInfoJson (const std::string &name, const std::string &jsonString);
static FederateInfo loadFederateInfoToml (const std::string &name, const std::string &tomlString);

FederateInfo loadFederateInfo(const std::string &name, const std::string &configString)
{
    FederateInfo ret;
    if (hasTomlExtension (configString))
    {
        ret=loadFederateInfoToml (name,configString);
    }
    else
    {
        ret=loadFederateInfoJson (name, configString);
    }
    return ret;
}

FederateInfo loadFederateInfoJson (const std::string &name, const std::string &jsonString)
{
    FederateInfo fi (name);
    Json_helics::Value doc;
    try
    {
        doc = loadJson (jsonString);
    }
    catch (const std::invalid_argument &ia)
    {
        throw (helics::InvalidParameter (ia.what ()));
    }

    jsonReplaceIfMember (doc, "name", fi.name);

    jsonReplaceIfMember (doc, "observer", fi.observer);
    jsonReplaceIfMember (doc, "rollback", fi.rollback);
    jsonReplaceIfMember (doc, "only_update_on_change", fi.only_update_on_change);
    jsonReplaceIfMember (doc, "only_transmit_on_change", fi.only_transmit_on_change);
    jsonReplaceIfMember (doc, "source_only", fi.source_only);
    if (doc.isMember ("uninterruptible"))
    {
        fi.uninterruptible = doc["uninterruptible"].asBool ();
    }
    if (doc.isMember ("interruptible"))  // can use either flag
    {
        fi.uninterruptible = !doc["uninterruptible"].asBool ();
    }
    jsonReplaceIfMember (doc, "forward_compute", fi.forwardCompute);
    jsonReplaceIfMember (doc, "realtime", fi.realtime);
    if (doc.isMember ("rttolerance"))
    {
        fi.rt_lag = loadJsonTime (doc["rttolerance"]);
        fi.rt_lead = fi.rt_lag;
    }
   jsonReplaceIfMember (doc, "rtlead", fi.rt_lead);
    jsonReplaceIfMember (doc, "rt_lag", fi.rt_lag);
    if (doc.isMember ("separator"))
    {
        auto sep = doc["separator"].asString ();
        if (!sep.empty ())
        {
            fi.separator = sep[0];
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
   jsonReplaceIfMember (doc, "coreName", fi.coreName);
    jsonReplaceIfMember (doc, "coreInit", fi.coreInitString);
    jsonReplaceIfMember (doc, "delayed_update", fi.wait_for_current_time_updates);
    if (doc.isMember ("maxIterations"))
    {
        fi.maxIterations = static_cast<int16_t> (doc["maxIterations"].asInt ());
    }
    if (doc.isMember ("log_level"))
    {
        fi.logLevel= doc["log_level"].asInt ();
    }
    jsonReplaceIfMember (doc, "period", fi.period);

    jsonReplaceIfMember (doc, "offset", fi.offset);
    jsonReplaceIfMember (doc, "timeDelta", fi.timeDelta);
    jsonReplaceIfMember (doc, "outputDelay", fi.outputDelay);
    jsonReplaceIfMember (doc, "inputDelay", fi.inputDelay);
    return fi;
}

FederateInfo loadFederateInfoToml (const std::string &name, const std::string &tomlString)
{
    FederateInfo fi (name);
    toml::Value doc;
    try
    {
        doc = loadToml (tomlString);
    }
    catch (const std::invalid_argument &ia)
    {
        throw (helics::InvalidParameter (ia.what ()));
    }
    tomlReplaceIfMember (doc, "name", fi.name);
    tomlReplaceIfMember (doc, "observer", fi.observer);
    tomlReplaceIfMember (doc, "rollback", fi.rollback);
    tomlReplaceIfMember (doc, "only_update_on_change", fi.only_update_on_change);
    tomlReplaceIfMember (doc, "only_transmit_on_change", fi.only_transmit_on_change);
    tomlReplaceIfMember (doc, "source_only", fi.source_only);
    tomlReplaceIfMember (doc, "uninterruptible", fi.uninterruptible);
   
	auto inter = doc.find ("interruptible");
    if (inter!=nullptr)  // can use either flag
    {
        fi.uninterruptible = !(inter->as<bool>());
    }
    tomlReplaceIfMember (doc, "forward_compute", fi.forwardCompute);
    tomlReplaceIfMember (doc, "realtime", fi.realtime);
    if (isMember(doc,"rttolerance"))
    {
        fi.rt_lag = loadTomlTime (doc["rttolerance"]);
        fi.rt_lead = fi.rt_lag;
    }
    tomlReplaceIfMember (doc, "rtlead", fi.rt_lead);
    tomlReplaceIfMember (doc, "rt_lag", fi.rt_lag);

    if (isMember (doc, "separator"))
    {
        auto sep = doc["separator"].as<std::string> ();
        if (!sep.empty ())
        {
            fi.separator = sep[0];
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
    tomlReplaceIfMember (doc, "coreName", fi.coreName);
    tomlReplaceIfMember (doc, "coreInit", fi.coreInitString);
    tomlReplaceIfMember (doc, "delayed_update", fi.wait_for_current_time_updates);
    fi.maxIterations=static_cast<int16_t>(tomlGetOrDefault<int32_t> (doc, "maxIterations", fi.maxIterations));
    tomlReplaceIfMember (doc, "log_level", fi.logLevel);
    tomlReplaceIfMember (doc, "period", fi.period);
   
	tomlReplaceIfMember (doc, "offset", fi.offset);
    tomlReplaceIfMember (doc, "timeDelta", fi.timeDelta);
    tomlReplaceIfMember (doc, "outputDelay", fi.outputDelay);
    tomlReplaceIfMember (doc, "inputDelay", fi.inputDelay);
    
    return fi;
}
}  // namespace helics
