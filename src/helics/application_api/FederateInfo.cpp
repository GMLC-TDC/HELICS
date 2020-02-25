/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "FederateInfo.hpp"

#include "../common/JsonProcessingFunctions.hpp"
#include "../common/TomlProcessingFunctions.hpp"
#include "../core/core-exceptions.hpp"
#include "../core/helicsCLI11.hpp"
#include "../core/helicsVersion.hpp"
#include "gmlc/utilities/stringOps.h"

#include <iostream>
#include <set>

namespace helics {
FederateInfo::FederateInfo(int argc, char* argv[])
{
    loadInfoFromArgsIgnoreOutput(argc, argv);
}

FederateInfo::FederateInfo(std::vector<std::string>& args)
{
    loadInfoFromArgs(args);
}

FederateInfo::FederateInfo(const std::string& args)
{
    loadInfoFromArgsIgnoreOutput(args);
}

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
    {"slow", helics_flag_slow_responding},
    {"slow_response", helics_flag_slow_responding},
    {"slow_responding", helics_flag_slow_responding},
    {"no_ping", helics_flag_slow_responding},
    {"disable_ping", helics_flag_slow_responding},
    {"only_update_on_change", helics_flag_only_update_on_change},
    {"only_transmit_on_change", helics_flag_only_transmit_on_change},
    {"forward_compute", helics_flag_forward_compute},
    {"realtime", helics_flag_realtime},
    {"restrictive_time_policy", helics_flag_restrictive_time_policy},
    {"conservative_time_policy", helics_flag_restrictive_time_policy},
    {"restrictive_time", helics_flag_restrictive_time_policy},
    {"conservative_time", helics_flag_restrictive_time_policy},
    {"ignore_time_mismatch", helics_flag_ignore_time_mismatch_warnings},
    {"delayed_update", helics_flag_wait_for_current_time_update},
    {"strict_input_type_checking", helics_handle_option_strict_type_checking},
    {"ignore_unit_mismatch", helics_handle_option_ignore_unit_mismatch},
    {"buffer_data", helics_handle_option_buffer_data},
    {"required", helics_handle_option_connection_required},
    {"optional", helics_handle_option_connection_optional},
    {"wait_for_current_time", helics_flag_wait_for_current_time_update},
    {"terminate_on_error",helics_flag_terminate_on_error},
    {"terminateonerror", helics_flag_terminate_on_error}};

static const std::set<std::string> validTimeProperties{"period",
                                                       "timedelta",
                                                       "time_delta",
                                                       "offset",
                                                       "rtlead",
                                                       "rtlag",
                                                       "rttolerance",
                                                       "rt_lead",
                                                       "rt_lag",
                                                       "rt_tolerance",
                                                       "inputdelay",
                                                       "outputdelay",
                                                       "input_delay",
                                                       "output_delay"};

static const std::set<std::string> validIntProperties{"max_iterations",
                                                      "loglevel",
                                                      "log_level",
                                                      "maxiterations"};

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
                                                    "ignore_unit_mismatch",
                                                    "restrictive_time_policy",
                                                    "conservative_time_policy",
                                                    "restrictive_time",
                                                    "conservative_time",
                                                    "buffer_data",
                                                    "slow_response",
                                                    "slow_responding",
                                                    "disable_ping"
                                                    "no_ping",
                                                    "slow",
                                                    "required",
                                                    "optional",
"terminate_on_error",
"terminateonerror" };

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
    {"ignore_unit_mismatch", helics_handle_option_ignore_unit_mismatch},
    {"ignore_units", helics_handle_option_ignore_unit_mismatch},
    {"strict_type_checking", helics_handle_option_strict_type_checking},
    {"strict_type_matching", helics_handle_option_strict_type_checking},
    {"strict_input_type_checking", helics_handle_option_strict_type_checking},
    {"strict_input_type_matching", helics_handle_option_strict_type_checking},
    {"strictinputtypechecking", helics_handle_option_strict_type_checking},
    {"strictinputtypematching", helics_handle_option_strict_type_checking},
    {"stricttypechecking", helics_handle_option_strict_type_checking},
    {"stricttypematching", helics_handle_option_strict_type_checking},
    {"strict", helics_handle_option_strict_type_checking}};

