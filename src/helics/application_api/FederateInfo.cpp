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
  {"broker,b"s, "address or name of the broker to connect"s},
  {"name,n"s, "name of the federate"s},
  {"corename"s, "the name of the core to create or find"s},
  {"coretype,t"s, "type of the core to connect to"s},
  {"offset"s, "the offset of the time steps"s},
  {"period"s, "the period of the federate"s},
  {"timedelta"s, "the time delta of the federate"s},
  {"rttolerance"s, "the time tolerance of the real time mode"s},
  {"rt_lag"s, "the amount of the time the federate is allowed to lag realtime before corrective action is taken"s},
  {"rt_lead"s,
   "the amount of the time the federate is allowed to lead realtime before corrective action is taken"s},
  {"coreinit,i"s, "the core initialization string"s},
  {"coreinitstring"s, "the core initialization string"s},
  {"maxiterations"s, ArgDescriptor::arg_type_t::int_type,
   "the maximum number of iterations a federate is allowed to take"s},
  {"log_level"s, ArgDescriptor::arg_type_t::int_type, "the logging level of a federate"s},
  {"separator"s, "separator character for local federates"s},
  {"inputdelay"s, "the input delay on incoming communication of the federate"s},
  {"outputdelay"s, "the output delay for outgoing communication of the federate"s},
  {"brokerport"s, ArgDescriptor::arg_type_t::int_type, "port number for the broker priority port"s},
  {"localport"s, "port number for the local receive port"s},
  {"autobroker"s, ArgDescriptor::arg_type_t::flag_type,
   "tell the core to automatically generate a broker if needed"s},
  {"port"s, ArgDescriptor::arg_type_t::int_type, "port number for the broker's port"s},
  {"flags,f"s, ArgDescriptor::arg_type_t::vector_string, "named flag for the federate"s}};

FederateInfo::FederateInfo (int argc, const char *const *argv) { loadInfoFromArgs (argc, argv); }

static const std::map<std::string, int> propStringsTranslations{
  {"period", helics_property_time_period},
  {"timedelta", helics_property_time_delta},
  {"time_delta", helics_property_time_delta},
  {"delta", helics_property_time_delta},
  {"offset", helics_property_time_offset},
  {"rtlead", helics_property_time_rt_lead},
  {"rtlag", helics_property_time_rt_lag},
  {"rttolerance", helics_property_time_rt_tolerance},
  {"rt_lead", helics_property_time_rt_lead},
  {"rt_lag", helics_property_time_rt_lag},
  {"rt_tolerance", helics_property_time_rt_tolerance},
  {"inputdelay", helics_property_time_input_delay},
  {"outputdelay", helics_property_time_output_delay},
  {"input_delay", helics_property_time_input_delay},
  {"output_delay", helics_property_time_output_delay},
  {"max_iterations", helics_property_int_max_iterations},
  {"loglevel", helics_property_int_log_level},
  {"log_level", helics_property_int_log_level},
  {"maxiterations", helics_property_int_max_iterations},
  {"iterations", helics_property_int_max_iterations},
  {"interruptible", helics_flag_interruptible},
  {"uninterruptible", helics_flag_uninterruptible},
  {"observer", helics_flag_observer},
  {"source_only", helics_flag_source_only},
  {"sourceonly", helics_flag_source_only},
  {"source", helics_flag_source_only},
  {"only_update_on_change", helics_flag_only_update_on_change},
  {"only_transmit_on_change", helics_flag_only_transmit_on_change},
  {"forward_compute", helics_flag_forward_compute},
  {"realtime", helics_flag_realtime},
  {"ignore_time_mismatch", helics_flag_ignore_time_mismatch_warnings},
  {"delayed_update", helics_flag_wait_for_current_time_update},
  {"strict_input_type_checking", helics_handle_option_strict_type_checking},
  {"buffer_data", helics_handle_option_buffer_data},
  {"required", helics_handle_option_connection_required},
  {"optional", helics_handle_option_connection_optional},
  {"wait_for_current_time", helics_flag_wait_for_current_time_update}};

