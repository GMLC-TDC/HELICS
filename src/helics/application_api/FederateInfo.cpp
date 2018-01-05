/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/

#include "Federate.h"

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <iostream>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4702)
#include "json/json.h"
#pragma warning(pop)
#else
#include "json/json.h"
#endif

namespace po = boost::program_options;
namespace filesystem = boost::filesystem;


static void argumentParser(int argc, const char *const *argv, po::variables_map &vm_map);

namespace helics
{
    FederateInfo::FederateInfo(int argc, const char * const *argv)
    {
        loadInfoFromArgs(argc, argv);
    }

    void FederateInfo::loadInfoFromArgs(int argc, const char * const *argv)
    {
        po::variables_map vm;
        argumentParser(argc, argv, vm);
        if (vm.count("name") > 0)
        {
            name = vm["name"].as<std::string>();
        }
        if (vm.count("corename") > 0)
        {
            coreName = vm["corename"].as<std::string>();
        }
        std::string coretypename;
        if (vm.count("core") > 0)
        {
            coretypename = vm["core"].as<std::string>();
        }

        coreType = helics::coreTypeFromString(coretypename);

        coreInitString = "1";
        if (vm.count("coreinit") > 0)
        {
            coreInitString.push_back(' ');
            coreInitString = vm["coreinit"].as<std::string>();
        }
        if (vm.count("broker") > 0)
        {
            coreInitString += " --broker=";
            coreInitString += vm["broker"].as<std::string>();
        }

        if (vm.count("timedelta") > 0)
        {
            timeDelta = vm["timedelta"].as<double>();
        }

        if (vm.count("period") > 0)
        {
            period = vm["period"].as<double>();
        }

        if (vm.count("offset") > 0)
        {
            offset = vm["offset"].as<double>();
        }
        if (vm.count("max_iterations")>0)
        {
            max_iterations = static_cast<int16_t> (vm["maxiterations"].as<int>());
        }
    }



