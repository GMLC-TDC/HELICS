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

#include "../common/argParser.h"

#include "../common/JsonProcessingFunctions.hpp"

#include "../common/stringOps.h"
#include "../core/core-exceptions.hpp"
#include "../core/helicsVersion.hpp"
#include "SignalGenerators.hpp"

namespace helics
{
namespace apps
{
void SignalGenerator::set (const std::string & /*parameter*/, double /*val*/) {}
/** set a string parameter*/
void SignalGenerator::setString (const std::string & /*parameter*/, const std::string & /*val*/) {}

using namespace std::string_literals;
static const ArgDescriptors InfoArgs{{"default_period", "the default period publications"}};

Source::Source (int argc, char *argv[]) : App ("source", argc, argv)
{
    variable_map vm_map;
    if (!deactivated)
    {
        fed->setFlag (SOURCE_ONLY_FLAG);
        argumentParser (argc, argv, vm_map, InfoArgs, "input"s);
        loadArguments (vm_map);
        if (!masterFileName.empty ())
        {
            loadFile (masterFileName);
        }
    }
    else
    {
        argumentParser (argc, argv, vm_map, InfoArgs);
    }
}

Source::Source (const FederateInfo &fi) : App (fi) { fed->setFlag (SOURCE_ONLY_FLAG); }

Source::Source (const std::shared_ptr<Core> &core, const FederateInfo &fi) : App (core, fi)
{
    fed->setFlag (SOURCE_ONLY_FLAG);
}

Source::Source (const std::string &name, const std::string &jsonString) : App (name, jsonString)
{
    fed->setFlag (SOURCE_ONLY_FLAG);

    Source::loadJsonFile (jsonString);
}

static void setGeneratorProperty (SignalGenerator *gen, const std::string &name, const Json_helics::Value &prop)
{
    if (prop.isDouble ())
    {
        gen->set (name, prop.asDouble ());
    }
    else
    {
        try
        {
            auto time = loadJsonTime (prop);
            if (time > Time::minVal ())
            {
                gen->set (name, static_cast<double> (time));
            }
            else
            {
                gen->setString (name, prop.asString ());
            }
        }
        catch (const std::invalid_argument &)
        {
            gen->setString (name, prop.asString ());
        }
    }
}

void Source::loadJsonFile (const std::string &jsonFile)
{
    // we want to load the default period before constructing the interfaces so the default period works
    auto doc = loadJson (jsonFile);

    if (doc.isMember ("source"))
    {
        auto appConfig = doc["source"];
        if (appConfig.isMember ("defaultperiod"))
        {
            defaultPeriod = loadJsonTime (appConfig["defaultperiod"]);
        }
    }

    loadJsonFileConfiguration ("source", jsonFile);
    auto pubCount = fed->getPublicationCount ();
    for (int ii = 0; ii < pubCount; ++ii)
    {
        SourceObject newObj;

        newObj.pub = Publication (fed, ii);
        newObj.period = defaultPeriod;
        sources.push_back (newObj);
        pubids[newObj.pub.getKey ()] = static_cast<int> (sources.size ()) - 1;
    }
    /* auto eptCount = fed->getEndpointCount();
     for (int ii = 0; ii < eptCount; ++ii)
     {
         endpoints.emplace_back(fed.get(), ii);
         eptids[endpoints.back().getName()] = static_cast<int> (endpoints.size() - 1);
     }
     */

    if (doc.isMember ("publications"))
    {
        auto pubArray = doc["publications"];

        for (const auto &pubElement : pubArray)
        {
            auto key = getKey (pubElement);
            if (pubElement.isMember ("start"))
            {
                setStartTime (key, loadJsonTime (pubElement["start"]));
            }
            if (pubElement.isMember ("period"))
            {
                setPeriod (key, loadJsonTime (pubElement["period"]));
            }
            if (pubElement.isMember ("generator"))
            {
                if (pubElement["generator"].isInt ())
                {
                    linkPublicationToGenerator (key, pubElement["generator"].asInt ());
                }
                else
                {
                    linkPublicationToGenerator (key, pubElement["generator"].asString ());
                }
            }
        }
    }
    if (doc.isMember ("generators"))
    {
        auto genArray = doc["generators"];
        for (const auto &genElement : genArray)
        {
            auto key = getKey (genElement);
            auto type = genElement["type"];
            int index = -1;
            if (!type.isNull ())
            {
                index = addSignalGenerator (key, type.asString ());
            }
            else
            {
                std::cout << "generator " << key << " does not specify a type\n";
                continue;
            }
            auto mnames = genElement.getMemberNames ();
            for (auto &el : mnames)
            {
                if ((el == "type") || (el == "name") || (el == "key"))
                {
                    continue;
                }
                if (el == "properties")
                {
                    for (auto &prop : genElement["properties"])
                    {
                        if ((prop.isMember ("name")) && (prop.isMember ("value")))
                        {
                            setGeneratorProperty (generators[index].get (), prop["name"].asString (),
                                                  prop["value"]);
                        }
                    }
                }
                else
                {
                    setGeneratorProperty (generators[index].get (), el, genElement[el]);
                }
            }
        }
    }
}

void Source::initialize ()
{
    auto state = fed->getCurrentState ();
    if (state != Federate::op_states::startup)
    {
        return;
    }

    int ii = 0;
    for (auto &src : sources)
    {
        if (src.generatorIndex < 0)
        {
            if (!src.generatorName.empty ())
            {
                auto fnd = generatorLookup.find (src.generatorName);
                if (fnd != generatorLookup.end ())
                {
                    src.generatorIndex = fnd->second;
                }
                else
                {
                    std::cout << "unable to link to signal generator " << src.generatorName << std::endl;
                    src.nextTime = Time::maxVal ();
                    src.generatorIndex = 0;
                }
            }
            else
            {
                if (ii < static_cast<int> (generators.size ()))
                {
                    src.generatorIndex = ii;
                }
                else
                {
                    src.generatorIndex = 0;
                }
            }
        }
        else
        {
            if (src.generatorIndex >= static_cast<int> (generators.size ()))
            {
                std::cerr << "invalid generator index for " << src.pub.getKey () << "disabling output\n";
                src.nextTime = Time::maxVal ();
            }
        }
    }

    fed->enterInitializationState ();
}

void Source::runTo (Time stopTime_input)
{
    auto state = fed->getCurrentState ();
    if (state == Federate::op_states::startup)
    {
        initialize ();
    }
    Time nextRequestTime = Time::maxVal ();
    Time currentTime;
    if (state != Federate::op_states::execution)
    {
        // send stuff before timeZero

        runSourceLoop (timeZero - timeEpsilon);

        fed->enterExecutionState ();
        // send the stuff at timeZero
        nextRequestTime = runSourceLoop (timeZero);
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
    helics::Time nextPrintTime = currentTime + 10.0;
    while ((nextRequestTime < Time::maxVal ()) && (nextRequestTime <= stopTime_input))
    {
        currentTime = fed->requestTime (nextRequestTime);
        nextRequestTime = runSourceLoop (currentTime);
        if (currentTime >= nextPrintTime)
        {
            std::cout << "processed time " << static_cast<double> (currentTime) << "\n";
            nextPrintTime += 10.0;
        }
    }
}

void Source::addPublication (const std::string &key,
                             const std::string &generator,
                             helics_type_t type,
                             Time period,
                             const std::string &units)
{
    // skip already existing publications
    if (pubids.find (key) != pubids.end ())
    {
        std::cerr << "publication already exists\n";
        return;
    }
    SourceObject newObj;

    newObj.pub = Publication (useLocal ? LOCAL : GLOBAL, fed, key, type, units);
    newObj.period = period;
    if (!generator.empty ())
    {
        auto res = generatorLookup.find (generator);
        if (res != generatorLookup.end ())
        {
            newObj.generatorIndex = res->second;
        }
    }
    sources.push_back (newObj);
    pubids[key] = static_cast<int> (sources.size ()) - 1;
}

int Source::addSignalGenerator (const std::string &name, const std::string &type)
{
    std::shared_ptr<SignalGenerator> gen;
    if (type == "sine")
    {
        gen = std::make_shared<SineGenerator> ();
    }
    else if (type == "ramp")
    {
        gen = std::make_shared<RampGenerator> ();
    }
    else if ((type == "oscillator") || (type == "phasor"))
    {
        gen = std::make_shared<PhasorGenerator> ();
    }
    generators.push_back (std::move (gen));
    auto index = static_cast<int> (generators.size () - 1);
    generatorLookup.emplace (name, index);
    return index;
}

std::shared_ptr<SignalGenerator> Source::getGenerator (int index)
{
    if (index < static_cast<int> (generators.size ()))
    {
        return generators[index];
    }
    return nullptr;
}

/** set the start time for a publication */
void Source::setStartTime (const std::string &key, Time startTime)
{
    auto fnd = pubids.find (key);
    if (fnd != pubids.end ())
    {
        sources[fnd->second].nextTime = startTime;
    }
}
/** set the start time for a publication */
void Source::setPeriod (const std::string &key, Time period)
{
    auto fnd = pubids.find (key);
    if (fnd != pubids.end ())
    {
        sources[fnd->second].period = period;
    }
}

/** tie a publication to a signal generator*/
void Source::linkPublicationToGenerator (const std::string &key, const std::string &generator)
{
    auto fnd = pubids.find (key);
    if (fnd == pubids.end ())
    {
        // only get here if something wasn't found
        throw (InvalidParameter (key + " was not recognized as a valid publication"));
    }
    auto findGen = generatorLookup.find (generator);
    if (findGen != generatorLookup.end ())
    {
        sources[fnd->second].generatorIndex = findGen->second;
        return;
    }
    sources[fnd->second].generatorName = generator;
}

/** tie a publication to a signal generator*/
void Source::linkPublicationToGenerator (const std::string &key, int genIndex)
{
    auto fnd = pubids.find (key);
    if (fnd == pubids.end ())
    {
        // only get here if something wasn't found
        throw (InvalidParameter (key + " was not recognized as a valid publication"));
    }
    sources[fnd->second].generatorIndex = genIndex;
}

Time Source::runSource (SourceObject &obj, Time currentTime)
{
    if (currentTime >= obj.nextTime)
    {
        if (obj.generatorIndex >= static_cast<int> (generators.size ()))
        {
            return Time::maxVal ();
        }
        auto val = generators[obj.generatorIndex]->generate (currentTime);
        obj.pub.publish (val);
        obj.nextTime += obj.period;
        if (obj.nextTime < currentTime)
        {
            auto periods = std::floor ((currentTime - obj.nextTime) / obj.period);
            obj.nextTime += periods * obj.period + obj.period;
        }
    }
    return obj.nextTime;
}

Time Source::runSourceLoop (Time currentTime)
{
    if (currentTime < timeZero)
    {
        for (auto &src : sources)
        {
            if (src.nextTime < timeZero)
            {
                runSource (src, currentTime);
                src.nextTime = timeZero;
            }
        }
        return timeZero;
    }
    Time minTime = Time::maxVal ();
    for (auto &src : sources)
    {
        auto tm = runSource (src, currentTime);
        if (tm < minTime)
        {
            minTime = tm;
        }
    }
    return minTime;
}
int Source::loadArguments (boost::program_options::variables_map &vm_map)
{
    //   std::cout << "read file " << points.size () << " points for " << tags.size () << " tags \n";

    if (vm_map.count ("default_period") == 0)
    {
        defaultPeriod = loadTimeFromString (vm_map["default_period"].as<std::string> ());
    }
    return 0;
}

}  // namespace apps
}  // namespace helics
