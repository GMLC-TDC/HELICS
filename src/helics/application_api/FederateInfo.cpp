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

FederateInfo::FederateInfo(CoreType cType)
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
    {"period", HELICS_PROPERTY_TIME_PERIOD},
    {"timeperiod", HELICS_PROPERTY_TIME_PERIOD},
    {"time_period", HELICS_PROPERTY_TIME_PERIOD},
    {"delta", HELICS_PROPERTY_TIME_DELTA},
    {"timedelta", HELICS_PROPERTY_TIME_DELTA},
    {"time_delta", HELICS_PROPERTY_TIME_DELTA},
    {"timeDelta", HELICS_PROPERTY_TIME_DELTA},
    {"offset", HELICS_PROPERTY_TIME_OFFSET},
    {"timeoffset", HELICS_PROPERTY_TIME_OFFSET},
    {"time_offset", HELICS_PROPERTY_TIME_OFFSET},
    {"rtlead", HELICS_PROPERTY_TIME_RT_LEAD},
    {"rtlag", HELICS_PROPERTY_TIME_RT_LAG},
    {"rttolerance", HELICS_PROPERTY_TIME_RT_TOLERANCE},
    {"timertlead", HELICS_PROPERTY_TIME_RT_LEAD},
    {"timertlag", HELICS_PROPERTY_TIME_RT_LAG},
    {"timerttolerance", HELICS_PROPERTY_TIME_RT_TOLERANCE},
    {"rtLead", HELICS_PROPERTY_TIME_RT_LEAD},
    {"rtLag", HELICS_PROPERTY_TIME_RT_LAG},
    {"rtTolerance", HELICS_PROPERTY_TIME_RT_TOLERANCE},
    {"rt_lead", HELICS_PROPERTY_TIME_RT_LEAD},
    {"rt_lag", HELICS_PROPERTY_TIME_RT_LAG},
    {"rt_tolerance", HELICS_PROPERTY_TIME_RT_TOLERANCE},
    {"time_rt_lead", HELICS_PROPERTY_TIME_RT_LEAD},
    {"time_rt_lag", HELICS_PROPERTY_TIME_RT_LAG},
    {"time_rt_tolerance", HELICS_PROPERTY_TIME_RT_TOLERANCE},
    {"inputdelay", HELICS_PROPERTY_TIME_INPUT_DELAY},
    {"outputdelay", HELICS_PROPERTY_TIME_OUTPUT_DELAY},
    {"inputDelay", HELICS_PROPERTY_TIME_INPUT_DELAY},
    {"outputDelay", HELICS_PROPERTY_TIME_OUTPUT_DELAY},
    {"input_delay", HELICS_PROPERTY_TIME_INPUT_DELAY},
    {"output_delay", HELICS_PROPERTY_TIME_OUTPUT_DELAY},
    {"timeinputdelay", HELICS_PROPERTY_TIME_INPUT_DELAY},
    {"timeoutputdelay", HELICS_PROPERTY_TIME_OUTPUT_DELAY},
    {"time_input_delay", HELICS_PROPERTY_TIME_INPUT_DELAY},
    {"time_output_delay", HELICS_PROPERTY_TIME_OUTPUT_DELAY},
    {"granttimeout", HELICS_PROPERTY_TIME_GRANT_TIMEOUT},
    {"grant_timeout", HELICS_PROPERTY_TIME_GRANT_TIMEOUT},
    {"loglevel", HELICS_PROPERTY_INT_LOG_LEVEL},
    {"log_level", HELICS_PROPERTY_INT_LOG_LEVEL},
    {"logLevel", HELICS_PROPERTY_INT_LOG_LEVEL},
    {"intloglevel", HELICS_PROPERTY_INT_LOG_LEVEL},
    {"intLogLevel", HELICS_PROPERTY_INT_LOG_LEVEL},
    {"int_log_level", HELICS_PROPERTY_INT_LOG_LEVEL},
    {"consoleLogLevel", HELICS_PROPERTY_INT_CONSOLE_LOG_LEVEL},
    {"consoleloglevel", HELICS_PROPERTY_INT_CONSOLE_LOG_LEVEL},
    {"console_log_level", HELICS_PROPERTY_INT_CONSOLE_LOG_LEVEL},
    {"intconsoleloglevel", HELICS_PROPERTY_INT_CONSOLE_LOG_LEVEL},
    {"intConsoleLogLevel", HELICS_PROPERTY_INT_CONSOLE_LOG_LEVEL},
    {"int_console_log_level", HELICS_PROPERTY_INT_CONSOLE_LOG_LEVEL},
    {"fileloglevel", HELICS_PROPERTY_INT_FILE_LOG_LEVEL},
    {"fileLogLevel", HELICS_PROPERTY_INT_FILE_LOG_LEVEL},
    {"file_log_level", HELICS_PROPERTY_INT_FILE_LOG_LEVEL},
    {"intfileloglevel", HELICS_PROPERTY_INT_FILE_LOG_LEVEL},
    {"intFileLogLevel", HELICS_PROPERTY_INT_FILE_LOG_LEVEL},
    {"int_file_log_level", HELICS_PROPERTY_INT_FILE_LOG_LEVEL},
    {"maxiterations", HELICS_PROPERTY_INT_MAX_ITERATIONS},
    {"max_iterations", HELICS_PROPERTY_INT_MAX_ITERATIONS},
    {"maxIterations", HELICS_PROPERTY_INT_MAX_ITERATIONS},
    {"intmaxiterations", HELICS_PROPERTY_INT_MAX_ITERATIONS},
    {"intMaxIterations", HELICS_PROPERTY_INT_MAX_ITERATIONS},
    {"int_max_iterations", HELICS_PROPERTY_INT_MAX_ITERATIONS}};