static const std::map<std::string, int> log_level_map{{"none", helics_log_level_no_print},
                                                      {"no_print", helics_log_level_no_print},
                                                      {"error", helics_log_level_error},
                                                      {"warning", helics_log_level_warning},
                                                      {"summary", helics_log_level_summary},
                                                      {"connections", helics_log_level_connections},
                                                      /** connections+ interface definitions*/
                                                      {"interfaces", helics_log_level_interfaces},
                                                      /** interfaces + timing message*/
                                                      {"timing", helics_log_level_timing},
                                                      /** timing+ data transfer notices*/
                                                      {"data", helics_log_level_data},
                                                      /** same as data*/
                                                      {"debug", helics_log_level_data},
                                                      /** all internal messages*/
                                                      {"trace", helics_log_level_trace}};

static void loadFlags(FederateInfo& fi, const std::string& flags)
{
    auto sflgs = gmlc::utilities::stringOps::splitline(flags);
    for (auto& flg : sflgs) {
        if (flg == "autobroker") {
            fi.autobroker = true;
            continue;
        }
        auto loc = validFlagOptions.find(flg);
        if (loc != validFlagOptions.end()) {
            fi.setFlagOption(propStringsTranslations.at(flg), true);
        } else {
            try {
                auto val = std::stoi(flg);
                fi.setFlagOption(val, (val > 0));
            }
            catch (const std::invalid_argument&) {
                std::cerr << "unrecognized flag " << flg << std::endl;
            }
        }
    }
}

int getPropertyIndex(std::string val)
{
    auto fnd = propStringsTranslations.find(val);
    if (fnd != propStringsTranslations.end()) {
        return fnd->second;
    }
    gmlc::utilities::makeLowerCase(val);
    fnd = propStringsTranslations.find(val);
    if (fnd != propStringsTranslations.end()) {
        return fnd->second;
    }
    val.erase(std::remove(val.begin(), val.end(), '_'), val.end());
    fnd = propStringsTranslations.find(val);
    if (fnd != propStringsTranslations.end()) {
        return fnd->second;
    }
    return -1;
}

int getOptionIndex(std::string val)
{
    auto fnd = optionStringsTranslations.find(val);
    if (fnd != optionStringsTranslations.end()) {
        return fnd->second;
    }
    gmlc::utilities::makeLowerCase(val);
    fnd = optionStringsTranslations.find(val);
    if (fnd != optionStringsTranslations.end()) {
        return fnd->second;
    }
    val.erase(std::remove(val.begin(), val.end(), '_'), val.end());
    fnd = optionStringsTranslations.find(val);
    if (fnd != optionStringsTranslations.end()) {
        return fnd->second;
    }
    return -1;
}

using namespace std::string_literals;

