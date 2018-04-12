/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include "helicsApp.hpp"
#include "PrecHelper.hpp"

#include <fstream>
#include <iostream>

#include <boost/filesystem.hpp>
#include "../common/argParser.h"

#include "../common/JsonProcessingFunctions.hpp"

#include "../common/stringOps.h"
#include "../core/helicsVersion.hpp"

namespace filesystem = boost::filesystem;

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

namespace helics
{
namespace apps
{

static const ArgDescriptors InfoArgs{
    {"local", ArgDescriptor::arg_type_t::flag_type, "specify otherwise unspecified endpoints and publications as local( i.e.the keys will be prepended with the player name"},
    {"stop",  "the time to stop the player"}
};

App::App (const std::string &appName,int argc, char *argv[])
{
    variable_map vm_map;
    auto res = argumentParser(argc, argv, vm_map, InfoArgs,"input");
    if (res == versionReturn)
    {
        std::cout << helics::versionString<< '\n';
    }
    if (res == helpReturn)
    {
        FederateInfo helpTemp(argc, argv);
    }
    if (res < 0)
    {
        deactivated = true;
        return;
    }
    FederateInfo fi (appName);
    fi.loadInfoFromArgs (argc, argv);
    fed = std::make_shared<CombinationFederate> (fi);
    App::loadArguments(vm_map);

}

App::App (const FederateInfo &fi) : fed (std::make_shared<CombinationFederate> (fi))
{
}

App::App (const std::shared_ptr<Core> &core, const FederateInfo &fi)
    : fed (std::make_shared<CombinationFederate> (core, fi))
{

}

App::App (const std::string &name, const std::string &jsonString) : fed ( std::make_shared<CombinationFederate> (name,jsonString))
{
    
    if (jsonString.size () < 200)
    {
        masterFileName = jsonString;
    }
}

App::~App () = default;


void App::loadFile (const std::string &filename)
{
    auto ext = filesystem::path (filename).extension ().string ();
    if ((ext == ".json") || (ext == ".JSON"))
    {
        loadJsonFile (filename);
    }
    else
    {
        loadTextFile (filename);
    }
}

void App::loadTextFile (const std::string &filename)
{
    using namespace stringOps;
    std::ifstream infile (filename);
    std::string str;

    // count the lines
    while (std::getline (infile, str))
    {
        if (str.empty ())
        {
            continue;
        }
        auto fc = str.find_first_not_of (" \t\n\r\0");
        if ((fc == std::string::npos) || (str[fc] == '#'))
        {
            continue;
        }
        if (str[fc] == '!')
        {

        }
       
    }
   
}

void App::loadJsonFile (const std::string &jsonString)
{
    loadJsonFileConfiguration("application", jsonString);
}


void App::loadJsonFileConfiguration(const std::string &appName, const std::string &jsonString)
{
    fed->registerInterfaces(jsonString);


    auto doc = loadJsonString(jsonString);


    if (doc.isMember("app"))
    {
        auto appConfig = doc["app"];
        loadConfigOptions(appConfig);
    }
    if (doc.isMember("config"))
    {
        auto appConfig = doc["config"];
        loadConfigOptions(appConfig);
    }
    if (doc.isMember(appName))
    {
        auto appConfig = doc[appName];
        loadConfigOptions(appConfig);
    }
}

void App::loadConfigOptions(const Json_helics::Value &element)
{
    if (element.isMember("stop"))
    {
        stopTime = loadJsonTime(element["stop"]);
    }
    if (element.isMember("local"))
    {
        useLocal = element["local"].asBool();
    }
    if (element.isMember("file"))
    {
        if (element["file"].isArray())
        {
            for (decltype(element.size()) ii = 0; ii<element.size(); ++ii)
            {
                loadFile(element["file"][ii].asString());
            }
        }
        else
        {
            loadFile(element["file"].asString());
        }
    }
}
void App::initialize ()
{
    auto state = fed->getCurrentState ();
    if (state == Federate::op_states::startup)
    {
        fed->enterInitializationState ();
    }
}


void App::finalize()
{
    fed->finalize();
}

/*run the App*/
void App::run ()
{
    runTo (stopTime);
    fed->disconnect();
}

int App::loadArguments(boost::program_options::variables_map &vm_map)
{

    if (vm_map.count("local"))
    {
        useLocal = true;
    }
    if (vm_map.count("input") == 0)
    {
        if (!fileLoaded)
        {
            if (filesystem::exists("helics.json"))
            {
                masterFileName = "helics.json";
            }
        }
    }
    else if (filesystem::exists (vm_map["input"].as<std::string> ()))
    {
        masterFileName = vm_map["input"].as<std::string>();
    }

    if (vm_map.count ("stop") > 0)
    {
        stopTime = loadTimeFromString(vm_map["stop"].as<std::string> ());
    }
    return 0;
}

}  // namespace apps
} // namespace helics