static const std::unordered_map<std::string, int> flagStringsTranslations{
    {"source_only", HELICS_FLAG_SOURCE_ONLY},
    {"sourceonly", HELICS_FLAG_SOURCE_ONLY},
    {"sourceOnly", HELICS_FLAG_SOURCE_ONLY},
    {"source", HELICS_FLAG_SOURCE_ONLY},
    {"observer", HELICS_FLAG_OBSERVER},
    {"slow", HELICS_FLAG_SLOW_RESPONDING},
    {"slow_response", HELICS_FLAG_SLOW_RESPONDING},
    {"slow_responding", HELICS_FLAG_SLOW_RESPONDING},
    {"slowResponding", HELICS_FLAG_SLOW_RESPONDING},
    {"uninterruptible", HELICS_FLAG_UNINTERRUPTIBLE},
    {"interruptible", HELICS_FLAG_INTERRUPTIBLE},
    {"debugging", HELICS_FLAG_DEBUGGING},
    {"profiling", HELICS_FLAG_PROFILING},
    {"local_profiling_capture", HELICS_FLAG_LOCAL_PROFILING_CAPTURE},
    {"profiling_marker", HELICS_FLAG_PROFILING_MARKER},
    {"only_update_on_change", HELICS_FLAG_ONLY_UPDATE_ON_CHANGE},
    {"onlyupdateonchange", HELICS_FLAG_ONLY_UPDATE_ON_CHANGE},
    {"onlyUpdateOnChange", HELICS_FLAG_ONLY_UPDATE_ON_CHANGE},
    {"only_transmit_on_change", HELICS_FLAG_ONLY_TRANSMIT_ON_CHANGE},
    {"onlytransmitonchange", HELICS_FLAG_ONLY_TRANSMIT_ON_CHANGE},
    {"onlyTransmitOnChange", HELICS_FLAG_ONLY_TRANSMIT_ON_CHANGE},
    {"forward_compute", HELICS_FLAG_FORWARD_COMPUTE},
    {"forwardcompute", HELICS_FLAG_FORWARD_COMPUTE},
    {"forwardCompute", HELICS_FLAG_FORWARD_COMPUTE},
    {"real_time", HELICS_FLAG_REALTIME},
    {"realtime", HELICS_FLAG_REALTIME},
    {"real_time", HELICS_FLAG_REALTIME},
    {"realTime", HELICS_FLAG_REALTIME},
    {"json", HELICS_FLAG_USE_JSON_SERIALIZATION},
    {"use_json_serialization", HELICS_FLAG_USE_JSON_SERIALIZATION},
    {"restrictivetimepolicy", HELICS_FLAG_RESTRICTIVE_TIME_POLICY},
    {"restrictive_time_policy", HELICS_FLAG_RESTRICTIVE_TIME_POLICY},
    {"restrictiveTimePolicy", HELICS_FLAG_RESTRICTIVE_TIME_POLICY},
    {"ignore_time_mismatch", HELICS_FLAG_IGNORE_TIME_MISMATCH_WARNINGS},
    {"ignoretimeMismatch", HELICS_FLAG_IGNORE_TIME_MISMATCH_WARNINGS},
    {"ignore_time_mismatch", HELICS_FLAG_IGNORE_TIME_MISMATCH_WARNINGS},
    {"strict_input_type_checking", HELICS_HANDLE_OPTION_STRICT_TYPE_CHECKING},
    {"strict_config_checking", HELICS_FLAG_STRICT_CONFIG_CHECKING},
    {"strictconfigchecking", HELICS_FLAG_STRICT_CONFIG_CHECKING},
    {"strictConfigChecking", HELICS_FLAG_STRICT_CONFIG_CHECKING},
    {"strictinputtypechecking", HELICS_HANDLE_OPTION_STRICT_TYPE_CHECKING},
    {"strictInputTypeChecking", HELICS_HANDLE_OPTION_STRICT_TYPE_CHECKING},
    {"ignore_unit_mismatch", HELICS_HANDLE_OPTION_IGNORE_UNIT_MISMATCH},
    {"ignoreunitmismatch", HELICS_HANDLE_OPTION_IGNORE_UNIT_MISMATCH},
    {"ignoreUnitMismatch", HELICS_HANDLE_OPTION_IGNORE_UNIT_MISMATCH},
    {"buffer_data", HELICS_HANDLE_OPTION_BUFFER_DATA},
    {"bufferdata", HELICS_HANDLE_OPTION_BUFFER_DATA},
    {"bufferData", HELICS_HANDLE_OPTION_BUFFER_DATA},

    {"optional", HELICS_HANDLE_OPTION_CONNECTION_OPTIONAL},
    {"uninterruptible", HELICS_FLAG_UNINTERRUPTIBLE},
    {"interruptible", HELICS_FLAG_INTERRUPTIBLE},
    {"connection_required", HELICS_HANDLE_OPTION_CONNECTION_REQUIRED},
    {"connectionrequired", HELICS_HANDLE_OPTION_CONNECTION_REQUIRED},
    {"connectionRequired", HELICS_HANDLE_OPTION_CONNECTION_REQUIRED},
    {"required",
     HELICS_HANDLE_OPTION_CONNECTION_REQUIRED},  // LIKELY TO BE DEPRECATED In the future
    {"connectionoptional", HELICS_HANDLE_OPTION_CONNECTION_OPTIONAL},
    {"connection_optional", HELICS_HANDLE_OPTION_CONNECTION_OPTIONAL},
    {"connectionOptional", HELICS_HANDLE_OPTION_CONNECTION_OPTIONAL},
    {"wait_for_current_time_update", HELICS_FLAG_WAIT_FOR_CURRENT_TIME_UPDATE},
    {"waitforcurrenttimeupdate", HELICS_FLAG_WAIT_FOR_CURRENT_TIME_UPDATE},
    {"waitForCurrentTimeUpdate", HELICS_FLAG_WAIT_FOR_CURRENT_TIME_UPDATE},
    {"delay_init_entry", HELICS_FLAG_DELAY_INIT_ENTRY},
    {"delayinitentry", HELICS_FLAG_DELAY_INIT_ENTRY},
    {"delayInitEntry", HELICS_FLAG_DELAY_INIT_ENTRY},
    {"enable_init_entry", HELICS_FLAG_ENABLE_INIT_ENTRY},
    {"enableinitentry", HELICS_FLAG_ENABLE_INIT_ENTRY},
    {"enableInitEntry", HELICS_FLAG_ENABLE_INIT_ENTRY},
    {"ignore_time_mismatch_warnings", HELICS_FLAG_IGNORE_TIME_MISMATCH_WARNINGS},
    {"ignoretimemismatchwarnings", HELICS_FLAG_IGNORE_TIME_MISMATCH_WARNINGS},
    {"ignoreTimeMismatchWarnings", HELICS_FLAG_IGNORE_TIME_MISMATCH_WARNINGS},
    {"rollback", HELICS_FLAG_ROLLBACK},
    {"single_thread_federate", HELICS_FLAG_SINGLE_THREAD_FEDERATE},
    {"singlethreadfederate", HELICS_FLAG_SINGLE_THREAD_FEDERATE},
    {"singleThreadFederate", HELICS_FLAG_SINGLE_THREAD_FEDERATE},
    {"force_logging_flush", HELICS_FLAG_FORCE_LOGGING_FLUSH},
    {"forceloggingflush", HELICS_FLAG_FORCE_LOGGING_FLUSH},
    {"forceLoggingFlush", HELICS_FLAG_FORCE_LOGGING_FLUSH},
    {"dump_log", HELICS_FLAG_DUMPLOG},
    {"dumplog", HELICS_FLAG_DUMPLOG},
    {"dumpLog", HELICS_FLAG_DUMPLOG},
    {"event_triggered", HELICS_FLAG_EVENT_TRIGGERED},
    {"eventtriggered", HELICS_FLAG_EVENT_TRIGGERED},
    {"eventTriggered", HELICS_FLAG_EVENT_TRIGGERED},
    {"terminate_on_error", HELICS_FLAG_TERMINATE_ON_ERROR},
    {"terminateOnError", HELICS_FLAG_TERMINATE_ON_ERROR},
    {"terminateonerror", HELICS_FLAG_TERMINATE_ON_ERROR}};

