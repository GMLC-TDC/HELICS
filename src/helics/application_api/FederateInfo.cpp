/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#include "Federate.hpp"

#include "../common/JsonProcessingFunctions.hpp"
#include "../core/core-exceptions.hpp"
#include "../core/helicsVersion.hpp"
#include <fstream>
#include <iostream>
#include <boost/filesystem.hpp>

#include "../common/argParser.h"

namespace filesystem = boost::filesystem;

namespace helics
{
using namespace std::string_literals;
static const ArgDescriptors InfoArgs{{"broker,b"s, "address of the broker to connect"s},
                                     {"name,n"s, "name of the player federate"s},
                                     {"corename"s, "the name of the core to create or find"s},
                                     {"core,c"s, "type of the core to connect to"s},
                                     {"offset"s, "the offset of the time steps"s},
                                     {"period"s, "the period of the federate"s},
                                     {"timedelta"s, "the time delta of the federate"s},
                                     { "rttolerance"s, "the time tolerance of the real time mode"s },
                                     {"coreinit,i"s, "the core initialization string"s},
                                     {"separator"s, "separator character for local federates"s},
                                     {"inputdelay"s, "the input delay on incoming communication of the federate"s},
                                     {"outputdelay"s,
                                      "the output delay for outgoing communication of the federate"s},
                                     {"flags,f"s, ArgDescriptor::arg_type_t::vector_string,
                                      "named flag for the federate"s}};

FederateInfo::FederateInfo (int argc, const char *const *argv) { loadInfoFromArgs (argc, argv); }

void FederateInfo::loadInfoFromArgs (int argc, const char *const *argv)
{
    variable_map vm;
    auto res = argumentParser (argc, argv, vm, InfoArgs);
    if (res == versionReturn)
    {
        std::cout << helics::versionString << '\n';
    }
    if (res < 0)
    {
        return;
    }
    if (vm.count ("name") > 0)
    {
        name = vm["name"].as<std::string> ();
    }
    if (vm.count ("corename") > 0)
    {
        coreName = vm["corename"].as<std::string> ();
    }
    std::string coretypename;
    if (vm.count ("core") > 0)
    {
        coretypename = vm["core"].as<std::string> ();
    }

    coreType = helics::coreTypeFromString (coretypename);

    coreInitString = "1";
    if (vm.count ("coreinit") > 0)
    {
        coreInitString.push_back (' ');
        coreInitString = vm["coreinit"].as<std::string> ();
    }
    if (vm.count ("broker") > 0)
    {
        coreInitString += " --broker=";
        coreInitString += vm["broker"].as<std::string> ();
    }

    if (vm.count ("timedelta") > 0)
    {
        timeDelta = loadTimeFromString (vm["timedelta"].as<std::string> ());
    }
    if (vm.count ("inputdelay") > 0)
    {
        timeDelta = loadTimeFromString (vm["inputdelay"].as<std::string> ());
    }
    if (vm.count("rttolerance") > 0)
    {
        rtTolerance = loadTimeFromString(vm["rttolerance"].as<std::string>());
    }

    if (vm.count ("outputdelay") > 0)
    {
        timeDelta = loadTimeFromString (vm["outputdelay"].as<std::string> ());
    }

    if (vm.count ("period") > 0)
    {
        period = loadTimeFromString (vm["period"].as<std::string> ());
    }

    if (vm.count ("offset") > 0)
    {
        offset = loadTimeFromString (vm["offset"].as<std::string> ());
    }
    if (vm.count ("maxiterations") > 0)
    {
        maxIterations = static_cast<int16_t> (vm["maxiterations"].as<int> ());
    }
    if (vm.count ("separator") > 0)
    {
        auto sep = vm["separator"].as<std::string> ();
        if (!sep.empty ())
        {
            separator = sep[0];
        }
    }
    if (vm.count ("flags") > 0)
    {
        auto vflag = vm["flags"].as<std::vector<std::string>> ();
        for (auto flag : vflag)
        {
            if (flag == "observer")
            {
                observer = true;
            }
            else if (flag == "rollback")
            {
                rollback = true;
            }
            else if (flag == "only_update_on_change")
            {
                only_update_on_change = true;
            }
            else if (flag == "only_transmit_on_change")
            {
                only_transmit_on_change = true;
            }
            else if (flag == "source_only")
            {
                source_only = true;
            }
            else if (flag == "uninterruptible")
            {
                uninterruptible = true;
            }
            else if (flag == "interruptible")  // can use either flag
            {
                uninterruptible = false;
            }
            else if (flag == "forward_compute")
            {
                forwardCompute = true;
            }
            else if (flag == "realtime")
            {
                realtime = true;
            }
            else if (flag == "delayed_update")
            {
               wait_for_current_time_updates = true;
            }
            else
            {
                std::cerr << "unrecognized flag " << flag << std::endl;
            }
        }
    }
}

FederateInfo loadFederateInfo (const std::string &jsonString)
{
    return loadFederateInfo (std::string (), jsonString);
}

FederateInfo loadFederateInfo (const std::string &name, const std::string &jsonString)
{
    FederateInfo fi (name);
    Json_helics::Value doc;
    try
    {
        doc = loadJsonString (jsonString);
    }
    catch (const std::invalid_argument &ia)
    {
        throw (helics::InvalidParameter (ia.what ()));
    }

    if (doc.isMember ("name"))
    {
        fi.name = doc["name"].asString ();
    }

    if (doc.isMember ("observer"))
    {
        fi.observer = doc["observer"].asBool ();
    }
    if (doc.isMember ("rollback"))
    {
        fi.rollback = doc["rollback"].asBool ();
    }
    if (doc.isMember ("only_update_on_change"))
    {
        fi.only_update_on_change = doc["only_update_on_change"].asBool ();
    }
    if (doc.isMember ("only_transmit_on_change"))
    {
        fi.only_transmit_on_change = doc["only_transmit_on_change"].asBool ();
    }
    if (doc.isMember ("source_only"))
    {
        fi.source_only = doc["sourc_only"].asBool ();
    }
    if (doc.isMember ("uninterruptible"))
    {
        fi.uninterruptible = doc["uninterruptible"].asBool ();
    }
    if (doc.isMember ("interruptible"))  // can use either flag
    {
        fi.uninterruptible = !doc["uninterruptible"].asBool ();
    }
    if (doc.isMember ("forward_compute"))
    {
        fi.forwardCompute = doc["forward_compute"].asBool ();
    }
    if (doc.isMember("realtime"))
    {
        fi.realtime= doc["realtime"].asBool();
    }
    if (doc.isMember("rttolerance"))
    {
        fi.rtTolerance = loadJsonTime(doc["rttolerance"]);
    }
    if (doc.isMember ("separator"))
    {
        auto sep = doc["separator"].asString ();
        if (!sep.empty ())
        {
            fi.separator = sep[0];
        }
    }
    if (doc.isMember ("coreType"))
    {
        try
        {
            fi.coreType = coreTypeFromString (doc["coreType"].asString ());
        }
        catch (const std::invalid_argument &ia)
        {
            std::cerr << "Unrecognized core type\n";
        }
    }
    if (doc.isMember ("coreName"))
    {
        fi.coreName = doc["coreName"].asString ();
    }
    if (doc.isMember ("coreInit"))
    {
        fi.coreInitString = doc["coreInit"].asString ();
    }
    if (doc.isMember("delayed_update"))
    {
        fi.wait_for_current_time_updates= doc["delayed_update"].asBool();
    }
    if (doc.isMember ("maxIterations"))
    {
        fi.maxIterations = static_cast<int16_t> (doc["maxIterations"].asInt ());
    }
    if (doc.isMember ("period"))
    {
        fi.period = loadJsonTime (doc["period"]);
    }

    if (doc.isMember ("offset"))
    {
        fi.offset = loadJsonTime (doc["offset"]);
    }

    if (doc.isMember ("timeDelta"))
    {
        fi.timeDelta = loadJsonTime (doc["timeDelta"]);
    }

    if (doc.isMember ("outputDelay"))
    {
        fi.outputDelay = loadJsonTime (doc["outputDelay"]);
    }
    if (doc.isMember ("inputDelay"))
    {
        fi.inputDelay = loadJsonTime (doc["inputDelay"]);
    }
    return fi;
}
}  // namespace helics