std::unique_ptr<helicsCLI11App> FederateInfo::makeCLIApp()
{
    auto app = std::make_unique<helicsCLI11App>("Federate Info Parsing");

    app->add_option("--name,-n", defName, "name of the federate");
    app->add_option("--corename", coreName, "the name of the core to create or find")
        ->ignore_underscore();
    app->addTypeOption();
    app->add_option(
           "--coreinitstring,-i,--coreinit",
           coreInitString,
           "The initialization arguments for the core")
        ->transform([](std::string arg) {
            arg.insert(arg.begin(), ' ');
            return arg;
        })
        ->ignore_underscore()
        ->multi_option_policy(CLI::MultiOptionPolicy::Join);
    app->add_option(
           "--brokerinitstring,--brokerinit",
           brokerInitString,
           "The initialization arguments for the broker if autogenerated")
        ->transform([](std::string arg) {
            arg.insert(arg.begin(), ' ');
            return arg;
        })
        ->ignore_underscore()
        ->multi_option_policy(CLI::MultiOptionPolicy::Join);
    app->add_option("--broker,--brokeraddress", broker, "address or name of the broker to connect")
        ->ignore_underscore();
    app->add_option("--brokerport", brokerPort, "Port number of the Broker")
        ->ignore_underscore()
        ->check(CLI::PositiveNumber);

    app->add_option_function<int>(
           "--port",
           [this](int port) {
               if (brokerPort > 0)
                   localport = std::to_string(port);
               else
                   brokerPort = port;
           },
           "Specify the port number to use")
        ->check(CLI::PositiveNumber);
    app->add_option("--localport", localport, "Port number to use for connections to this federate")
        ->ignore_underscore();
    app->add_flag(
        "--autobroker", autobroker, "tell the core to automatically generate a broker if needed");
    app->add_option(
        "--key,--broker_key",
        key,
        "specify a key to use to match a broker should match the broker key");
    app->add_option_function<Time>(
        "--offset",
        [this](Time val) { setProperty(helics_property_time_offset, val); },
        "the offset of the time steps (default in ms)");
    app->add_option_function<Time>(
        "--period",
        [this](Time val) { setProperty(helics_property_time_period, val); },
        "the execution cycle of the federate (default in ms)");
    app->add_option_function<Time>(
           "--timedelta",
           [this](Time val) { setProperty(helics_property_time_delta, val); },
           "The minimum time between time grants for a Federate (default in ms)")
        ->ignore_underscore();
    auto rtgroup = app->add_option_group("realtime");
    rtgroup
        ->add_option_function<Time>(
            "--rtlag",
            [this](Time val) { setProperty(helics_property_time_rt_lag, val); },
            "the amount of the time the federate is allowed to lag realtime before "
            "corrective action is taken (default in ms)")
        ->ignore_underscore();
    rtgroup
        ->add_option_function<Time>(
            "--rtlead",
            [this](Time val) { setProperty(helics_property_time_rt_lead, val); },
            "the amount of the time the federate is allowed to lead realtime before "
            "corrective action is taken (default in ms)")
        ->ignore_underscore();
    rtgroup
        ->add_option_function<Time>(
            "--rttolerance",
            [this](Time val) { setProperty(helics_property_time_rt_tolerance, val); },
            "the time tolerance of the real time mode (default in ms)")
        ->ignore_underscore();

    app->add_option_function<Time>(
           "--inputdelay",
           [this](Time val) { setProperty(helics_property_time_input_delay, val); },
           "the input delay on incoming communication of the federate (default in ms)")
        ->ignore_underscore();
    app->add_option_function<Time>(
           "--outputdelay",
           [this](Time val) { setProperty(helics_property_time_output_delay, val); },
           "the output delay for outgoing communication of the federate (default in ms)")
        ->ignore_underscore();
    app->add_option_function<int>(
           "--maxiterations",
           [this](int val) { setProperty(helics_property_int_max_iterations, val); },
           "the maximum number of iterations a federate is allowed to take")
        ->ignore_underscore()
        ->check(CLI::PositiveNumber);
    app->add_option_function<int>(
           "--loglevel,--log-level",
           [this](int val) { setProperty(helics_property_time_output_delay, val); },
           "the logging level of a federate")
        ->ignore_underscore()
        ->transform(
            CLI::CheckedTransformer(&log_level_map, CLI::ignore_case, CLI::ignore_underscore));

    app->add_option(
           "--separator",
           [this](CLI::results_t res) {
               if (res[0].size() != 1) return false;
               separator = res[0][0];
               return true;
           },
           "separator character for local federates")
        ->default_str(std::string(1, separator))
        ->type_size(1)
        ->type_name("CHAR");
    app->add_option("--flags,-f,--flag", "named flag for the federate")
        ->type_size(-1)
        ->delimiter(',')
        ->each([this](const std::string& flag) { loadFlags(*this, flag); });
    app->allow_extras();
#ifdef HELICS_DISABLE_ASIO
    rtgroup->disabled();
#endif
    return app;
}