static const std::unordered_map<std::string, int> optionStringsTranslations{
    {"buffer_data", HELICS_HANDLE_OPTION_BUFFER_DATA},
    {"bufferdata", HELICS_HANDLE_OPTION_BUFFER_DATA},
    {"bufferData", HELICS_HANDLE_OPTION_BUFFER_DATA},
    {"connectionoptional", HELICS_HANDLE_OPTION_CONNECTION_OPTIONAL},
    {"connection_optional", HELICS_HANDLE_OPTION_CONNECTION_OPTIONAL},
    {"connectionOptional", HELICS_HANDLE_OPTION_CONNECTION_OPTIONAL},
    {"connectionrequired", HELICS_HANDLE_OPTION_CONNECTION_REQUIRED},
    {"connection_required", HELICS_HANDLE_OPTION_CONNECTION_REQUIRED},
    {"required",
     HELICS_HANDLE_OPTION_CONNECTION_REQUIRED},  // LIKELY to be deprecated in the future
    {"connectionRequired", HELICS_HANDLE_OPTION_CONNECTION_REQUIRED},
    {"uninterruptible", HELICS_HANDLE_OPTION_IGNORE_INTERRUPTS},
    {"multiple_connections_allowed", HELICS_HANDLE_OPTION_MULTIPLE_CONNECTIONS_ALLOWED},
    {"multipleconnectionsallowed", HELICS_HANDLE_OPTION_MULTIPLE_CONNECTIONS_ALLOWED},
    {"multipleConnectionsAllowed", HELICS_HANDLE_OPTION_MULTIPLE_CONNECTIONS_ALLOWED},
    {"single_connection_only", HELICS_HANDLE_OPTION_SINGLE_CONNECTION_ONLY},
    {"singleconnectiononly", HELICS_HANDLE_OPTION_SINGLE_CONNECTION_ONLY},
    {"singleConnectionOnly", HELICS_HANDLE_OPTION_SINGLE_CONNECTION_ONLY},
    {"only_transmit_on_change", HELICS_HANDLE_OPTION_ONLY_TRANSMIT_ON_CHANGE},
    {"onlytransmitonchange", HELICS_HANDLE_OPTION_ONLY_TRANSMIT_ON_CHANGE},
    {"onlyTransmitOnChange", HELICS_HANDLE_OPTION_ONLY_TRANSMIT_ON_CHANGE},
    {"only_update_on_change", HELICS_HANDLE_OPTION_ONLY_UPDATE_ON_CHANGE},
    {"onlyupdateonchange", HELICS_HANDLE_OPTION_ONLY_UPDATE_ON_CHANGE},
    {"onlyUpdateOnChange", HELICS_HANDLE_OPTION_ONLY_UPDATE_ON_CHANGE},
    {"ignore_unit_mismatch", HELICS_HANDLE_OPTION_IGNORE_UNIT_MISMATCH},
    {"ignoreunitmismatch", HELICS_HANDLE_OPTION_IGNORE_UNIT_MISMATCH},
    {"ignoreUnitMismatch", HELICS_HANDLE_OPTION_IGNORE_UNIT_MISMATCH},
    {"strict_input_type_checking", HELICS_HANDLE_OPTION_STRICT_TYPE_CHECKING},
    {"strictinputtypechecking", HELICS_HANDLE_OPTION_STRICT_TYPE_CHECKING},
    {"strictInputTypeChecking", HELICS_HANDLE_OPTION_STRICT_TYPE_CHECKING},
    {"connections", HELICS_HANDLE_OPTION_CONNECTIONS},
    {"clear_priority_list", HELICS_HANDLE_OPTION_CLEAR_PRIORITY_LIST},
    {"clearPriorityList", HELICS_HANDLE_OPTION_CLEAR_PRIORITY_LIST},
    {"clearprioritylist", HELICS_HANDLE_OPTION_CLEAR_PRIORITY_LIST},
    {"input_priority_location", HELICS_HANDLE_OPTION_INPUT_PRIORITY_LOCATION},
    {"inputprioritylocation", HELICS_HANDLE_OPTION_INPUT_PRIORITY_LOCATION},
    {"inputPriorityLocation", HELICS_HANDLE_OPTION_INPUT_PRIORITY_LOCATION},
    {"multi_input_handling_method", HELICS_HANDLE_OPTION_MULTI_INPUT_HANDLING_METHOD},
    {"multiinputhandlingmethod", HELICS_HANDLE_OPTION_MULTI_INPUT_HANDLING_METHOD},
    {"multiInputHandlingMethod", HELICS_HANDLE_OPTION_MULTI_INPUT_HANDLING_METHOD}};

