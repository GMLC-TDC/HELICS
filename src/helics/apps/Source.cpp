/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include "Source.hpp"
#include "PrecHelper.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <stdexcept>
#include <boost/filesystem.hpp>
#include "../common/argParser.h"

#include "../common/JsonProcessingFunctions.hpp"

#include "../common/base64.h"
#include "../common/stringOps.h"
#include "../core/helicsVersion.hpp"
#include "../core/core-exceptions.hpp"
#include "SignalGenerators.hpp"

namespace filesystem = boost::filesystem;

namespace helics
{
namespace apps
{

void SignalGenerator::set(const std::string & /*parameter*/, double /*val*/)
{

}
/** set a string parameter*/
void SignalGenerator::setString(const std::string & /*parameter*/, const std::string &/*val*/)
{

}

using namespace std::string_literals;
static const ArgDescriptors InfoArgs{
    { "stop", "the time to stop recording" },
    {"default_period","the default period publications"}
};

Source::Source (int argc, char *argv[])
{

    variable_map vm_map;
    auto res = argumentParser(argc, argv, vm_map, InfoArgs, "input"s);
    if (res == versionReturn)
    {
        std::cout << helics::versionString << '\n';
    }
    if (res < 0)
    {
        deactivated = true;
        return;
    }
    FederateInfo fi("source");

    fi.loadInfoFromArgs(argc, argv);
    fed = std::make_shared<CombinationFederate>(fi);
    fed->setFlag(SOURCE_ONLY_FLAG);

    loadArguments(vm_map);
}

Source::Source (const FederateInfo &fi) : fed (std::make_shared<CombinationFederate> (fi))
{
    fed->setFlag (SOURCE_ONLY_FLAG);
}

Source::Source (const std::shared_ptr<Core> &core, const FederateInfo &fi)
    : fed (std::make_shared<CombinationFederate> (core, fi))
{
    fed->setFlag (SOURCE_ONLY_FLAG);
}

Source::Source (const std::string &jsonString) : fed (std::make_shared<CombinationFederate> (jsonString))
{
    fed->setFlag (SOURCE_ONLY_FLAG);

    loadJsonFile (jsonString);
}

Source::~Source () = default;


void Source::finalize()
{
    fed->finalize();
}

void Source::loadFile (const std::string &filename)
{
    
    auto ext = filesystem::path(filename).extension().string();
    if ((ext == ".json") || (ext == ".JSON"))
    {
        loadJsonFile(filename);
    }
    else
    {
        // loadTextFile(filename);
    }

}


void Source::loadJsonFile(const std::string &jsonFile)
{
    fed->registerInterfaces(jsonFile);

    auto pubCount = fed->getPublicationCount();
    for (int ii = 0; ii < pubCount; ++ii)
    {
        SourceObject newObj;

        newObj.pub = Publication(fed.get(), ii);
        newObj.period = defaultPeriod;
        sources.push_back(newObj);
        pubids[newObj.pub.getKey()] = static_cast<int> (sources.size()) - 1;

    }
   /* auto eptCount = fed->getEndpointCount();
    for (int ii = 0; ii < eptCount; ++ii)
    {
        endpoints.emplace_back(fed.get(), ii);
        eptids[endpoints.back().getName()] = static_cast<int> (endpoints.size() - 1);
    }
    */
    auto doc = loadJsonString(jsonFile);


    if (doc.isMember("source"))
    {
        auto playerConfig = doc["source"];
        if (playerConfig.isMember("stop"))
        {
            stopTime = loadJsonTime(playerConfig["stop"]);
        }
        if (playerConfig.isMember("local"))
        {
            useLocal = playerConfig["local"].asBool();
        }
    }
    auto genArray = doc["generators"];
    if (genArray.isArray())
    {
        
    }
}

/*run the source*/
void Source::run ()
{
    run (stopTime);
    fed->finalize ();
}

void Source::run (Time stopTime_input)
{
    auto state = fed->getCurrentState ();
    if (state == Federate::op_states::startup)
    {
        initialize ();
    }
    Time nextRequestTime = Time::maxVal();
    Time currentTime;
    if (state != Federate::op_states::execution)
    {
        // send stuff before timeZero

        runSourceLoop(timeZero - timeEpsilon);

        fed->enterExecutionState ();
        // send the stuff at timeZero
        nextRequestTime = runSourceLoop(timeZero);
        currentTime = timeZero;
    }
    else
    {
       currentTime = fed->getCurrentTime ();
       
       for (auto &src : sources)
       {
           if (src.nextTime < nextRequestTime)
           {
               nextRequestTime = src.nextTime;
           }
       }
    }
    helics::Time nextPrintTime = currentTime+10.0;
    while ((nextRequestTime < Time::maxVal()) && (nextRequestTime <= stopTime_input))
    {
        currentTime = fed->requestTime(nextRequestTime);
        nextRequestTime = runSourceLoop(currentTime);
        if (currentTime >= nextPrintTime)
        {
            std::cout << "processed time " << static_cast<double> (currentTime) << "\n";
            nextPrintTime += 10.0;
        }
    }
   
}

void Source::addPublication(const std::string  &key, helics_type_t type, Time period, const std::string &units)
{
    // skip already existing publications
    if (pubids.find (key) != pubids.end ())
    {
        std::cerr << "publication already exists\n";
        return;
    }
    SourceObject newObj;
    
    newObj.pub=Publication (useLocal?LOCAL:GLOBAL, fed.get (), key, type, units);
    newObj.period = period;
    sources.push_back(newObj);
    pubids[key] = static_cast<int> (sources.size ()) - 1;
}

int Source::addSignalGenerator(const std::string & name, const std::string &type)
{
    std::shared_ptr<SignalGenerator> gen;
    if (type == "sine")
    {
        gen = std::make_shared<SineGenerator>();
    }
    else if (type == "ramp")
    {
        gen = std::make_shared<RampGenerator>();
    }
    else if ((type == "oscillator") || (type == "phasor"))
    {
        gen = std::make_shared<PhasorGenerator>();
    }
    generators.push_back(std::move(gen));
    int index = static_cast<int>(generators.size() - 1);
    generatorIndex.emplace(name, index);
    return index;
}

std::shared_ptr<SignalGenerator> Source::getGenerator(int index)
{

    if (index<static_cast<int>(generators.size()))
    {
        return generators[index];
    }
    return nullptr;
}

/** set the start time for a publication */
void Source::setStartTime(const std::string &key, Time startTime)
{
    auto fnd = pubids.find(key);
    if (fnd != pubids.end())
    {
        sources[fnd->second].nextTime = startTime;
    }
}
/** set the start time for a publication */
void Source::setPeriod(const std::string &key, Time period)
{
    auto fnd = pubids.find(key);
    if (fnd != pubids.end())
    {
        sources[fnd->second].period = period;
    }
}

/** tie a publication to a signal generator*/
void Source::linkPublicationToGenerator(const std::string &key, const std::string & generator)
{
    auto fnd = pubids.find(key);
    if (fnd != pubids.end())
    {
        auto findGen = generatorIndex.find(generator);
        if (findGen != generatorIndex.end())
        {
            sources[fnd->second].generatorIndex = findGen->second;
            return;
        }
        //only get here if something wasn't found
        throw(InvalidParameter(generator +" did not name a valid generator"));
    }
    //only get here if something wasn't found
    throw(InvalidParameter(key+" was not recognized as a valid publication"));
}

/** tie a publication to a signal generator*/
void Source::linkPublicationToGenerator(const std::string &key, int genIndex)
{
    auto fnd = pubids.find(key);
    if (fnd != pubids.end())
    {
       if (genIndex<static_cast<int>(generators.size()))
        {
            sources[fnd->second].generatorIndex = genIndex;
            return;
        }
       throw(InvalidParameter("generator index was invalid"));
    }
    //only get here if something wasn't found
    throw(InvalidParameter(key + " was not recognized as a valid publication"));
}


Time Source::runSource(SourceObject &obj, Time currentTime)
{
    if (currentTime >= obj.nextTime)
    {
        if (obj.generatorIndex >= static_cast<int>(generators.size()))
        {
            return Time::maxVal();
        }
        auto val=generators[obj.generatorIndex]->generate(currentTime);
        obj.pub.publish(val);
        obj.nextTime += obj.period;
        if (obj.nextTime < currentTime)
        {
            auto periods = std::floor( (currentTime - obj.nextTime) / obj.period);
            obj.nextTime += periods * obj.period + obj.period;
        }
    }
    return obj.nextTime;
}

Time Source::runSourceLoop(Time currentTime)
{
    if (currentTime < timeZero)
    {
        for (auto &src : sources)
        {
            if (src.nextTime < timeZero)
            {
                runSource(src, currentTime);
                src.nextTime = timeZero;
            }
        }
        return timeZero;
    }
    else
    {
        Time minTime = Time::maxVal();
        for (auto &src : sources)
        {
            auto tm = runSource(src, currentTime);
            if (tm < minTime)
            {
                minTime = tm;
            }
        }
        return minTime;
    }
}
int Source::loadArguments (boost::program_options::variables_map &vm_map)
{
    std::string file;
    if (vm_map.count("input") == 0)
    {
        if (!fileLoaded)
        {
            if (filesystem::exists("helics.json"))
            {
                file = "helics.json";
            }
        }
    }
    if (filesystem::exists(vm_map["input"].as<std::string>()))
    {
        file = vm_map["input"].as<std::string>();
    }
    if (!file.empty())
    {
        loadFile(file);
    }

    if (vm_map.count("stop") > 0)
    {
        stopTime = loadTimeFromString(vm_map["stop"].as<std::string>());
    }

 //   std::cout << "read file " << points.size () << " points for " << tags.size () << " tags \n";

    if (vm_map.count("default_period") == 0)
    {
        defaultPeriod = loadTimeFromString(vm_map["default_period"].as<std::string>());
    }
    return 0;
}

}  // namespace apps
} // namespace helics

