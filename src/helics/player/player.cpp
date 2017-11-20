/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "../application_api/Publications.hpp"
#include "../application_api/ValueFederate.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <regex>
#include <set>
#include <stdexcept>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include "PrecHelper.h"

class ValueSetter
{
  public:
    helics::Time time;
    int index;
    std::string type;
    std::string pubName;
    std::string value;
};

namespace po = boost::program_options;
namespace filesystem = boost::filesystem;

bool vComp (ValueSetter &v1, ValueSetter &v2) { return (v1.time < v2.time); }
void argumentParser (int argc, const char *const *argv, po::variables_map &vm_map);

const std::regex creg (R"raw((-?\d+(\.\d+)?|\.\d+)\s*([^\s]*)(\s+[cCdDvVsSiIfF]?\s+|\s+)([^\s]*))raw");

int main (int argc, char *argv[])
{
    std::ifstream infile;
    helics::helicsType_t defType = helics::helicsType_t::helicsString;

    po::variables_map vm;
    argumentParser (argc, argv, vm);

    std::vector<ValueSetter> points;
    if (vm.count ("input") == 0)
    {
        return 0;
    }

    if (vm.count ("datatype") > 0)
    {
        defType = helics::getTypeFromString (vm["datatype"].as<std::string> ());
        if (defType == helics::helicsType_t::helicsInvalid)
        {
            std::cerr << vm["datatype"].as<std::string> () << " is not recognized as a valid type \n";
            return -3;
        }
    }

    if (!filesystem::exists (vm["input"].as<std::string> ()))
    {
        std::cerr << vm["input"].as<std::string> () << " does not exist \n";
        return -3;
    }
    infile.open (vm["input"].as<std::string> ().c_str ());
    std::string str;

    int lcnt = 0;
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
        ++lcnt;
    }
    points.resize (lcnt);
    // now start over and actual do the loading
    infile.close ();
    infile.open (vm["input"].as<std::string> ().c_str ());
    int icnt = 0;
    std::smatch m;
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
        std::regex_search (str, m, creg);
        if (m.size () == 6)
        {
            points[icnt].time = helics::Time (std::stod (m[1]));
            points[icnt].pubName = m[3];
            points[icnt].type = m[4];
            points[icnt].value = m[5];
            ++icnt;
        }
    }
    // collapse tags to the reduced list
    std::set<std::pair<std::string, std::string>> tags;
    for (auto &vs : points)
    {
        tags.emplace (vs.pubName, vs.type);
    }

    std::vector<helics::Publication> publications;
    std::map<std::string, int> pubids;
    std::cout << "read file " << points.size () << " points for " << tags.size () << " tags \n";
    std::string name = "player";
    if (vm.count ("name") > 0)
    {
        name = vm["name"].as<std::string> ();
    }

    std::string coretype;
    if (vm.count ("core") > 0)
    {
        coretype = vm["core"].as<std::string> ();
    }
    helics::Time stopTime = helics::Time::maxVal ();
    if (vm.count ("stop") > 0)
    {
        stopTime = vm["stop"].as<double> ();
    }
    helics::FederateInfo fi (name);
    try
    {
        fi.coreType = helics::coreTypeFromString (coretype);
    }
    catch (std::invalid_argument &ia)
    {
        std::cerr << coretype << " is not recognized as a valid core type [zmq,ipc,udp,tcp,test,mpi]\n";
        return (-1);
    }
    fi.coreInitString = "1";
    if (vm.count ("coreinit") > 0)
    {
        fi.coreInitString.push_back (' ');
        fi.coreInitString += vm["coreinit"].as<std::string> ();
    }
    if (vm.count ("broker") > 0)
    {
        fi.coreInitString += " --broker=";
        fi.coreInitString += vm["broker"].as<std::string> ();
    }
    fi.source_only = true;
    if (vm.count ("timedelta") > 0)
    {
        fi.timeDelta = vm["timedelta"].as<double> ();
    }
    auto vFed = std::make_unique<helics::ValueFederate> (fi);

    std::string prevTag;
    for (auto &tname : tags)
    {
        if (tname.first == prevTag)
        {
            continue;  // skip subsequent tags with different types
        }
        publications.push_back (helics::Publication (helics::GLOBAL, vFed.get (), tname.first,
                                                     helics::getTypeFromString (tname.second)));
        prevTag = tname.first;
        pubids[tname.first] = static_cast<int> (publications.size ()) - 1;
    }
    // load up the ids
    for (auto &vs : points)
    {
        vs.index = pubids[vs.pubName];
    }

    std::sort (points.begin (), points.end (), vComp);

    int pointIndex = 0;
    std::cout << "entering init State\n";
    vFed->enterInitializationState ();
    std::cout << "entered init State\n";
    while (points[pointIndex].time < helics::timeZero)
    {
        publications[points[pointIndex].index].publish (points[pointIndex].value);
        ++pointIndex;
    }

    vFed->enterExecutionState ();
    std::cout << "entered exec State\n";
    while (points[pointIndex].time == helics::timeZero)
    {
        publications[points[pointIndex].index].publish (points[pointIndex].value);
        ++pointIndex;
    }
    helics::Time nextPrintTime = 10.0;
    while (pointIndex < static_cast<int> (points.size ()))
    {
        if (points[pointIndex].time > stopTime)
        {
            break;
        }
        auto newTime = vFed->requestTime (points[pointIndex].time);

        while (points[pointIndex].time <= newTime)
        {
            publications[points[pointIndex].index].publish (points[pointIndex].value);
            ++pointIndex;
            if (pointIndex == static_cast<int> (points.size ()))
            {
                break;
            }
        }
        if (newTime >= nextPrintTime)
        {
            std::cout << "processed time " << static_cast<double> (newTime) << "\n";
            nextPrintTime += 10.0;
        }
    }
    vFed->finalize ();

    return 0;
}

