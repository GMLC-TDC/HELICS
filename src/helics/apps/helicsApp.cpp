/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "helicsApp.hpp"

#include "../common/JsonProcessingFunctions.hpp"
#include "../common/TomlProcessingFunctions.hpp"
#include "../core/helicsCLI11.hpp"
#include "../core/helicsVersion.hpp"
#include "PrecHelper.hpp"
#include "gmlc/utilities/stringOps.h"

#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
// static const std::regex creg
// (R"raw((-?\d+(\.\d+)?|\.\d+)[\s,]*([^\s]*)(\s+[cCdDvVsSiIfF]?\s+|\s+)([^\s]*))raw");

/*
std::shared_ptr<CombinationFederate> fed;
std::vector<ValueSetter> points;
std::set<std::pair<std::string, std::string>> tags;
std::vector<Publication> publications;
std::vector<Endpoint> endpoints;
std::map<std::string, int> pubids;
std::map<std::string, int> eptids;
*/

namespace helics::apps {
App::App(std::string_view defaultAppName, std::vector<std::string> args)
{
    auto app = generateParser();
    FederateInfo fedInfo;
    fedInfo.injectParser(app.get());
    app->helics_parse(std::move(args));
    processArgs(app, fedInfo, defaultAppName);
}

App::App(std::string_view defaultAppName, int argc, char* argv[])
{
    auto app = generateParser();
    FederateInfo fedInfo;
    fedInfo.injectParser(app.get());
    app->helics_parse(argc, argv);
    processArgs(app, fedInfo, defaultAppName);
}

void App::processArgs(std::unique_ptr<helicsCLI11App>& app,
                      FederateInfo& fedInfo,
                      std::string_view defaultAppName)
{
    remArgs = app->remaining_for_passthrough();
    auto ret = app->last_output;
    if (ret == helicsCLI11App::ParseOutput::HELP_CALL) {
        helpMode = true;
    }
    if (ret != helicsCLI11App::ParseOutput::OK) {
        deactivated = true;
        return;
    }

    if (inputFileName.empty()) {
        if (!fileLoaded) {
            if (CLI::ExistingFile("helics.json").empty()) {
                inputFileName = "helics.json";
            }
        }
    }

    if (fedInfo.defName.empty()) {
        fedInfo.defName = defaultAppName;
    }

    fed = std::make_shared<CombinationFederate>("", fedInfo);
    configFileName = fed->getConfigFile();
}

App::App(std::string_view appName, const FederateInfo& fedInfo):
    fed(std::make_shared<CombinationFederate>(appName, fedInfo))
{
    configFileName = fed->getConfigFile();
}

App::App(std::string_view appName, const std::shared_ptr<Core>& core, const FederateInfo& fedInfo):
    fed(std::make_shared<CombinationFederate>(appName, core, fedInfo))
{
    configFileName = fed->getConfigFile();
}

App::App(std::string_view appName, CoreApp& core, const FederateInfo& fedInfo):
    fed(std::make_shared<CombinationFederate>(appName, core, fedInfo))
{
    configFileName = fed->getConfigFile();
}

App::App(std::string_view appName, const std::string& configString):
    fed(std::make_shared<CombinationFederate>(appName, configString))
{
    configFileName = fed->getConfigFile();
}

App::~App() = default;

std::unique_ptr<helicsCLI11App> App::generateParser()
{
    auto app =
        std::make_unique<helicsCLI11App>("Common options for all Helics Apps", "[HELICS_APP]");

    app->add_flag("--local",
                  useLocal,
                  "Specify otherwise unspecified endpoints and publications as local "
                  "(i.e. the names will be prepended with the app name)");
    app->add_option("--stop", stopTime, "The time to stop the app");
    app->add_option("--input,input",
                    inputFileName,
                    "The primary input file containing app configuration")
        ->check(CLI::ExistingFile);
    app->allow_extras()->validate_positionals();
    return app;
}

void App::loadFile(const std::string& filename, bool enableFederateInterfaceRegistration)
{
    if (fileops::hasJsonExtension(filename)) {
        loadJsonFile(filename, enableFederateInterfaceRegistration);
    } else if (fileops::hasTomlExtension(filename)) {
        if (enableFederateInterfaceRegistration) {
            fed->registerInterfaces(filename);
        } else {
            fed->logWarningMessage("Toml files are not support for app configuration");
        }
    } else {
        loadTextFile(filename);
    }
}

AppTextParser::AppTextParser(const std::string& filename): filePtr(filename), mFileName(filename) {}

std::vector<int> AppTextParser::preParseFile(const std::vector<char>& klines)
{
    reset();
    std::vector<int> counts(1 + klines.size());
    std::string str;
    bool inMline{false};
    // count the lines
    while (std::getline(filePtr, str)) {
        if (str.empty()) {
            continue;
        }
        auto firstChar = str.find_first_not_of(" \t\n\r\0");
        if (firstChar == std::string::npos) {
            continue;
        }
        if (inMline) {
            if (firstChar + 2 < str.size()) {
                if ((str[firstChar] == '#') && (str[firstChar + 1] == '#') &&
                    (str[firstChar + 2] == ']')) {
                    inMline = false;
                }
            }
            continue;
        }
        if (str[firstChar] == '#') {
            if (firstChar + 2 < str.size()) {
                if ((str[firstChar + 1] == '#') && (str[firstChar + 2] == '[')) {
                    inMline = true;
                }
            }
            continue;
        }
        if (str[firstChar] == '!') {
            configStr += str.substr(firstChar + 1);
            configStr.push_back('\n');
            continue;
        }

        ++counts[0];
        for (std::size_t ii = 0; ii < klines.size(); ++ii) {
            if (str[firstChar] == klines[ii]) {
                ++counts[ii + 1];
            }
        }
    }
    return counts;
}

bool AppTextParser::loadNextLine(std::string& line, int& lineNumber)
{
    while (std::getline(filePtr, line)) {
        ++currentLineNumber;
        if (line.empty()) {
            continue;
        }
        auto firstChar = line.find_first_not_of(" \t\n\r\0");
        if (firstChar == std::string::npos) {
            continue;
        }
        if (mLineComment) {
            if (firstChar + 2 < line.size()) {
                if ((line[firstChar] == '#') && (line[firstChar + 1] == '#') &&
                    (line[firstChar + 2] == ']')) {
                    mLineComment = false;
                }
            }
            continue;
        }
        if (line[firstChar] == '#') {
            if (firstChar + 2 < line.size()) {
                if ((line[firstChar + 1] == '#') && (line[firstChar + 2] == '[')) {
                    mLineComment = true;
                }
            }
            continue;
        }
        if (line[firstChar] == '!') {
            continue;
        }
        lineNumber = currentLineNumber;
        return true;
    }
    return false;
}

void AppTextParser::reset()
{
    filePtr.close();
    filePtr.open(mFileName);
    mLineComment = false;
}

void App::loadConfigOptions(AppTextParser& aparser)
{
    const auto& configStr = aparser.configString();
    if (!configStr.empty()) {
        auto app = generateParser();
        std::istringstream sstr(configStr);
        app->parse_from_stream(sstr);
    }
}

void App::loadTextFile(const std::string& textFile)
{
    AppTextParser aparser(textFile);
    aparser.preParseFile({});
    loadConfigOptions(aparser);
}

void App::loadInputFiles()
{
    if (!configFileName.empty()) {
        /** this one would have been loaded through the federate already*/
        loadFile(configFileName, false);
    }
    if (!inputFileName.empty()) {
        loadFile(inputFileName, true);
    }
}

void App::loadJsonFile(const std::string& jsonString, bool enableFederateInterfaceRegistration)
{
    loadJsonFileConfiguration("application", jsonString, enableFederateInterfaceRegistration);
}

void App::loadJsonFileConfiguration(const std::string& appName,
                                    const std::string& jsonString,
                                    bool enableFederateInterfaceRegistration)
{
    if (enableFederateInterfaceRegistration) {
        fed->registerInterfaces(jsonString);
    }
    auto doc = fileops::loadJson(jsonString);

    if (doc.contains("app")) {
        auto& appConfig = doc["app"];
        loadConfigOptions(appConfig);
    }
    if (doc.contains("config")) {
        auto& appConfig = doc["config"];
        loadConfigOptions(appConfig);
    }
    if (doc.contains(appName)) {
        auto& appConfig = doc[appName];
        loadConfigOptions(appConfig);
    }
}

void App::loadConfigOptions(const fileops::JsonBuffer& elementBuff)
{
    const auto& element = elementBuff.json();
    if (element.contains("stop")) {
        stopTime = fileops::loadJsonTime(element["stop"]);
    }
    if (element.contains("local")) {
        useLocal = element["local"].get<bool>();
    }
    if (element.contains("file")) {
        if (element["file"].is_array()) {
            for (decltype(element.size()) ii = 0; ii < element.size(); ++ii) {
                loadFile(element["file"][ii].get<std::string>());
            }
        } else {
            loadFile(element["file"].get<std::string>());
        }
    }
}
void App::initialize()
{
    auto currentMode = fed->getCurrentMode();
    if (currentMode == Federate::Modes::STARTUP) {
        fed->enterInitializingMode();
    }
}

void App::finalize()
{
    fed->finalize();
}

/*run the App*/
void App::run()
{
    runTo(stopTime);
    fed->disconnect();
}

}  // namespace helics::apps