static const std::set<std::string> validTimeProperties{"period",      "timedelta",    "time_delta",  "offset",
                                                       "rtlead",      "rtlag",        "rttolerance", "rt_lead",
                                                       "rt_lag",      "rt_tolerance", "inputdelay",  "outputdelay",
                                                       "input_delay", "output_delay"};

static const std::set<std::string> validIntProperties{"max_iterations", "loglevel", "log_level", "maxiterations"};

static const std::set<std::string> validFlagOptions{"interruptible",
                                                    "uninterruptible",
                                                    "observer",
                                                    "source_only",
                                                    "sourceonly",
                                                    "only_update_on_change",
                                                    "only_transmit_on_change",
                                                    "forward_compute",
                                                    "realtime",
                                                    "delayed_update",
                                                    "wait_for_current_time",
                                                    "strict_input_type_checking",
                                                    "buffer_data",
                                                    "required",
                                                    "optional"};

static const std::map<std::string, int> optionStringsTranslations{
  {"buffer", helics_handle_option_buffer_data},
  {"buffer_data", helics_handle_option_buffer_data},
  {"bufferdata", helics_handle_option_buffer_data},
  {"optional", helics_handle_option_connection_optional},
  {"connectionoptional", helics_handle_option_connection_optional},
  {"connection_optional", helics_handle_option_connection_optional},
  {"required", helics_handle_option_connection_required},
  {"connectionrequired", helics_handle_option_connection_required},
  {"connection_required", helics_handle_option_connection_required},
  {"ignore_interrupts", helics_handle_option_ignore_interrupts},
  {"ignoreinterrupts", helics_handle_option_ignore_interrupts},
  {"nointerrupts", helics_handle_option_ignore_interrupts},
  {"no_interrupts", helics_handle_option_ignore_interrupts},
  {"uninterruptible", helics_handle_option_ignore_interrupts},
  {"multiple", helics_handle_option_multiple_connections_allowed},
  {"multiple_connections", helics_handle_option_multiple_connections_allowed},
  {"multiple_connections_allowed", helics_handle_option_multiple_connections_allowed},
  {"multipleconnections", helics_handle_option_multiple_connections_allowed},
  {"multipleconnectionsallowed", helics_handle_option_multiple_connections_allowed},
  {"single", helics_handle_option_single_connection_only},
  {"single_connection", helics_handle_option_single_connection_only},
  {"single_connection_only", helics_handle_option_single_connection_only},
  {"singleconnection", helics_handle_option_single_connection_only},
  {"singleconnectionsonly", helics_handle_option_single_connection_only},
  {"only_transmit_on_change", helics_handle_option_only_transmit_on_change},
  {"onlytransmitonchange", helics_handle_option_only_transmit_on_change},
  {"only_update_on_change", helics_handle_option_only_update_on_change},
  {"onlyupdateonchange", helics_handle_option_only_update_on_change},
  {"strict_type_checking", helics_handle_option_strict_type_checking},
  {"strict_type_matching", helics_handle_option_strict_type_checking},
  {"strict_input_type_checking", helics_handle_option_strict_type_checking},
  {"strict_input_type_matching", helics_handle_option_strict_type_checking},
  {"strictinputtypechecking", helics_handle_option_strict_type_checking},
  {"strictinputtypematching", helics_handle_option_strict_type_checking},
  {"stricttypechecking", helics_handle_option_strict_type_checking},
  {"stricttypematching", helics_handle_option_strict_type_checking},
  {"strict", helics_handle_option_strict_type_checking}};

