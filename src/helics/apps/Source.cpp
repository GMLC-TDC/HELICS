/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "Source.hpp"

#include "../common/JsonProcessingFunctions.hpp"
#include "../core/core-exceptions.hpp"
#include "../core/helicsCLI11.hpp"
#include "../core/helicsVersion.hpp"
#include "PrecHelper.hpp"
#include "SignalGenerators.hpp"
#include "gmlc/utilities/stringOps.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace helics {
namespace apps {
    void SignalGenerator::set(const std::string& /*parameter*/, double /*val*/) {}
    /** set a string parameter*/
    void SignalGenerator::setString(const std::string& /*parameter*/, const std::string& /*val*/) {}

    Source::Source(int argc, char* argv[]): App("source", argc, argv) { processArgs(); }

    Source::Source(std::vector<std::string> args): App("source", std::move(args)) { processArgs(); }

    void Source::processArgs()
    {
        helicsCLI11App app("Options specific to the Source App");
        app.add_option("--default_period", defaultPeriod, "the default period publications");
        if (!deactivated) {
            fed->setFlagOption(helics_flag_source_only);
            app.parse(remArgs);
            if (!masterFileName.empty()) {
                loadFile(masterFileName);
            }
        } else if (helpMode) {
            app.remove_helics_specifics();
            std::cout << app.help();
        }
    }
    Source::Source(const std::string& appName, const FederateInfo& fi): App(appName, fi)
    {
        fed->setFlagOption(helics_flag_source_only);
    }

    Source::Source(const std::string& appName,
                   const std::shared_ptr<Core>& core,
                   const FederateInfo& fi):
        App(appName, core, fi)
    {
        fed->setFlagOption(helics_flag_source_only);
    }

    Source::Source(const std::string& appName, CoreApp& core, const FederateInfo& fi):
        App(appName, core, fi)
    {
        fed->setFlagOption(helics_flag_source_only);
    }

    Source::Source(const std::string& name, const std::string& configString):
        App(name, configString)
    {
        fed->setFlagOption(helics_flag_source_only);

        Source::loadJsonFile(configString);
    }

    static void
        setGeneratorProperty(SignalGenerator* gen, const std::string& name, const Json::Value& prop)
    {
        if (prop.isDouble()) {
            gen->set(name, prop.asDouble());
        } else {
            try {
                auto time = loadJsonTime(prop);
                if (time > Time::minVal()) {
                    gen->set(name, static_cast<double>(time));
                } else {
                    gen->setString(name, prop.asString());
                }
            }
            catch (const std::invalid_argument&) {
                gen->setString(name, prop.asString());
            }
        }
    }

