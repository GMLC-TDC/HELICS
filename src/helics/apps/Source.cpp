/*
Copyright (c) 2017-2024,
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
    void SignalGenerator::set(std::string_view /*parameter*/, double /*val*/) {}
    /** set a string parameter*/
    void SignalGenerator::setString(std::string_view /*parameter*/, std::string_view /*val*/) {}

    Source::Source(int argc, char* argv[]): App("source_${#}", argc, argv)
    {
        processArgs();
        initialSetup();
    }

    Source::Source(std::vector<std::string> args): App("source_${#}", std::move(args))
    {
        processArgs();
        initialSetup();
    }

    void Source::processArgs()
    {
        helicsCLI11App app("Options specific to the Source App");
        app.add_option("--default_period", defaultPeriod, "the default period publications");
        if (!deactivated) {
            app.parse(remArgs);
        } else if (helpMode) {
            app.remove_helics_specifics();
            std::cout << app.help();
        }
    }
    Source::Source(std::string_view appName, const FederateInfo& fedInfo): App(appName, fedInfo)
    {
        initialSetup();
    }

    Source::Source(std::string_view appName,
                   const std::shared_ptr<Core>& core,
                   const FederateInfo& fedInfo): App(appName, core, fedInfo)
    {
        initialSetup();
    }

    Source::Source(std::string_view appName, CoreApp& core, const FederateInfo& fedInfo):
        App(appName, core, fedInfo)
    {
        initialSetup();
    }

    Source::Source(std::string_view name, const std::string& configString): App(name, configString)
    {
        processArgs();
        initialSetup();
    }

    void Source::initialSetup()
    {
        if (!deactivated) {
            fed->setFlagOption(HELICS_FLAG_SOURCE_ONLY);
            loadInputFiles();
        }
    }

    static void setGeneratorProperty(SignalGenerator* gen,
                                     std::string_view name,
                                     const nlohmann::json& prop)
    {
        if (prop.is_number()) {
            gen->set(name, prop.get<double>());
        } else {
            try {
                auto time = fileops::loadJsonTime(prop);
                if (time > Time::minVal()) {
                    gen->set(name, static_cast<double>(time));
                } else {
                    gen->setString(name, prop.get<std::string>());
                }
            }
            catch (const std::invalid_argument&) {
                gen->setString(name, prop.get<std::string>());
            }
        }
    }

    void Source::loadJsonFile(const std::string& jsonString,
                              bool enableFederateInterfaceRegistration)
    {
        // we want to load the default period before constructing the interfaces so the default
        // period works
        auto doc = fileops::loadJson(jsonString);

        if (doc.contains("source")) {
            auto appConfig = doc["source"];
            if (appConfig.contains("defaultperiod")) {
                defaultPeriod = fileops::loadJsonTime(appConfig["defaultperiod"]);
            }
        }

        loadJsonFileConfiguration("source", jsonString, enableFederateInterfaceRegistration);
        auto pubCount = fed->getPublicationCount();
        for (int ii = 0; ii < pubCount; ++ii) {
            sources.emplace_back(fed->getPublication(ii), defaultPeriod);
            pubids[sources.back().pub.getName()] = static_cast<int>(sources.size()) - 1;
        }
        /* auto eptCount = fed->getEndpointCount();
     for (int ii = 0; ii < eptCount; ++ii)
     {
         endpoints.emplace_back(fed.get(), ii);
         eptids[endpoints.back().getName()] = static_cast<int> (endpoints.size() - 1);
     }
     */

        if (doc.contains("publications")) {
            auto pubArray = doc["publications"];

            for (const auto& pubElement : pubArray) {
                auto key = fileops::getName(pubElement);
                if (pubElement.contains("start")) {
                    setStartTime(key, fileops::loadJsonTime(pubElement["start"]));
                }
                if (pubElement.contains("period")) {
                    setPeriod(key, fileops::loadJsonTime(pubElement["period"]));
                }
                if (pubElement.contains("generator")) {
                    if (pubElement["generator"].is_number_integer()) {
                        linkPublicationToGenerator(key, pubElement["generator"].get<int>());
                    } else {
                        linkPublicationToGenerator(key, pubElement["generator"].get<std::string>());
                    }
                }
            }
        }
        if (doc.contains("generators")) {
            auto genArray = doc["generators"];
            for (const auto& genElement : genArray) {
                auto key = fileops::getName(genElement);
                auto type = genElement["type"];
                int index = -1;
                if (!type.is_null()) {
                    index = addSignalGenerator(key, type.get<std::string>());
                } else {
                    std::cout << "generator " << key << " does not specify a type\n";
                    continue;
                }
                for (auto& el : genElement.items()) {
                    if ((el.key() == "type") || (el.key() == "name") || (el.key() == "key")) {
                        continue;
                    }
                    if (el.key() == "properties") {
                        for (auto& prop : el.value()) {
                            if ((prop.contains("name")) && (prop.contains("value"))) {
                                setGeneratorProperty(generators[index].get(),
                                                     prop["name"].get<std::string>(),
                                                     prop["value"]);
                            }
                        }
                    } else {
                        setGeneratorProperty(generators[index].get(), el.key(), el.value());
                    }
                }
            }
        }
    }

    void Source::initialize()
    {
        auto md = fed->getCurrentMode();
        if (md != Federate::Modes::STARTUP) {
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
                    std::cerr << "invalid generator index for " << src.pub.getName()
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
        if (md == Federate::Modes::STARTUP) {
            initialize();
        }
        Time nextRequestTime = Time::maxVal();
        Time currentTime;
        if (md != Federate::Modes::EXECUTING) {
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

    void Source::addPublication(std::string_view key,
                                std::string_view generator,
                                DataType type,
                                Time period,
                                std::string_view units)
    {
        // skip already existing publications
        if (pubids.find(key) != pubids.end()) {
            std::cerr << "publication already exists\n";
            return;
        }
        SourceObject newObj(Publication(useLocal ? InterfaceVisibility::LOCAL :
                                                   InterfaceVisibility::GLOBAL,
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
        pubids[sources.back().pub.getName()] = static_cast<int>(sources.size()) - 1;
    }

    int Source::addSignalGenerator(std::string_view name, std::string_view type)
    {
        std::shared_ptr<SignalGenerator> gen;
        if (type == "sine") {
            gen = std::make_shared<SineGenerator>(name);
        } else if (type == "ramp") {
            gen = std::make_shared<RampGenerator>(name);
        } else if ((type == "oscillator") || (type == "phasor")) {
            gen = std::make_shared<PhasorGenerator>(name);
        }
        generators.push_back(std::move(gen));
        auto index = static_cast<int>(generators.size() - 1);
        generatorLookup.emplace(generators.back()->getName(), index);
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
    void Source::setStartTime(std::string_view key, Time startTime)
    {
        auto fnd = pubids.find(key);
        if (fnd != pubids.end()) {
            sources[fnd->second].nextTime = startTime;
        }
    }
    /** set the start time for a publication */
    void Source::setPeriod(std::string_view key, Time period)
    {
        auto fnd = pubids.find(key);
        if (fnd != pubids.end()) {
            sources[fnd->second].period = period;
        }
    }

    /** tie a publication to a signal generator*/
    void Source::linkPublicationToGenerator(std::string_view key, std::string_view generator)
    {
        auto fnd = pubids.find(key);
        if (fnd == pubids.end()) {
            // only get here if something wasn't found
            throw(
                InvalidParameter(std::string(key) + " was not recognized as a valid publication"));
        }
        auto findGen = generatorLookup.find(generator);
        if (findGen != generatorLookup.end()) {
            sources[fnd->second].generatorIndex = findGen->second;
            return;
        }
        sources[fnd->second].generatorName = generator;
    }

    /** tie a publication to a signal generator*/
    void Source::linkPublicationToGenerator(std::string_view key, int genIndex)
    {
        auto fnd = pubids.find(key);
        if (fnd == pubids.end()) {
            // only get here if something wasn't found
            throw(
                InvalidParameter(std::string(key) + " was not recognized as a valid publication"));
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
