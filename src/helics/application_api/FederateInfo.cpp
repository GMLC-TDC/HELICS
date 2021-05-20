/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "FederateInfo.hpp"

#include "../common/JsonProcessingFunctions.hpp"
#include "../common/TomlProcessingFunctions.hpp"
#include "../common/addTargets.hpp"
#include "../core/core-exceptions.hpp"
#include "../core/helicsCLI11.hpp"
#include "../core/helicsCLI11JsonConfig.hpp"
#include "../core/helicsVersion.hpp"
#include "gmlc/utilities/stringOps.h"

#include <iostream>
#include <map>
#include <set>
#include <unordered_map>
#include <utility>

namespace helics {
FederateInfo::FederateInfo()
{
    loadInfoFromArgsIgnoreOutput("");
}

FederateInfo::FederateInfo(core_type cType)
{
    loadInfoFromArgsIgnoreOutput("");
    coreType = cType;
}

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

static const std::unordered_map<std::string, int> propStringsTranslations{
    {"period", helics_property_time_period},
    {"timeperiod", helics_property_time_period},
    {"time_period", helics_property_time_period},
    {"timedelta", helics_property_time_delta},
    {"time_delta", helics_property_time_delta},
    {"timeDelta", helics_property_time_delta},
    {"delta", helics_property_time_delta},
    {"offset", helics_property_time_offset},
    {"timeoffset", helics_property_time_offset},
    {"time_offset", helics_property_time_offset},
    {"rtlead", helics_property_time_rt_lead},
    {"rtlag", helics_property_time_rt_lag},
    {"rttolerance", helics_property_time_rt_tolerance},
    {"timertlead", helics_property_time_rt_lead},
    {"timertlag", helics_property_time_rt_lag},
    {"timerttolerance", helics_property_time_rt_tolerance},
    {"rtLead", helics_property_time_rt_lead},
    {"rtLag", helics_property_time_rt_lag},
    {"rtTolerance", helics_property_time_rt_tolerance},
    {"rt_lead", helics_property_time_rt_lead},
    {"rt_lag", helics_property_time_rt_lag},
    {"rt_tolerance", helics_property_time_rt_tolerance},
    {"time_rt_lead", helics_property_time_rt_lead},
    {"time_rt_lag", helics_property_time_rt_lag},
    {"time_rt_tolerance", helics_property_time_rt_tolerance},
    {"inputdelay", helics_property_time_input_delay},
    {"outputdelay", helics_property_time_output_delay},
    {"inputDelay", helics_property_time_input_delay},
    {"outputDelay", helics_property_time_output_delay},
    {"input_delay", helics_property_time_input_delay},
    {"output_delay", helics_property_time_output_delay},
    {"timeinputdelay", helics_property_time_input_delay},
    {"timeoutputdelay", helics_property_time_output_delay},
    {"time_input_delay", helics_property_time_input_delay},
    {"time_output_delay", helics_property_time_output_delay},
    {"loglevel", helics_property_int_log_level},
    {"log_level", helics_property_int_log_level},
    {"logLevel", helics_property_int_log_level},
    {"intloglevel", helics_property_int_log_level},
    {"int_log_level", helics_property_int_log_level},
    {"consoleloglevel", helics_property_int_console_log_level},
    {"console_log_level", helics_property_int_console_log_level},
    {"intconsoleloglevel", helics_property_int_console_log_level},
    {"int_console_log_level", helics_property_int_console_log_level},
    {"fileloglevel", helics_property_int_file_log_level},
    {"file_log_level", helics_property_int_file_log_level},
    {"intfileloglevel", helics_property_int_file_log_level},
    {"int_file_log_level", helics_property_int_file_log_level},
    {"maxiterations", helics_property_int_max_iterations},
    {"max_iterations", helics_property_int_max_iterations},
    {"maxIterations", helics_property_int_max_iterations},
    {"intmaxiterations", helics_property_int_max_iterations},
    {"int_max_iterations", helics_property_int_max_iterations},
    {"iterations", helics_property_int_max_iterations}};

static const std::unordered_map<std::string, int> flagStringsTranslations{
    {"source_only", helics_flag_source_only},
    {"sourceonly", helics_flag_source_only},
    {"sourceOnly", helics_flag_source_only},
    {"source", helics_flag_source_only},
    {"observer", helics_flag_observer},
    {"slow", helics_flag_slow_responding},
    {"slow_response", helics_flag_slow_responding},
    {"slow_responding", helics_flag_slow_responding},
    {"slowResponding", helics_flag_slow_responding},
    {"no_ping", helics_flag_slow_responding},
    {"disable_ping", helics_flag_slow_responding},
    {"debugging", helics_flag_debugging},
    {"only_update_on_change", helics_flag_only_update_on_change},
    {"only_transmit_on_change", helics_flag_only_transmit_on_change},
    {"forward_compute", helics_flag_forward_compute},
    {"realtime", helics_flag_realtime},
    {"real_time", helics_flag_realtime},
    {"realTime", helics_flag_realtime},
    {"restrictive_time_policy", helics_flag_restrictive_time_policy},
    {"conservative_time_policy", helics_flag_restrictive_time_policy},
    {"restrictive_time", helics_flag_restrictive_time_policy},
    {"conservative_time", helics_flag_restrictive_time_policy},
    {"restrictiveTime", helics_flag_restrictive_time_policy},
    {"conservativeTime", helics_flag_restrictive_time_policy},
    {"ignore_time_mismatch", helics_flag_ignore_time_mismatch_warnings},
    {"delayed_update", helics_flag_wait_for_current_time_update},
    {"delayedUpdate", helics_flag_wait_for_current_time_update},
    {"strict_input_type_checking", helics_handle_option_strict_type_checking},
    {"strict_config_checking", helics_flag_strict_config_checking},
    {"strictconfigchecking", helics_flag_strict_config_checking},
    {"strictConfigChecking", helics_flag_strict_config_checking},
    {"ignore_unit_mismatch", helics_handle_option_ignore_unit_mismatch},
    {"buffer_data", helics_handle_option_buffer_data},
    {"bufferData", helics_handle_option_buffer_data},
    {"required", helics_handle_option_connection_required},
    {"optional", helics_handle_option_connection_optional},
    {"nointerrupts", helics_flag_uninterruptible},
    {"no_interrupts", helics_flag_uninterruptible},
    {"uninterruptible", helics_flag_uninterruptible},
    {"interruptible", helics_flag_interruptible},
    {"wait_for_current_time", helics_flag_wait_for_current_time_update},
    {"wait_for_current_time_update", helics_flag_wait_for_current_time_update},
    {"waitforcurrenttimeupdate", helics_flag_wait_for_current_time_update},
    {"waitforcurrenttime", helics_flag_wait_for_current_time_update},
    {"delay_init_entry", helics_flag_delay_init_entry},
    {"delayinitentry", helics_flag_delay_init_entry},
    {"enable_init_entry", helics_flag_enable_init_entry},
    {"enableinitentry", helics_flag_enable_init_entry},
    {"ignore_time_mismatch_warnings", helics_flag_ignore_time_mismatch_warnings},
    {"ignoretimemismatchwarnings", helics_flag_ignore_time_mismatch_warnings},
    {"rollback", helics_flag_rollback},
    {"single_thread_federate", helics_flag_single_thread_federate},
    {"singlethreadfederate", helics_flag_single_thread_federate},
    {"force_logging_flush", helics_flag_force_logging_flush},
    {"forceloggingflush", helics_flag_force_logging_flush},
    {"dumplog", helics_flag_dumplog},
    {"event_triggered", helics_flag_event_triggered},
    {"eventtriggered", helics_flag_event_triggered},
    {"eventTriggered", helics_flag_event_triggered},
    {"terminate_on_error", helics_flag_terminate_on_error},
    {"terminateOnError", helics_flag_terminate_on_error},
    {"terminateonerror", helics_flag_terminate_on_error}};

static const std::unordered_map<std::string, int> optionStringsTranslations{
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
    {"strict", helics_handle_option_strict_type_checking},
    {"connections", helics_handle_option_connections},
    {"clear_priority_list", helics_handle_option_clear_priority_list},
    {"clear_priority", helics_handle_option_clear_priority_list},
    {"input_priority", helics_handle_option_input_priority_location},
    {"priority", helics_handle_option_input_priority_location},
    {"input_priority_location", helics_handle_option_input_priority_location},
    {"priority_location", helics_handle_option_input_priority_location},
    {"multi_input_handling_method", helics_handle_option_multi_input_handling_method},
    {"multi_input_handling", helics_handle_option_multi_input_handling_method}};

static const std::map<std::string, int> option_value_map{
    {"0", 0},
    {"1", 1},
    {"false", 0},
    {"true", 1},
    {"on", 1},
    {"off", 0},
    {"disable", 0},
    {"enable", 1},
    {"disabled", 0},
    {"enabled", 1},
    {"2", 2},
    {"3", 3},
    {"4", 4},
    // vector operation values
    {"none", helics_multi_input_no_op},
    {"no_op", helics_multi_input_no_op},
    {"and", helics_multi_input_and_operation},
    {"or", helics_multi_input_or_operation},
    {"sum", helics_multi_input_sum_operation},
    {"max", helics_multi_input_max_operation},
    {"min", helics_multi_input_min_operation},
    {"average", helics_multi_input_average_operation},
    {"mean", helics_multi_input_average_operation},
    {"vectorize", helics_multi_input_vectorize_operation},
    {"diff", helics_multi_input_diff_operation}};

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
        if (flg == "debugging") {
            fi.debugging = true;
        }
        if (flg.empty()) {
            continue;  // LCOV_EXCL_LINE
        }
        auto loc = flagStringsTranslations.find(flg);
        if (loc != flagStringsTranslations.end()) {
            fi.setFlagOption(loc->second, true);
        } else {
            if (flg.front() == '-') {
                loc = flagStringsTranslations.find(flg.substr(1));
                if (loc != flagStringsTranslations.end()) {
                    fi.setFlagOption(loc->second, false);
                }
                continue;
            }
            try {
                auto val = std::stoi(flg);
                fi.setFlagOption(std::abs(val), (val > 0));
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
    auto res = getFlagIndex(val);
    if (res >= 0) {
        return res;
    }
    val.erase(std::remove(val.begin(), val.end(), '_'), val.end());
    fnd = propStringsTranslations.find(val);
    if (fnd != propStringsTranslations.end()) {
        return fnd->second;
    }
    return getFlagIndex(val);
}

int getFlagIndex(std::string val)
{
    auto fnd = flagStringsTranslations.find(val);
    if (fnd != flagStringsTranslations.end()) {
        return fnd->second;
    }
    gmlc::utilities::makeLowerCase(val);
    fnd = flagStringsTranslations.find(val);
    if (fnd != flagStringsTranslations.end()) {
        return fnd->second;
    }
    val.erase(std::remove(val.begin(), val.end(), '_'), val.end());
    fnd = flagStringsTranslations.find(val);
    if (fnd != flagStringsTranslations.end()) {
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

int getOptionValue(std::string val)
{
    auto fnd2 = option_value_map.find(val);
    if (fnd2 != option_value_map.end()) {
        return fnd2->second;
    }
    auto fnd = log_level_map.find(val);
    if (fnd != log_level_map.end()) {
        return fnd->second;
    }
    gmlc::utilities::makeLowerCase(val);
    fnd2 = option_value_map.find(val);
    if (fnd2 != option_value_map.end()) {
        return fnd2->second;
    }
    fnd = log_level_map.find(val);
    if (fnd != log_level_map.end()) {
        return fnd->second;
    }
    return -1;
}

std::unique_ptr<helicsCLI11App> FederateInfo::makeCLIApp()
{
    auto app = std::make_unique<helicsCLI11App>("Federate Info Parsing");
    app->option_defaults()->ignore_case();
    app->allow_config_extras(CLI::config_extras_mode::ignore_all);
    app->set_config("--config-file,--config,config",
                    "helicsConfig.ini",
                    "specify a configuration file");
    auto* fmtr = addJsonConfig(app.get());
    fmtr->maxLayers(0);
    app->add_option("--name,-n", defName, "name of the federate");
    auto* og = app->add_option_group("network type")->immediate_callback();
    og->add_option_function<std::string>(
          "--core",
          [this](const std::string& val) {
              coreType = coreTypeFromString(val);
              if (coreType == core_type::UNRECOGNIZED) {
                  coreName = val;
              }
          },
          "type or name of the core to connect to")
        ->default_str("(" + to_string(coreType) + ")");
    og->add_flag("--force_new_core",
                 forceNewCore,
                 "if set to true will force the federate to generate a new core");
    og->add_option_function<std::string>(
          "--coretype,-t,--type",
          [this](const std::string& val) {
              coreType = coreTypeFromString(val);
              if (coreType == core_type::UNRECOGNIZED) {
                  throw CLI::ValidationError(val + " is NOT a recognized core type");
              }
          },
          "type  of the core to connect to")
        ->default_str("(" + to_string(coreType) + ")")
        ->envname("HELICS_CORE_TYPE");
    app->add_option("--corename", coreName, "the name of the core to create or find")
        ->ignore_underscore();
    app->add_option("--coreinitstring,-i,--coreinit",
                    coreInitString,
                    "The initialization arguments for the core")
        ->transform([](std::string arg) {
            arg.insert(arg.begin(), ' ');
            return arg;
        })
        ->ignore_underscore()
        ->multi_option_policy(CLI::MultiOptionPolicy::Join)
        ->envname("HELICS_CORE_INIT_STRING");
    app->add_option("--brokerinitstring,--brokerinit",
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
               if (brokerPort > 0) {
                   localport = std::to_string(port);
               } else {
                   brokerPort = port;
               }
           },
           "Specify the port number to use")
        ->check(CLI::PositiveNumber);
    app->add_option("--localport", localport, "Port number to use for connections to this federate")
        ->ignore_underscore();
    app->add_flag("--autobroker",
                  autobroker,
                  "tell the core to automatically generate a broker if needed");
    app->add_flag("--debugging",
                  debugging,
                  "tell the core to allow user debugging in a nicer fashion");
    app->add_option("--key,--broker_key",
                    key,
                    "specify a key to use to match a broker should match the broker key");
    app->add_option_function<Time>(
           "--offset",
           [this](Time val) { setProperty(helics_property_time_offset, val); },
           "the offset of the time steps (default in ms)")
        ->configurable(false);
    app->add_option_function<Time>(
           "--period",
           [this](Time val) { setProperty(helics_property_time_period, val); },
           "the execution cycle of the federate (default in ms)")
        ->configurable(false);
    app->add_option_function<Time>(
           "--timedelta",
           [this](Time val) { setProperty(helics_property_time_delta, val); },
           "The minimum time between time grants for a Federate (default in ms)")
        ->ignore_underscore()
        ->configurable(false);
    auto* rtgroup = app->add_option_group("realtime");
    rtgroup->option_defaults()->ignore_underscore();
    rtgroup
        ->add_option_function<Time>(
            "--rtlag",
            [this](Time val) { setProperty(helics_property_time_rt_lag, val); },
            "the amount of the time the federate is allowed to lag realtime before "
            "corrective action is taken (default in ms)")
        ->configurable(false);
    rtgroup
        ->add_option_function<Time>(
            "--rtlead",
            [this](Time val) { setProperty(helics_property_time_rt_lead, val); },
            "the amount of the time the federate is allowed to lead realtime before "
            "corrective action is taken (default in ms)")
        ->configurable(false);
    rtgroup
        ->add_option_function<Time>(
            "--rttolerance",
            [this](Time val) { setProperty(helics_property_time_rt_tolerance, val); },
            "the time tolerance of the real time mode (default in ms)")
        ->configurable(false);

    app->add_option_function<Time>(
           "--inputdelay",
           [this](Time val) { setProperty(helics_property_time_input_delay, val); },
           "the input delay on incoming communication of the federate (default in ms)")
        ->ignore_underscore()
        ->configurable(false);
    app->add_option_function<Time>(
           "--outputdelay",
           [this](Time val) { setProperty(helics_property_time_output_delay, val); },
           "the output delay for outgoing communication of the federate (default in ms)")
        ->ignore_underscore()
        ->configurable(false);
    app->add_option_function<int>(
           "--maxiterations",
           [this](int val) { setProperty(helics_property_int_max_iterations, val); },
           "the maximum number of iterations a federate is allowed to take")
        ->ignore_underscore()
        ->check(CLI::PositiveNumber);
    app->add_option_function<int>(
           "--loglevel,--log-level",
           [this](int val) { setProperty(helics_property_int_log_level, val); },
           "the logging level of a federate")
        ->ignore_underscore()
        ->transform(
            CLI::CheckedTransformer(&log_level_map, CLI::ignore_case, CLI::ignore_underscore))
        ->envname("HELICS_LOG_LEVEL");

    app->add_option("--separator", separator, "separator character for local federates")
        ->default_str(std::string(1, separator));
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
    if (ret == helicsCLI11App::parse_output::parse_error) {
        throw helics::InvalidParameter("argument parsing failed");
    }
    config_additional(app.get());
    return app->remainArgs();
}

std::vector<std::string> FederateInfo::loadInfoFromArgs(int argc, char* argv[])
{
    auto app = makeCLIApp();
    auto ret = app->helics_parse(argc, argv);
    if (ret == helicsCLI11App::parse_output::parse_error) {
        throw helics::InvalidParameter("argument parsing failed");
    }
    config_additional(app.get());
    return app->remainArgs();
}

void FederateInfo::loadInfoFromArgsIgnoreOutput(const std::string& args)
{
    auto app = makeCLIApp();
    auto ret = app->helics_parse(args);
    if (ret == helicsCLI11App::parse_output::parse_error) {
        throw helics::InvalidParameter("argument parsing failed");
    }
    config_additional(app.get());
}

void FederateInfo::loadInfoFromArgsIgnoreOutput(int argc, char* argv[])
{
    auto app = makeCLIApp();
    auto ret = app->helics_parse(argc, argv);
    if (ret == helicsCLI11App::parse_output::parse_error) {
        throw helics::InvalidParameter("argument parsing failed");
    }
    config_additional(app.get());
}

void FederateInfo::loadInfoFromArgs(std::vector<std::string>& args)
{
    auto app = makeCLIApp();
    app->allow_extras();
    auto ret = app->helics_parse(args);
    if (ret == helicsCLI11App::parse_output::parse_error) {
        throw helics::InvalidParameter("argument parsing failed");
    }
    config_additional(app.get());
}

void FederateInfo::config_additional(helicsCLI11App* app)
{
    auto* opt = app->get_option("--config");
    if (opt->count() > 0) {
        auto configString = opt->as<std::string>();
        if (hasTomlExtension(configString)) {
            loadInfoFromToml(configString, false);
            fileInUse = configString;
        } else if (hasJsonExtension(configString)) {
            loadInfoFromJson(configString, false);
            fileInUse = configString;
        }
    }
}

FederateInfo loadFederateInfo(const std::string& configString)
{
    FederateInfo ret;

    if (hasTomlExtension(configString)) {
        ret.loadInfoFromToml(configString);
        ret.fileInUse = configString;
    } else if (hasJsonExtension(configString)) {
        ret.loadInfoFromJson(configString);
        ret.fileInUse = configString;
    } else if (configString.find_first_of('{') != std::string::npos) {
        ret.loadInfoFromJson(configString);
    } else if (configString.find("--") != std::string::npos) {
        ret.loadInfoFromArgsIgnoreOutput(configString);
    } else if (configString.find('=') != std::string::npos) {
        ret.loadInfoFromToml(configString);
    } else {
        ret.defName = configString;
    }
    return ret;
}

Time FederateInfo::checkTimeProperty(int propId, Time defVal) const
{
    for (const auto& tp : timeProps) {
        if (tp.first == propId) {
            return tp.second;
        }
    }
    return defVal;
}

bool FederateInfo::checkFlagProperty(int propId, bool defVal) const
{
    for (const auto& tp : flagProps) {
        if (tp.first == propId) {
            return tp.second;
        }
    }
    return defVal;
}

int FederateInfo::checkIntProperty(int propId, int defVal) const
{
    for (const auto& tp : intProps) {
        if (tp.first == propId) {
            return tp.second;
        }
    }
    return defVal;
}

void FederateInfo::loadInfoFromJson(const std::string& jsonString, bool runArgParser)
{
    Json::Value doc;
    try {
        doc = loadJson(jsonString);
    }
    catch (const std::invalid_argument& ia) {
        throw(helics::InvalidParameter(ia.what()));
    }

    std::function<void(const std::string&, Time)> timeCall = [this](const std::string& fname,
                                                                    Time arg) {
        setProperty(propStringsTranslations.at(fname), arg);
    };

    for (const auto& prop : propStringsTranslations) {
        if (prop.second > 200) {
            continue;
        }
        callIfMember(doc, prop.first, timeCall);
    }

    processOptions(
        doc,
        [](const std::string& option) { return getFlagIndex(option); },
        [](const std::string& value) { return getOptionValue(value); },
        [this](int32_t option, int32_t value) { setFlagOption(option, value != 0); });

    if (runArgParser) {
        auto app = makeCLIApp();
        app->allow_extras();
        try {
            if (jsonString.find('{') != std::string::npos) {
                std::istringstream jstring(jsonString);
                app->parse_from_stream(jstring);
            } else {
                std::ifstream file(jsonString);
                app->parse_from_stream(file);
            }
        }
        catch (const CLI::Error& e) {
            throw(InvalidIdentifier(e.what()));
        }
    }
}

void FederateInfo::loadInfoFromToml(const std::string& tomlString, bool runArgParser)
{
    toml::value doc;
    try {
        doc = loadToml(tomlString);
    }
    catch (const std::invalid_argument& ia) {
        throw(helics::InvalidParameter(ia.what()));
    }

    std::function<void(const std::string&, Time)> timeCall = [this](const std::string& fname,
                                                                    Time arg) {
        setProperty(propStringsTranslations.at(fname), arg);
    };

    for (const auto& prop : propStringsTranslations) {
        if (prop.second > 200) {
            continue;
        }
        callIfMember(doc, prop.first, timeCall);
    }

    processOptions(
        doc,
        [](const std::string& option) { return getFlagIndex(option); },
        [](const std::string& value) { return getOptionValue(value); },
        [this](int32_t option, int32_t value) { setFlagOption(option, value != 0); });

    if (runArgParser) {
        auto app = makeCLIApp();
        app->allow_extras();
        auto dptr = std::static_pointer_cast<HelicsConfigJSON>(app->get_config_formatter_base());
        dptr->skipJson(true);
        try {
            if (tomlString.find('=') != std::string::npos) {
                std::istringstream tstring(tomlString);
                app->parse_from_stream(tstring);
            } else {
                std::ifstream file(tomlString);
                app->parse_from_stream(file);
            }
        }
        catch (const CLI::Error& e) {
            throw(InvalidIdentifier(e.what()));
        }
    }
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
    if (fi.debugging) {
        res.append(" --debugging");
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
    if (!fi.fileInUse.empty()) {  // we used the file, specify a core section
        res += " --config_section=core --config-file=";
        res.append(fi.fileInUse);
    }
    return res;
}

}  // namespace helics