    void Source::loadJsonFile(const std::string& jsonString)
    {
        // we want to load the default period before constructing the interfaces so the default
        // period works
        auto doc = loadJson(jsonString);

        if (doc.isMember("source")) {
            auto appConfig = doc["source"];
            if (appConfig.isMember("defaultperiod")) {
                defaultPeriod = loadJsonTime(appConfig["defaultperiod"]);
            }
        }

        loadJsonFileConfiguration("source", jsonString);
        auto pubCount = fed->getPublicationCount();
        for (int ii = 0; ii < pubCount; ++ii) {
            sources.emplace_back(fed->getPublication(ii), defaultPeriod);
            pubids[sources.back().pub.getKey()] = static_cast<int>(sources.size()) - 1;
        }
        /* auto eptCount = fed->getEndpointCount();
     for (int ii = 0; ii < eptCount; ++ii)
     {
         endpoints.emplace_back(fed.get(), ii);
         eptids[endpoints.back().getName()] = static_cast<int> (endpoints.size() - 1);
     }
     */

        if (doc.isMember("publications")) {
            auto pubArray = doc["publications"];

            for (const auto& pubElement : pubArray) {
                auto key = getKey(pubElement);
                if (pubElement.isMember("start")) {
                    setStartTime(key, loadJsonTime(pubElement["start"]));
                }
                if (pubElement.isMember("period")) {
                    setPeriod(key, loadJsonTime(pubElement["period"]));
                }
                if (pubElement.isMember("generator")) {
                    if (pubElement["generator"].isInt()) {
                        linkPublicationToGenerator(key, pubElement["generator"].asInt());
                    } else {
                        linkPublicationToGenerator(key, pubElement["generator"].asString());
                    }
                }
            }
        }
        if (doc.isMember("generators")) {
            auto genArray = doc["generators"];
            for (const auto& genElement : genArray) {
                auto key = getKey(genElement);
                auto type = genElement["type"];
                int index = -1;
                if (!type.isNull()) {
                    index = addSignalGenerator(key, type.asString());
                } else {
                    std::cout << "generator " << key << " does not specify a type\n";
                    continue;
                }
                auto mnames = genElement.getMemberNames();
                for (auto& el : mnames) {
                    if ((el == "type") || (el == "name") || (el == "key")) {
                        continue;
                    }
                    if (el == "properties") {
                        for (auto& prop : genElement["properties"]) {
                            if ((prop.isMember("name")) && (prop.isMember("value"))) {
                                setGeneratorProperty(generators[index].get(),
                                                     prop["name"].asString(),
                                                     prop["value"]);
                            }
                        }
                    } else {
                        setGeneratorProperty(generators[index].get(), el, genElement[el]);
                    }
                }
            }
        }
    }

    void Source::initialize()
    {
        auto md = fed->getCurrentMode();
        if (md != Federate::modes::startup) {
            return;
        }

        int ii = 0;
        for (auto& src : sources) {
            if (src.generatorIndex < 0) {
                if (!src.generatorName.empty()) {
                    auto fnd = generatorLookup.find(src.generatorName);
                    if (fnd != generatorLookup.end()) {
                        src.generatorIndex = fnd->second;
                    } else {
                        std::cout << "unable to link to signal generator " << src.generatorName
                                  << std::endl;
                        src.nextTime = Time::maxVal();
                        src.generatorIndex = 0;
                    }
                } else {
                    if (ii < static_cast<int>(generators.size())) {
                        src.generatorIndex = ii;
                    } else {
                        src.generatorIndex = 0;
                    }
                }
            } else {
                if (src.generatorIndex >= static_cast<int>(generators.size())) {
                    std::cerr << "invalid generator index for " << src.pub.getKey()
                              << "disabling output\n";
                    src.nextTime = Time::maxVal();
                }
            }
        }

        fed->enterInitializingMode();
    }

    void Source::runTo(Time stopTime_input)
    {
        auto md = fed->getCurrentMode();
        if (md == Federate::modes::startup) {
            initialize();
        }
        Time nextRequestTime = Time::maxVal();
        Time currentTime;
        if (md != Federate::modes::executing) {
            // send stuff before timeZero

            runSourceLoop(timeZero - timeEpsilon);

            fed->enterExecutingMode();
            // send the stuff at timeZero
            nextRequestTime = runSourceLoop(timeZero);
            currentTime = timeZero;
        } else {
            currentTime = fed->getCurrentTime();

            for (auto& src : sources) {
                if (src.nextTime < nextRequestTime) {
                    nextRequestTime = src.nextTime;
                }
            }
        }
        helics::Time nextPrintTime = currentTime + 10.0;
        while ((nextRequestTime < Time::maxVal()) && (nextRequestTime <= stopTime_input)) {
            currentTime = fed->requestTime(nextRequestTime);
            nextRequestTime = runSourceLoop(currentTime);
            if (currentTime >= nextPrintTime) {
                std::cout << "processed time " << static_cast<double>(currentTime) << "\n";
                nextPrintTime += 10.0;
            }
        }
    }

