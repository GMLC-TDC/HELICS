/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "helicsApp.hpp"

#include "../common/JsonProcessingFunctions.hpp"
#include "../core/helicsCLI11.hpp"
#include "../core/helicsVersion.hpp"
#include "PrecHelper.hpp"
#include "gmlc/utilities/stringOps.h"

#include <fstream>
#include <iostream>
#include <memory>
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

namespace helics {
namespace apps {
    App::App(const std::string& defaultAppName, std::vector<std::string> args)
    {
        auto app = generateParser();
        app->helics_parse(std::move(args));
        processArgs(app, defaultAppName);
    }

    App::App(const std::string& defaultAppName, int argc, char* argv[])
    {
        auto app = generateParser();
        app->helics_parse(argc, argv);
        processArgs(app, defaultAppName);
    }

    void App::processArgs(std::unique_ptr<helicsCLI11App>& app, const std::string& defaultAppName)
    {
        remArgs = app->remaining_for_passthrough();
        auto ret = app->last_output;
        if (ret == helicsCLI11App::parse_output::help_call) {
            if (!app->quiet) {
                // this is just to run the help output
                FederateInfo helpTemp("--help");
                (void)helpTemp;
            }
            helpMode = true;
        }
        if (ret != helicsCLI11App::parse_output::ok) {
            deactivated = true;
            return;
        }

        if (masterFileName.empty()) {
            if (!fileLoaded) {
                if (CLI::ExistingFile("helics.json").empty()) {
                    masterFileName = "helics.json";
                }
            }
        }

        FederateInfo fi(remArgs);
        if (fi.defName.empty()) {
            fi.defName = defaultAppName;
        }

        fed = std::make_shared<CombinationFederate>("", fi);
    }

    App::App(const std::string& appName, const FederateInfo& fi):
        fed(std::make_shared<CombinationFederate>(appName, fi))
    {
    }

    App::App(const std::string& appName, const std::shared_ptr<Core>& core, const FederateInfo& fi):
        fed(std::make_shared<CombinationFederate>(appName, core, fi))
    {
    }

    App::App(const std::string& appName, CoreApp& core, const FederateInfo& fi):
        fed(std::make_shared<CombinationFederate>(appName, core, fi))
    {
    }

    App::App(const std::string& appName, const std::string& jsonString):
        fed(std::make_shared<CombinationFederate>(appName, jsonString))
    {
        if (jsonString.size() < 200) {
            masterFileName = jsonString;
        }
    }

    App::~App() = default;

    std::unique_ptr<helicsCLI11App> App::generateParser()
    {
        auto app =
            std::make_unique<helicsCLI11App>("common options for all Helics Apps", "[HELICS_APP]");

        app->add_flag("--local",
                      useLocal,
                      "specify otherwise unspecified endpoints and publications as local( "
                      "i.e.the keys will be prepended with the player name)");
        app->add_option("--stop", stopTime, "The time to stop the app");
        app->add_option("--input,input", masterFileName, "The primary input file")
            ->check(CLI::ExistingFile);
        app->allow_extras()->validate_positionals();
        return app;
    }

    void App::loadFile(const std::string& filename)
    {
        auto ext = filename.substr(filename.find_last_of('.'));
        if ((ext == ".json") || (ext == ".JSON")) {
            loadJsonFile(filename);
        } else {
            loadTextFile(filename);
        }
    }

    void App::loadTextFile(const std::string& textFile)
    {
        // using namespace gmlc::utilities::stringOps;
        std::ifstream infile(textFile);
        std::string str;

        // count the lines
        while (std::getline(infile, str)) {
            if (str.empty()) {
                continue;
            }
            auto fc = str.find_first_not_of(" \t\n\r\0");
            if ((fc == std::string::npos) || (str[fc] == '#')) {
                continue;
            }
            if (str[fc] == '!') {
            }
        }
    }

    void App::loadJsonFile(const std::string& jsonString)
    {
        loadJsonFileConfiguration("application", jsonString);
    }

    void App::loadJsonFileConfiguration(const std::string& appName, const std::string& jsonString)
    {
        fed->registerInterfaces(jsonString);

        auto doc = loadJson(jsonString);

        if (doc.isMember("app")) {
            auto appConfig = doc["app"];
            loadConfigOptions(appConfig);
        }
        if (doc.isMember("config")) {
            auto appConfig = doc["config"];
            loadConfigOptions(appConfig);
        }
        if (doc.isMember(appName)) {
            auto appConfig = doc[appName];
            loadConfigOptions(appConfig);
        }
    }

    void App::loadConfigOptions(const Json::Value& element)
    {
        if (element.isMember("stop")) {
            stopTime = loadJsonTime(element["stop"]);
        }
        if (element.isMember("local")) {
            useLocal = element["local"].asBool();
        }
        if (element.isMember("file")) {
            if (element["file"].isArray()) {
                for (decltype(element.size()) ii = 0; ii < element.size(); ++ii) {
                    loadFile(element["file"][ii].asString());
                }
            } else {
                loadFile(element["file"].asString());
            }
        }
    }
    void App::initialize()
    {
        auto md = fed->getCurrentMode();
        if (md == Federate::modes::startup) {
            fed->enterInitializingMode();
        }
    }

    void App::finalize() { fed->finalize(); }

    /*run the App*/
    void App::run()
    {
        runTo(stopTime);
        fed->disconnect();
    }

}  // namespace apps
}  // namespace helics