static void loadFlags (FederateInfo &fi, const std::string &flags)
{
    auto sflgs = stringOps::splitline (flags);
    for (auto &flg : sflgs)
    {
        if (flg == "autobroker")
        {
            fi.autobroker = true;
            continue;
        }
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

int getPropertyIndex (std::string val)
{
    auto fnd = propStringsTranslations.find (val);
    if (fnd != propStringsTranslations.end ())
    {
        return fnd->second;
    }
    makeLowerCase (val);
    fnd = propStringsTranslations.find (val);
    if (fnd != propStringsTranslations.end ())
    {
        return fnd->second;
    }
    val.erase (std::remove (val.begin (), val.end (), '_'), val.end ());
    fnd = propStringsTranslations.find (val);
    if (fnd != propStringsTranslations.end ())
    {
        return fnd->second;
    }
    return -1;
}

int getOptionIndex (std::string val)
{
    auto fnd = optionStringsTranslations.find (val);
    if (fnd != optionStringsTranslations.end ())
    {
        return fnd->second;
    }
    makeLowerCase (val);
    fnd = optionStringsTranslations.find (val);
    if (fnd != optionStringsTranslations.end ())
    {
        return fnd->second;
    }
    val.erase (std::remove (val.begin (), val.end (), '_'), val.end ());
    fnd = optionStringsTranslations.find (val);
    if (fnd != optionStringsTranslations.end ())
    {
        return fnd->second;
    }
    return -1;
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
    if (vm.count ("name") > 0)
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
    if (vm.count ("type") > 0)
    {
        coreType = helics::coreTypeFromString (vm["type"].as<std::string> ());
    }
    if (vm.count ("coretype") > 0)
    {
        coreType = helics::coreTypeFromString (vm["coretype"].as<std::string> ());
    }

    coreInitString = "";
    if (vm.count ("coreinit") > 0)
    {
        coreInitString.push_back (' ');
        coreInitString = vm["coreinit"].as<std::string> ();
    }
    if (vm.count ("coreinitstring") > 0)
    {
        coreInitString.push_back (' ');
        coreInitString = vm["coreinitstring"].as<std::string> ();
    }
    if (vm.count ("broker") > 0)
    {
        broker = vm["broker"].as<std::string> ();
    }
    if (vm.count ("broker_address") > 0)
    {
        broker = vm["broker_address"].as<std::string> ();
    }
    if (vm.count ("port") > 0)
    {  // there is some ambiguity of what port could mean this is dealt with later
        brokerPort = vm["port"].as<int> ();
    }
    if (vm.count ("brokerport") > 0)
    {
        if (brokerPort >= 0)
        {
            localport = std::to_string (brokerPort);
        }
        else
        {
            brokerPort = vm["brokerport"].as<int> ();
        }
    }
    if (vm.count ("localport") > 0)
    {
        localport = vm["localport"].as<std::string> ();
    }
    for (auto &tprop : validTimeProperties)
    {
        if (vm.count (tprop) > 0)
        {
            setProperty (propStringsTranslations.at (tprop), loadTimeFromString (vm[tprop].as<std::string> ()));
        }
    }
    for (auto &iprop : validIntProperties)
    {
        if (vm.count (iprop) > 0)
        {
            setProperty (propStringsTranslations.at (iprop), vm[iprop].as<int> ());
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

    if (vm.count ("autobroker") > 0)
    {
        autobroker = true;
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

static FederateInfo loadFederateInfoJson (const std::string &jsonString);
static FederateInfo loadFederateInfoToml (const std::string &tomlString);

FederateInfo loadFederateInfo (const std::string &configString)
{
    FederateInfo ret;
    if (hasTomlExtension (configString))
    {
        ret = loadFederateInfoToml (configString);
    }
    else if ((hasJsonExtension (configString)) || (configString.find_first_of ('{') != std::string::npos))
    {
        ret = loadFederateInfoJson (configString);
    }
    else
    {
        ret.defName = configString;
    }
    return ret;
}

FederateInfo loadFederateInfoJson (const std::string &jsonString)
{
    FederateInfo fi;
    Json::Value doc;
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
        fi.setProperty (propStringsTranslations.at (fname), arg);
    };

    std::function<void(const std::string &, int)> intCall = [&fi](const std::string &fname, int arg) {
        fi.setProperty (propStringsTranslations.at (fname), arg);
    };

    for (auto &prop : validTimeProperties)
    {
        callIfMember (doc, prop, timeCall);
    }

    for (auto &prop : validIntProperties)
    {
        callIfMember (doc, prop, intCall);
    }

    for (auto &prop : validFlagOptions)
    {
        callIfMember (doc, prop, flagCall);
    }
    if (doc.isMember ("flags"))
    {
        loadFlags (fi, doc["flags"].asString ());
    }

    replaceIfMember (doc, "broker", fi.broker);
    fi.brokerPort = getOrDefault (doc, "brokerport", int64_t (fi.brokerPort));
    replaceIfMember (doc, "localport", fi.localport);
    replaceIfMember (doc, "autobroker", fi.autobroker);
    if (doc.isMember ("port"))
    {
        if (fi.localport.empty ())
        {
            if (fi.brokerPort < 0)
            {
                fi.brokerPort = doc["port"].asInt ();
            }
            else
            {
                fi.localport = doc["port"].asString ();
            }
        }
        else
        {
            if (fi.brokerPort < 0)
            {
                fi.brokerPort = doc["port"].asInt ();
            }
        }
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
    else if (doc.isMember ("coretype"))
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
    else if (doc.isMember ("type"))
    {
        try
        {
            fi.coreType = coreTypeFromString (doc["type"].asString ());
        }
        catch (const std::invalid_argument &ia)
        {
            std::cerr << "Unrecognized core type\n";
        }
    }
    replaceIfMember (doc, "name", fi.defName);
    replaceIfMember (doc, "coreName", fi.coreName);
    replaceIfMember (doc, "corename", fi.coreName);
    replaceIfMember (doc, "coreInit", fi.coreInitString);
    replaceIfMember (doc, "coreinit", fi.coreInitString);
    replaceIfMember (doc, "coreinitstring", fi.coreInitString);
    return fi;
}

FederateInfo loadFederateInfoToml (const std::string &tomlString)
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
        fi.setProperty (propStringsTranslations.at (fname), arg);
    };

    std::function<void(const std::string &, int)> intCall = [&fi](const std::string &fname, int arg) {
        fi.setProperty (propStringsTranslations.at (fname), arg);
    };

    for (auto &prop : validTimeProperties)
    {
        callIfMember (doc, prop, timeCall);
    }

    for (auto &prop : validIntProperties)
    {
        callIfMember (doc, prop, intCall);
    }

    for (auto &prop : validFlagOptions)
    {
        callIfMember (doc, prop, flagCall);
    }
    if (isMember (doc, "flags"))
    {
        loadFlags (fi, doc["flags"].as<std::string> ());
    }
    replaceIfMember (doc, "autobroker", fi.autobroker);
    replaceIfMember (doc, "broker", fi.broker);
    fi.brokerPort = getOrDefault (doc, "brokerport", fi.brokerPort);
    replaceIfMember (doc, "localport", fi.localport);
    if (isMember (doc, "port"))
    {
        if (fi.localport.empty ())
        {
            if (fi.brokerPort < 0)
            {
                fi.brokerPort = doc["port"].as<int> ();
            }
            else
            {
                fi.localport = doc["port"].as<std::string> ();
            }
        }
        else
        {
            if (fi.brokerPort < 0)
            {
                fi.brokerPort = doc["port"].as<int> ();
            }
        }
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
    else if (isMember (doc, "type"))
    {
        try
        {
            fi.coreType = coreTypeFromString (doc["type"].as<std::string> ());
        }
        catch (const std::invalid_argument &ia)
        {
            std::cerr << "Unrecognized core type\n";
        }
    }
    replaceIfMember (doc, "name", fi.defName);
    replaceIfMember (doc, "coreName", fi.coreName);
    replaceIfMember (doc, "corename", fi.coreName);
    replaceIfMember (doc, "coreInit", fi.coreInitString);
    replaceIfMember (doc, "coreinit", fi.coreInitString);
    replaceIfMember (doc, "coreinitstring", fi.coreInitString);

    return fi;
}

std::string generateFullCoreInitString (const FederateInfo &fi)
{
    auto res = fi.coreInitString;
    if (!fi.broker.empty ())
    {
        res += " --broker=";
        res.append (fi.broker);
    }
    if (fi.brokerPort >= 0)
    {
        res += " --brokerport=";
        res.append (std::to_string (fi.brokerPort));
    }
    if (!fi.localport.empty ())
    {
        res += " --localport=";
        res.append (fi.localport);
    }
    if (fi.autobroker)
    {
        res.append (" --autobroker");
    }
    return res;
}

}  // namespace helics