static const std::map<std::string, int> option_value_map{
    {"0", 0},
    {"1", 1},
    {"-", 0},
    {"+", 1},
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
    {"none", HELICS_MULTI_INPUT_NO_OP},
    {"no_op", HELICS_MULTI_INPUT_NO_OP},
    {"and", HELICS_MULTI_INPUT_AND_OPERATION},
    {"or", HELICS_MULTI_INPUT_OR_OPERATION},
    {"sum", HELICS_MULTI_INPUT_SUM_OPERATION},
    {"max", HELICS_MULTI_INPUT_MAX_OPERATION},
    {"min", HELICS_MULTI_INPUT_MIN_OPERATION},
    {"average", HELICS_MULTI_INPUT_AVERAGE_OPERATION},
    {"mean", HELICS_MULTI_INPUT_AVERAGE_OPERATION},
    {"vectorize", HELICS_MULTI_INPUT_VECTORIZE_OPERATION},
    {"diff", HELICS_MULTI_INPUT_DIFF_OPERATION}};

static const std::map<std::string, int> log_level_map{{"none", HELICS_LOG_LEVEL_NO_PRINT},
                                                      {"no_print", HELICS_LOG_LEVEL_NO_PRINT},
                                                      {"error", HELICS_LOG_LEVEL_ERROR},
                                                      {"warning", HELICS_LOG_LEVEL_WARNING},
                                                      {"summary", HELICS_LOG_LEVEL_SUMMARY},
                                                      {"connections", HELICS_LOG_LEVEL_CONNECTIONS},
                                                      /** connections+ interface definitions*/
                                                      {"interfaces", HELICS_LOG_LEVEL_INTERFACES},
                                                      /** interfaces + timing message*/
                                                      {"timing", HELICS_LOG_LEVEL_TIMING},
                                                      {"profiling", HELICS_LOG_LEVEL_WARNING - 1},
                                                      /** timing+ data transfer notices*/
                                                      {"data", HELICS_LOG_LEVEL_DATA},
                                                      /** same as data for now*/
                                                      {"debug", HELICS_LOG_LEVEL_DEBUG},
                                                      /** all internal messages*/
                                                      {"trace", HELICS_LOG_LEVEL_TRACE}};

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
            continue;
        }
        if (flg == "json") {
            fi.useJsonSerialization = true;
            // purposely not continuing here so the setFlagOption gets called
        }
        if (flg == "profiling") {
            fi.profilerFileName = "log";
            // purposely not continuing here so the setFlagOption gets called
        }
        if (flg == "observer") {
            fi.observer = true;
            // purposely not continuing here so the setFlagOption gets called
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
    return HELICS_INVALID_OPTION_INDEX;
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
    return HELICS_INVALID_OPTION_INDEX;
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
    return HELICS_INVALID_OPTION_INDEX;
}