    FederateInfo LoadFederateInfo(const std::string &jsonString)
    {
        FederateInfo fi;
        std::ifstream file(jsonString);
        Json_helics::Value doc;

        if (file.is_open())
        {
            Json_helics::CharReaderBuilder rbuilder;
            std::string errs;
            bool ok = Json_helics::parseFromStream(rbuilder, file, &doc, &errs);
            if (!ok)
            {
                // should I throw an error here?
                return fi;
            }
        }
        else
        {
            Json_helics::CharReaderBuilder rbuilder;
            std::string errs;
            std::istringstream jstring(jsonString);
            bool ok = Json_helics::parseFromStream(rbuilder, jstring, &doc, &errs);
            if (!ok)
            {
                // should I throw an error here?
                return fi;
            }
        }

        if (doc.isMember("name"))
        {
            fi.name = doc["name"].asString();
        }

        if (doc.isMember("observer"))
        {
            fi.observer = doc["observer"].asBool();
        }
        if (doc.isMember("rollback"))
        {
            fi.rollback = doc["rollback"].asBool();
        }
        if (doc.isMember("only_update_on_change"))
        {
            fi.only_update_on_change = doc["only_update_on_change"].asBool();
        }
        if (doc.isMember("only_transmit_on_change"))
        {
            fi.only_transmit_on_change = doc["only_transmit_on_change"].asBool();
        }
        if (doc.isMember("source_only"))
        {
            fi.source_only = doc["sourc_only"].asBool();
        }
        if (doc.isMember("uninterruptible"))
        {
            fi.uninterruptible = doc["uninterruptible"].asBool();
        }
        if (doc.isMember("interruptible"))  // can use either flag
        {
            fi.uninterruptible = !doc["uninterruptible"].asBool();
        }
        if (doc.isMember("forwardCompute"))
        {
            fi.forwardCompute = doc["forwardCompute"].asBool();
        }
        if (doc.isMember("coreType"))
        {
            try
            {
                fi.coreType = coreTypeFromString(doc["coreType"].asString());
            }
            catch (const std::invalid_argument &ia)
            {
                std::cerr << "Unrecognized core type\n";
            }
        }
        if (doc.isMember("coreName"))
        {
            fi.coreName = doc["coreName"].asString();
        }
        if (doc.isMember("coreInit"))
        {
            fi.coreInitString = doc["coreInit"].asString();
        }
        if (doc.isMember("maxiterations"))
        {
            fi.max_iterations = static_cast<int16_t> (doc["maxiterations"].asInt());
        }
        if (doc.isMember("period"))
        {
            if (doc["period"].isObject())
            {
            }
            else
            {
                fi.timeDelta = doc["period"].asDouble();
            }
        }

        if (doc.isMember("offset"))
        {
            if (doc["offset"].isObject())
            {
            }
            else
            {
                fi.offset = doc["offset"].asDouble();
            }
        }

        if (doc.isMember("timeDelta"))
        {
            if (doc["timeDelta"].isObject())
            {
            }
            else
            {
                fi.timeDelta = doc["timeDelta"].asDouble();
            }
        }

        if (doc.isMember("lookAhead"))
        {
            if (doc["lookAhead"].isObject())
            {
                // TODO:: something about units yet
            }
            else
            {
                fi.lookAhead = doc["lookAhead"].asDouble();
            }
        }
        if (doc.isMember("impactWindow"))
        {
            if (doc["impactWindow"].isObject())
            {
                // TOOD:: something about units yet
            }
            else
            {
                fi.impactWindow = doc["impactWindow"].asDouble();
            }
        }
        return fi;
    }
} //namespace helics

    void argumentParser(int argc, const char *const *argv, po::variables_map &vm_map)
    {
        po::options_description cmd_only("command line only");
        po::options_description config("configuration");
        po::options_description hidden("hidden");

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
            ("offset", po::value<double>(), "the offset of the time steps")
            ("period", po::value<double>(), "the period of the federate")
            ("timedelta", po::value<double>(), "the time delta of the federate")
            ("coreinit,i", po::value<std::string>(), "the core initialization string")
            ("flags,f", po::value<std::vector<std::string>>(), "named flag for the federate");


        hidden.add_options() ("input", po::value<std::string>(), "input file");
        // clang-format on

        po::options_description cmd_line("command line options");
        po::options_description config_file("configuration file options");
        po::options_description visible("allowed options");

        cmd_line.add(cmd_only).add(config).add(hidden);
        config_file.add(config).add(hidden);
        visible.add(cmd_only).add(config);

        po::variables_map cmd_vm;
        try
        {
            po::store(po::command_line_parser(argc, argv).options(cmd_line).allow_unregistered().run(), cmd_vm);
        }
        catch (std::exception &e)
        {
            std::cerr << e.what() << std::endl;
            throw (e);
        }

        po::notify(cmd_vm);

        // objects/pointers/variables/constants

        // program options control
        if (cmd_vm.count("help") > 0)
        {
            std::cout << visible << '\n';
            return;
        }

        if (cmd_vm.count("version") > 0)
        {
            std::cout << helics::getHelicsVersionString() << '\n';
            return;
        }

        po::store(po::command_line_parser(argc, argv).options(cmd_line).allow_unregistered().run(), vm_map);

        if (cmd_vm.count("config-file") > 0)
        {
            std::string config_file_name = cmd_vm["config-file"].as<std::string>();
            if (!filesystem::exists(config_file_name))
            {
                std::cerr << "config file " << config_file_name << " does not exist\n";
                throw (std::invalid_argument("unknown config file"));
            }
            else
            {
                std::ifstream fstr(config_file_name.c_str());
                po::store(po::parse_config_file(fstr, config_file), vm_map);
                fstr.close();
            }
        }

        po::notify(vm_map);

    }