std::vector<std::string> FederateInfo::loadInfoFromArgs(const std::string& args)
{
    auto app = makeCLIApp();
    auto ret = app->helics_parse(args);
    if (ret == helicsCLI11App::parse_output::ok) {
        coreType = app->getCoreType();
    } else if (ret == helicsCLI11App::parse_output::parse_error) {
        throw helics::InvalidParameter("argument parsing failed");
    }
    return app->remaining_for_passthrough();
}

std::vector<std::string> FederateInfo::loadInfoFromArgs(int argc, char* argv[])
{
    auto app = makeCLIApp();
    auto ret = app->helics_parse(argc, argv);
    if (ret == helicsCLI11App::parse_output::ok) {
        coreType = app->getCoreType();
    } else if (ret == helicsCLI11App::parse_output::parse_error) {
        throw helics::InvalidParameter("argument parsing failed");
    }
    return app->remaining_for_passthrough();
}

void FederateInfo::loadInfoFromArgsIgnoreOutput(const std::string& args)
{
    auto app = makeCLIApp();
    app->helics_parse(args);
    coreType = app->getCoreType();
}

void FederateInfo::loadInfoFromArgsIgnoreOutput(int argc, char* argv[])
{
    auto app = makeCLIApp();
    app->helics_parse(argc, argv);
    coreType = app->getCoreType();
}

void FederateInfo::loadInfoFromArgs(std::vector<std::string>& args)
{
    auto app = makeCLIApp();
    app->allow_extras();
    auto ret = app->helics_parse(args);
    if (ret == helicsCLI11App::parse_output::ok) {
        coreType = app->getCoreType();
    } else if (ret == helicsCLI11App::parse_output::parse_error) {
        throw helics::InvalidParameter("argument parsing failed");
    }
}

// function to load different formats easily
static FederateInfo loadFederateInfoJson(const std::string& jsonString);
static FederateInfo loadFederateInfoToml(const std::string& tomlString);

FederateInfo loadFederateInfo(const std::string& configString)
{
    FederateInfo ret;
    if (hasTomlExtension(configString)) {
        ret = loadFederateInfoToml(configString);
    } else if (
        (hasJsonExtension(configString)) ||
        (configString.find_first_of('{') != std::string::npos)) {
        ret = loadFederateInfoJson(configString);
    } else if (configString.find("--") != std::string::npos) {
        ret.loadInfoFromArgsIgnoreOutput(configString);
    } else {
        ret.defName = configString;
    }
    return ret;
}

