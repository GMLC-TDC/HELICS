/*

Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/

#include "Federate.hpp"

#include "../common/JsonProcessingFunctions.hpp"
#include "../core/core-exceptions.hpp"
#include "../core/helicsVersion.hpp"
#include <iostream>
#include <fstream>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include "../common/JsonProcessingFunctions.hpp"

namespace po = boost::program_options;
namespace filesystem = boost::filesystem;

static void argumentParser (int argc, const char *const *argv, po::variables_map &vm_map);

namespace helics
{
FederateInfo::FederateInfo (int argc, const char *const *argv) { loadInfoFromArgs (argc, argv); }

void FederateInfo::loadInfoFromArgs (int argc, const char *const *argv)
{
    po::variables_map vm;
    argumentParser (argc, argv, vm);
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
        timeDelta = loadTimeFromString(vm["timedelta"].as<std::string> ());
    }
    if (vm.count ("inputdelay") > 0)
    {
        timeDelta = loadTimeFromString(vm["inputdelay"].as<std::string> ());
    }

    if (vm.count ("outputdelay") > 0)
    {
        timeDelta = loadTimeFromString(vm["outputdelay"].as<std::string> ());
    }

    if (vm.count ("period") > 0)
    {
        period = loadTimeFromString(vm["period"].as<std::string> ());
    }

    if (vm.count ("offset") > 0)
    {
        offset = loadTimeFromString(vm["offset"].as<std::string> ());
    }
    if (vm.count ("maxiterations") > 0)
    {
        maxIterations = static_cast<int16_t> (vm["maxiterations"].as<int> ());
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
            else
            {
                std::cerr << "unrecognized flag " << flag << std::endl;
            }
        }
    }
}

FederateInfo loadFederateInfo (const std::string &jsonString)
{
    FederateInfo fi;
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

void argumentParser (int argc, const char *const *argv, po::variables_map &vm_map)
{
    po::options_description cmd_only ("command line only");
    po::options_description config ("configuration");
    po::options_description hidden ("hidden");

    // clang-format off
        // input boost controls
        cmd_only.add_options()
            ("help,h", "produce help message")
            ("version,v", "helics version number")
            ("config-file", po::value<std::string>(), "specify a configuration file to use");


        config.add_options()
            ("broker,b", po::value<std::string>(), "address of the broker to connect")
            ("name,n", po::value<std::string>(), "name of the player federate")
            ("corename", po::value<std::string>(), "the name of the core to create or find")
            ("core,c", po::value<std::string>(), "type of the core to connect to")
            ("offset", po::value<std::string>(), "the offset of the time steps")
            ("period", po::value<std::string>(), "the period of the federate")
            ("timedelta", po::value<std::string>(), "the time delta of the federate")
            ("coreinit,i", po::value<std::string>(), "the core initialization string")
            ("inputdelay", po::value<std::string>(), "the time delta of the federate")
            ("outputdelay", po::value<std::string>(), "the time delta of the federate")
            ("flags,f", po::value<std::vector<std::string>>(), "named flag for the federate");


        hidden.add_options() ("input", po::value<std::string>(), "input file");
    // clang-format on

    po::options_description cmd_line ("command line options");
    po::options_description config_file ("configuration file options");
    po::options_description visible ("allowed options");

    cmd_line.add (cmd_only).add (config).add (hidden);
    config_file.add (config).add (hidden);
    visible.add (cmd_only).add (config);

    po::variables_map cmd_vm;
    try
    {
        po::store (po::command_line_parser (argc, argv).options (cmd_line).allow_unregistered ().run (), cmd_vm);
    }
    catch (std::exception &e)
    {
        std::cerr << e.what () << std::endl;
        throw (e);
    }

    po::notify (cmd_vm);

    // objects/pointers/variables/constants

    // program options control
    if (cmd_vm.count ("help") > 0)
    {
        std::cout << visible << '\n';
        return;
    }

    if (cmd_vm.count ("version") > 0)
    {
        std::cout << helics::helicsVersionString () << '\n';
        return;
    }

    po::store (po::command_line_parser (argc, argv).options (cmd_line).allow_unregistered ().run (), vm_map);

    if (cmd_vm.count ("config-file") > 0)
    {
        std::string config_file_name = cmd_vm["config-file"].as<std::string> ();
        if (!filesystem::exists (config_file_name))
        {
            std::cerr << "config file " << config_file_name << " does not exist\n";
            throw (std::invalid_argument ("unknown config file"));
        }
        else
        {
            std::ifstream fstr (config_file_name.c_str ());
            po::store (po::parse_config_file (fstr, config_file), vm_map);
            fstr.close ();
        }
    }

    po::notify (vm_map);
}