void argumentParser (int argc, const char *const *argv, po::variables_map &vm_map)
{
    po::options_description cmd_only ("command line only");
    po::options_description config ("configuration");
    po::options_description hidden ("hidden");

    // clang-format off
    // input boost controls
    cmd_only.add_options () 
		("help,h", "produce help message")
		("version,v","HELICS version number")
		("config-file", po::value<std::string> (),"specify a configuration file to use");


    config.add_options ()
		("broker,b", po::value<std::string> (),"address to connect the broker to")
		("name,n", po::value<std::string> (),"name of the player federate")
		("datatype",po::value<std::string>(),"type of the publication data type to use")
		("core,c",po::value<std::string> (),"type of the core to connect to")
		("stop", po::value<double>(), "the time to stop the player")
		("timedelta", po::value<double>(), "the time delta of the federate")
		("coreinit,i", po::value<std::string>(), "the core initialization string");

    // clang-format on

    hidden.add_options () ("input", po::value<std::string> (), "input file");

    po::options_description cmd_line ("command line options");
    po::options_description config_file ("configuration file options");
    po::options_description visible ("allowed options");

    cmd_line.add (cmd_only).add (config).add (hidden);
    config_file.add (config).add (hidden);
    visible.add (cmd_only).add (config);

    po::positional_options_description p;
    p.add ("input", -1);

    po::variables_map cmd_vm;
    try
    {
        po::store (po::command_line_parser (argc, argv).options (cmd_line).positional (p).run (), cmd_vm);
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
        std::cout << helics::getHelicsVersionString () << '\n';
        return;
    }

    po::store (po::command_line_parser (argc, argv).options (cmd_line).positional (p).run (), vm_map);

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
    // check to make sure we have some input file
    if (vm_map.count ("input") == 0)
    {
        std::cout << " no input file specified\n";
        std::cout << visible << '\n';
        return;
    }
}