FederateInfo loadFederateInfoJson(const std::string& jsonString)
{
    FederateInfo fi;
    Json::Value doc;
    try {
        doc = loadJson(jsonString);
    }
    catch (const std::invalid_argument& ia) {
        throw(helics::InvalidParameter(ia.what()));
    }

    std::function<void(const std::string&, bool)> flagCall =
        [&fi](const std::string& fname, bool arg) {
            fi.setFlagOption(propStringsTranslations.at(fname), arg);
        };

    std::function<void(const std::string&, Time)> timeCall =
        [&fi](const std::string& fname, Time arg) {
            fi.setProperty(propStringsTranslations.at(fname), arg);
        };

    std::function<void(const std::string&, int)> intCall =
        [&fi](const std::string& fname, int arg) {
            fi.setProperty(propStringsTranslations.at(fname), arg);
        };

    std::function<void(const std::string&)> logTranslations = [&](const std::string& arg) {
        auto valf = log_level_map.find(arg);
        if (valf != log_level_map.end()) {
            fi.setProperty(helics_property_int_log_level, valf->second);
        } else {
            throw(helics::InvalidIdentifier(arg + " is not a valid log level"));
        }
    };

    for (auto& prop : validTimeProperties) {
        callIfMember(doc, prop, timeCall);
    }

    if (!callIfMember(doc, "max_iterations", intCall)) {
        callIfMember(doc, "maxiterations", intCall);
    }

    bool lfound = callIfMember(doc, "log_level", intCall);
    if (!lfound) {
        lfound = callIfMember(doc, "loglevel", intCall);
    }
    if (!lfound) {
        lfound = callIfMember(doc, "log_level", logTranslations);
    }
    if (!lfound) {
        lfound = callIfMember(doc, "loglevel", logTranslations);
    }

    for (auto& prop : validFlagOptions) {
        callIfMember(doc, prop, flagCall);
    }
    if (doc.isMember("flags")) {
        loadFlags(fi, doc["flags"].asString());
    }

    replaceIfMember(doc, "broker", fi.broker);
    replaceIfMember(doc, "key", fi.key);
    fi.brokerPort = static_cast<int>(getOrDefault(doc, "brokerport", int64_t(fi.brokerPort)));
    replaceIfMember(doc, "localport", fi.localport);
    replaceIfMember(doc, "autobroker", fi.autobroker);
    if (doc.isMember("port")) {
        if (fi.localport.empty()) {
            if (fi.brokerPort < 0) {
                fi.brokerPort = doc["port"].asInt();
            } else {
                fi.localport = doc["port"].asString();
            }
        } else {
            if (fi.brokerPort < 0) {
                fi.brokerPort = doc["port"].asInt();
            }
        }
    }
    if (doc.isMember("separator")) {
        auto sep = doc["separator"].asString();
        if (!sep.empty()) {
            fi.separator = sep[0];
        }
    }
    if (doc.isMember("core")) {
        try {
            fi.coreType = coreTypeFromString(doc["core"].asString());
        }
        catch (const std::invalid_argument&) {
            fi.coreName = doc["core"].asString();
        }
    }
    if (doc.isMember("coreType")) {
        try {
            fi.coreType = coreTypeFromString(doc["coreType"].asString());
        }
        catch (const std::invalid_argument&) {
            std::cerr << "Unrecognized core type\n";
        }
    } else if (doc.isMember("coretype")) {
        try {
            fi.coreType = coreTypeFromString(doc["coretype"].asString());
        }
        catch (const std::invalid_argument&) {
            std::cerr << "Unrecognized core type\n";
        }
    } else if (doc.isMember("type")) {
        try {
            fi.coreType = coreTypeFromString(doc["type"].asString());
        }
        catch (const std::invalid_argument&) {
            std::cerr << "Unrecognized core type\n";
        }
    }
    replaceIfMember(doc, "name", fi.defName);
    replaceIfMember(doc, "coreName", fi.coreName);
    replaceIfMember(doc, "corename", fi.coreName);
    replaceIfMember(doc, "coreInit", fi.coreInitString);
    replaceIfMember(doc, "coreinit", fi.coreInitString);
    replaceIfMember(doc, "coreinitstring", fi.coreInitString);
    replaceIfMember(doc, "brokerInit", fi.brokerInitString);
    replaceIfMember(doc, "brokerinit", fi.brokerInitString);
    replaceIfMember(doc, "brokerinitstring", fi.brokerInitString);
    return fi;
}