    void Source::addPublication(const std::string& key,
                                const std::string& generator,
                                data_type type,
                                Time period,
                                const std::string& units)
    {
        // skip already existing publications
        if (pubids.find(key) != pubids.end()) {
            std::cerr << "publication already exists\n";
            return;
        }
        SourceObject newObj(Publication(useLocal ? interface_visibility::local :
                                                   interface_visibility::global,
                                        fed,
                                        key,
                                        typeNameStringRef(type),
                                        units),
                            period);

        if (!generator.empty()) {
            auto res = generatorLookup.find(generator);
            if (res != generatorLookup.end()) {
                newObj.generatorIndex = res->second;
            }
        }
        sources.push_back(std::move(newObj));
        pubids[key] = static_cast<int>(sources.size()) - 1;
    }

    int Source::addSignalGenerator(const std::string& name, const std::string& type)
    {
        std::shared_ptr<SignalGenerator> gen;
        if (type == "sine") {
            gen = std::make_shared<SineGenerator>();
        } else if (type == "ramp") {
            gen = std::make_shared<RampGenerator>();
        } else if ((type == "oscillator") || (type == "phasor")) {
            gen = std::make_shared<PhasorGenerator>();
        }
        generators.push_back(std::move(gen));
        auto index = static_cast<int>(generators.size() - 1);
        generatorLookup.emplace(name, index);
        return index;
    }

    std::shared_ptr<SignalGenerator> Source::getGenerator(int index)
    {
        if (index < static_cast<int>(generators.size())) {
            return generators[index];
        }
        return nullptr;
    }

    /** set the start time for a publication */
    void Source::setStartTime(const std::string& key, Time startTime)
    {
        auto fnd = pubids.find(key);
        if (fnd != pubids.end()) {
            sources[fnd->second].nextTime = startTime;
        }
    }
    /** set the start time for a publication */
    void Source::setPeriod(const std::string& key, Time period)
    {
        auto fnd = pubids.find(key);
        if (fnd != pubids.end()) {
            sources[fnd->second].period = period;
        }
    }

    /** tie a publication to a signal generator*/
    void Source::linkPublicationToGenerator(const std::string& key, const std::string& generator)
    {
        auto fnd = pubids.find(key);
        if (fnd == pubids.end()) {
            // only get here if something wasn't found
            throw(InvalidParameter(key + " was not recognized as a valid publication"));
        }
        auto findGen = generatorLookup.find(generator);
        if (findGen != generatorLookup.end()) {
            sources[fnd->second].generatorIndex = findGen->second;
            return;
        }
        sources[fnd->second].generatorName = generator;
    }

    /** tie a publication to a signal generator*/
    void Source::linkPublicationToGenerator(const std::string& key, int genIndex)
    {
        auto fnd = pubids.find(key);
        if (fnd == pubids.end()) {
            // only get here if something wasn't found
            throw(InvalidParameter(key + " was not recognized as a valid publication"));
        }
        sources[fnd->second].generatorIndex = genIndex;
    }

    Time Source::runSource(SourceObject& obj, Time currentTime)
    {
        if (currentTime >= obj.nextTime) {
            if (obj.generatorIndex >= static_cast<int>(generators.size())) {
                return Time::maxVal();
            }
            auto val = generators[obj.generatorIndex]->generate(currentTime);
            obj.pub.publish(val);
            obj.nextTime += obj.period;
            if (obj.nextTime < currentTime) {
                auto periods = std::floor((currentTime - obj.nextTime) / obj.period);
                obj.nextTime += periods * obj.period + obj.period;
            }
        }
        return obj.nextTime;
    }

    Time Source::runSourceLoop(Time currentTime)
    {
        if (currentTime < timeZero) {
            for (auto& src : sources) {
                if (src.nextTime < timeZero) {
                    runSource(src, currentTime);
                    src.nextTime = timeZero;
                }
            }
            return timeZero;
        }
        Time minTime = Time::maxVal();
        for (auto& src : sources) {
            auto tm = runSource(src, currentTime);
            if (tm < minTime) {
                minTime = tm;
            }
        }
        return minTime;
    }

}  // namespace apps
}  // namespace helics