std::unique_ptr<helicsCLI11App> FederateInfo::makeCLIApp()
{
    /*
    The parser for the command line options ignores underscores and cases so
    there's no need to explicitly support all three case styles. If users type
    in any of the normal HELICS variants (nocase, camelCase, and snake_case) it
    will work.
    */
    auto app = std::make_unique<helicsCLI11App>("Federate Info Parsing");
    app->option_defaults()->ignore_case()->ignore_underscore();
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
              if (coreType == CoreType::UNRECOGNIZED) {
                  coreName = val;
              }
          },
          "type or name of the core to connect to")
        ->default_str("(" + to_string(coreType) + ")");
    og->add_flag("--force_new_core",
                 forceNewCore,
                 "if set to true will force the federate to generate a new core");
    og->add_option_function<std::string>(
          "--coretype,-t",
          [this](const std::string& val) {
              coreType = coreTypeFromString(val);
              if (coreType == CoreType::UNRECOGNIZED) {
                  throw CLI::ValidationError(val + " is NOT a recognized core type");
              }
          },
          "type  of the core to connect to")
        ->default_str("(" + to_string(coreType) + ")")
        ->envname("HELICS_CORE_TYPE");
    app->add_option("--corename", coreName, "the name of the core to create or find");
    app->add_option("--coreinitstring,-i",
                    coreInitString,
                    "The initialization arguments for the core")
        ->transform([](std::string arg) {
            arg.insert(arg.begin(), ' ');
            return arg;
        })
        ->multi_option_policy(CLI::MultiOptionPolicy::Join)
        ->envname("HELICS_CORE_INIT_STRING");
    app->add_option("--brokerinitstring",
                    brokerInitString,
                    "The initialization arguments for the broker if autogenerated")
        ->transform([](std::string arg) {
            arg.insert(arg.begin(), ' ');
            return arg;
        })
        ->multi_option_policy(CLI::MultiOptionPolicy::Join);
    app->add_option("--broker,--brokeraddress", broker, "address or name of the broker to connect");
    app->add_option("--brokerport", brokerPort, "Port number of the Broker")
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
    app->add_option("--localport",
                    localport,
                    "Port number to use for connections to this federate");
    app->add_flag("--autobroker",
                  autobroker,
                  "tell the core to automatically generate a broker if needed");
    app->add_flag("--debugging",
                  debugging,
                  "tell the core to allow user debugging in a nicer fashion");
    app->add_flag("--observer",
                  observer,
                  "tell the federate/core that this federate is an observer");
    app->add_flag(
        "--json",
        useJsonSerialization,
        "tell the core and federate to use JSON based serialization for all messages, to ensure compatibility");
    app->add_option(
           "--profiler",
           profilerFileName,
           "Enable profiling and specify a file name, \"log\" for sending profiling messages to the logger, otherwise specify the output file name")
        ->expected(0, 1)
        ->default_str("log");
    app->add_option("--broker_key,--brokerkey,--brokerKey",
                    key,
                    "specify a key to use to match a broker should match the broker key");
    app->add_option_function<Time>(
           "--offset",
           [this](Time val) { setProperty(HELICS_PROPERTY_TIME_OFFSET, val); },
           "the offset of the time steps (default in ms)")
        ->configurable(false);
    app->add_option_function<Time>(
           "--period",
           [this](Time val) { setProperty(HELICS_PROPERTY_TIME_PERIOD, val); },
           "the execution cycle of the federate (default in ms)")
        ->configurable(false);
    app->add_option_function<Time>(
           "--timedelta",
           [this](Time val) { setProperty(HELICS_PROPERTY_TIME_DELTA, val); },
           "The minimum time between time grants for a Federate (default in ms)")
        ->configurable(false);
    auto* rtgroup = app->add_option_group("realtime");
    rtgroup->option_defaults()->ignore_underscore();
    rtgroup
        ->add_option_function<Time>(
            "--rtlag",
            [this](Time val) { setProperty(HELICS_PROPERTY_TIME_RT_LAG, val); },
            "the amount of the time the federate is allowed to lag realtime before "
            "corrective action is taken (default in ms)")
        ->configurable(false);
    rtgroup
        ->add_option_function<Time>(
            "--rtlead",
            [this](Time val) { setProperty(HELICS_PROPERTY_TIME_RT_LEAD, val); },
            "the amount of the time the federate is allowed to lead realtime before "
            "corrective action is taken (default in ms)")
        ->configurable(false);
    rtgroup
        ->add_option_function<Time>(
            "--rttolerance",
            [this](Time val) { setProperty(HELICS_PROPERTY_TIME_RT_TOLERANCE, val); },
            "the time tolerance of the real time mode (default in ms)")
        ->configurable(false);

    app->add_option_function<Time>(
           "--inputdelay",
           [this](Time val) { setProperty(HELICS_PROPERTY_TIME_INPUT_DELAY, val); },
           "the INPUT delay on incoming communication of the federate (default in ms)")
        ->configurable(false);
    app->add_option_function<Time>(
           "--outputdelay",
           [this](Time val) { setProperty(HELICS_PROPERTY_TIME_OUTPUT_DELAY, val); },
           "the output delay for outgoing communication of the federate (default in ms)")
        ->configurable(false);
    app->add_option_function<Time>(
           "--grant_timeout",
           [this](Time val) { setProperty(HELICS_PROPERTY_TIME_GRANT_TIMEOUT, val); },
           "timeout to trigger diagnostic action when a federate time grant is not available with a the timeout (default in ms)")
        ->configurable(false);
    app->add_option_function<int>(
           "--maxiterations",
           [this](int val) { setProperty(HELICS_PROPERTY_INT_MAX_ITERATIONS, val); },
           "the maximum number of iterations a federate is allowed to take")
        ->check(CLI::PositiveNumber);
    app->add_option_function<int>(
           "--loglevel",
           [this](int val) { setProperty(HELICS_PROPERTY_INT_LOG_LEVEL, val); },
           "the logging level of a federate")
        ->transform(
            CLI::CheckedTransformer(&log_level_map, CLI::ignore_case, CLI::ignore_underscore))

        ->transform(CLI::IsMember(&log_level_map, CLI::ignore_case, CLI::ignore_underscore))
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
        if (fileops::hasTomlExtension(configString)) {
            loadInfoFromToml(configString, false);
            fileInUse = configString;
        } else if (fileops::hasJsonExtension(configString)) {
            loadInfoFromJson(configString, false);
            fileInUse = configString;
        }
    }
}

FederateInfo loadFederateInfo(const std::string& configString)
{
    FederateInfo ret;

    if (fileops::hasTomlExtension(configString)) {
        ret.loadInfoFromToml(configString);
        ret.fileInUse = configString;
    } else if (fileops::hasJsonExtension(configString)) {
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
        doc = fileops::loadJson(jsonString);
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
        fileops::callIfMember(doc, prop.first, timeCall);
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
        doc = fileops::loadToml(tomlString);
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
        fileops::callIfMember(doc, prop.first, timeCall);
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
    if (fi.observer) {
        res.append(" --observer");
    }
    if (fi.useJsonSerialization) {
        res.append(" --json");
    }
    if (!fi.profilerFileName.empty()) {
        res.append(" --profiler=");
        res.append(fi.profilerFileName);
    }
    if (!fi.brokerInitString.empty()) {
        res.append(" --broker_init_string \"");
        res.append(fi.brokerInitString);
        res.append("\"");
    }
    if (!fi.key.empty()) {
        res += " --broker_key=";
        res.append(fi.key);
    }
    if (!fi.fileInUse.empty()) {  // we used the file, specify a core section
        res += " --config_section=core --config-file=";
        res.append(fi.fileInUse);
    }
    return res;
}

}  // namespace helics