FederateInfo loadFederateInfoToml(const std::string& tomlString)
{
    FederateInfo fi;
    toml::value doc;
    try {
        doc = loadToml(tomlString);
    }
    catch (const std::invalid_argument& ia) {
        throw(helics::InvalidParameter(ia.what()));
    }
    std::function<void(const std::string&, bool)> flagCall =
        [&fi](const std::string& fname, bool arg) {
            fi.setFlagOption(propStringsTranslations.at(fname), arg);
        };

    std::function<void(const std::string&, Time)> timeCall =
        [&fi](const std::string& fname, Time arg) {
            fi.setProperty(propStringsTranslations.at(fname), arg);
        };

    std::function<void(const std::string&, int)> intCall =
        [&fi](const std::string& fname, int arg) {
            fi.setProperty(propStringsTranslations.at(fname), arg);
        };

    std::function<void(const std::string&)> logTranslations = [&](const std::string& arg) {
        auto valf = log_level_map.find(arg);
        if (valf != log_level_map.end()) {
            fi.setProperty(helics_property_int_log_level, valf->second);
        } else {
            throw(helics::InvalidIdentifier(arg + " is not a valid log level"));
        }
    };

    for (auto& prop : validTimeProperties) {
        callIfMember(doc, prop, timeCall);
    }

    if (!callIfMember(doc, "max_iterations", intCall)) {
        callIfMember(doc, "maxiterations", intCall);
    }

    bool lfound = callIfMember(doc, "log_level", intCall);
    if (!lfound) {
        lfound = callIfMember(doc, "loglevel", intCall);
    }
    if (!lfound) {
        lfound = callIfMember(doc, "log_level", logTranslations);
    }
    if (!lfound) {
        lfound = callIfMember(doc, "loglevel", logTranslations);
    }

    for (auto& prop : validFlagOptions) {
        callIfMember(doc, prop, flagCall);
    }
    if (isMember(doc, "flags")) {
        loadFlags(fi, toml::find<std::string>(doc, "flags"));
    }
    replaceIfMember(doc, "autobroker", fi.autobroker);
    replaceIfMember(doc, "broker", fi.broker);
    replaceIfMember(doc, "key", fi.key);
    fi.brokerPort = static_cast<int>(getOrDefault(doc, "brokerport", int64_t(fi.brokerPort)));
    replaceIfMember(doc, "localport", fi.localport);
    if (isMember(doc, "port")) {
        if (fi.localport.empty()) {
            if (fi.brokerPort < 0) {
                fi.brokerPort = doc["port"].as_integer();
            } else {
                fi.localport = tomlAsString(doc["port"]);
            }
        } else {
            if (fi.brokerPort < 0) {
                fi.brokerPort = doc["port"].as_integer();
            }
        }
    }
    if (isMember(doc, "separator")) {
        std::string sep = tomlAsString(doc["separator"]);
        if (!sep.empty()) {
            fi.separator = sep[0];
        }
    }
    if (isMember(doc, "core")) {
        try {
            fi.coreType = coreTypeFromString(tomlAsString(doc["core"]));
        }
        catch (const std::invalid_argument&) {
            fi.coreName = tomlAsString(doc["core"]);
        }
    }
    if (isMember(doc, "coreType")) {
        try {
            fi.coreType = coreTypeFromString(tomlAsString(doc["coreType"]));
        }
        catch (const std::invalid_argument&) {
            std::cerr << "Unrecognized core type\n";
        }
    } else if (isMember(doc, "coretype")) {
        try {
            fi.coreType = coreTypeFromString(tomlAsString(doc["coretype"]));
        }
        catch (const std::invalid_argument&) {
            std::cerr << "Unrecognized core type\n";
        }
    } else if (isMember(doc, "type")) {
        try {
            fi.coreType = coreTypeFromString(tomlAsString(doc["type"]));
        }
        catch (const std::invalid_argument&) {
            std::cerr << "Unrecognized core type\n";
        }
    }
    replaceIfMember(doc, "name", fi.defName);
    replaceIfMember(doc, "coreName", fi.coreName);
    replaceIfMember(doc, "corename", fi.coreName);
    replaceIfMember(doc, "coreInit", fi.coreInitString);
    replaceIfMember(doc, "coreinit", fi.coreInitString);
    replaceIfMember(doc, "coreinitstring", fi.coreInitString);
    replaceIfMember(doc, "brokerInit", fi.brokerInitString);
    replaceIfMember(doc, "brokerinit", fi.brokerInitString);
    replaceIfMember(doc, "brokerinitstring", fi.brokerInitString);

    return fi;
}

std::string generateFullCoreInitString(const FederateInfo& fi)
{
    auto res = fi.coreInitString;
    if (!fi.broker.empty()) {
        res += " --broker=";
        res.append(fi.broker);
    }
    if (fi.brokerPort >= 0) {
        res += " --brokerport=";
        res.append(std::to_string(fi.brokerPort));
    }
    if (!fi.localport.empty()) {
        res += " --localport=";
        res.append(fi.localport);
    }
    if (fi.autobroker) {
        res.append(" --autobroker");
    }

    if (!fi.brokerInitString.empty()) {
        res.append(" --brokerinit \"");
        res.append(fi.brokerInitString);
        res.append("\"");
    }
    if (!fi.key.empty()) {
        res += " --key=";
        res.append(fi.key);
    }
    return res;
}

} // namespace helics
