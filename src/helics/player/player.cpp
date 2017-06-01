/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "helics/application_api/ValueFederate.h"
#include <algorithm>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <regex>
#include <set>
#include <stdexcept>
#include <string>

#include "PrecHelper.h"

class ValueSetter
{
  public:
    helics::Time time;
    helics::publication_id_t id;
    valueTypes_t type;
    std::string pubName;
    std::string value;
};


namespace po = boost::program_options;
namespace filesystem = boost::filesystem;

bool vComp (ValueSetter &v1, ValueSetter &v2) { return (v1.time < v2.time); }
void argumentParser (int argc, char *argv[], po::variables_map &vm_map);

void sendPublication (helics::ValueFederate *vFed, ValueSetter &vs);

const std::regex creg (R"raw((-?\d+(\.\d+)?|\.\d+)\s*([^\s]*)(\s+[cCdDvVsSiIfF]?\s+|\s+)([^\s]*))raw");

int main (int argc, char *argv[])
{
    std::ifstream infile;
    valueTypes_t defType = valueTypes_t::stringValue;

    po::variables_map vm;
    argumentParser (argc, argv, vm);


    std::vector<ValueSetter> points;
    if (vm.count ("input") == 0)
    {
        return 0;
    }

    if (vm.count ("type") > 0)
    {
        defType = getType (vm["type"].as<std::string> ());
        if (defType == valueTypes_t::unknownValue)
        {
            std::cerr << vm["type"].as<std::string> () << "is not recognized as a valid type \n";
            return -3;
        }
    }

    if (!filesystem::exists (vm["input"].as<std::string> ()))
    {
        std::cerr << vm["input"].as<std::string> () << "does not exist \n";
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
            points[icnt].type = getType (m[4]);
            if (points[icnt].type == valueTypes_t::unknownValue)
            {
                points[icnt].type = defType;
            }
            points[icnt].value = m[5];
            ++icnt;
        }
    }
    // collapse tags to the reduced list
    std::set<std::pair<std::string, valueTypes_t>> tags;
    for (auto &vs : points)
    {
        tags.emplace (vs.pubName, vs.type);
    }

    std::map<std::string, std::pair<helics::publication_id_t, valueTypes_t>> pubids;
    std::cout << "read file " << points.size () << " points for " << tags.size () << " tags \n";
    std::string name = "player";
    if (vm.count ("name") > 0)
    {
        name = vm["name"].as<std::string> ();
    }

    std::string corename = "";
    if (vm.count ("core") > 0)
    {
        corename = vm["core"].as<std::string> ();
    }
    helics::Time stopTime = helics::Time::maxVal ();
    if (vm.count ("stop") > 0)
    {
        stopTime = vm["stop"].as<double> ();
    }
    helics::FederateInfo fi (name);
    fi.coreType = corename;
    fi.coreInitString = "1";
    if (vm.count ("coreinit") > 0)
    {
        fi.coreInitString = vm["coreinit"].as<std::string> ();
    }
    fi.sourceOnly = true;
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
        auto id = vFed->registerGlobalPublication (tname.first, typeString (tname.second));
        pubids.emplace (tname.first, std::make_pair (id, tname.second));
        prevTag = tname.first;
    }
    // load up the ids
    for (auto &vs : points)
    {
        std::tie (vs.id, vs.type) = pubids[vs.pubName];
    }

    std::sort (points.begin (), points.end (), vComp);

    int pointIndex = 0;
    std::cout << "entering init State\n";
    vFed->enterInitializationState ();
    std::cout << "entered init State\n";
    while (points[pointIndex].time < helics::timeZero)
    {
        sendPublication (vFed.get (), points[pointIndex]);
        ++pointIndex;
    }

    vFed->enterExecutionState ();
    std::cout << "entered exec State\n";
    while (points[pointIndex].time == helics::timeZero)
    {
        sendPublication (vFed.get (), points[pointIndex]);
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
            sendPublication (vFed.get (), points[pointIndex]);
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


void sendPublication (helics::ValueFederate *vFed, ValueSetter &vs)
{
    switch (vs.type)
    {
    case valueTypes_t::stringValue:
    default:
        vFed->publish (vs.id, vs.value);
        break;
    case valueTypes_t::doubleValue:
    {
        try
        {
            double val = std::stod (vs.value);
            vFed->publish (vs.id, val);
        }
        catch (std::invalid_argument &)
        {
            std::cerr << " unable to convert " << vs.value << " to double";
        }
        break;
    }

    case valueTypes_t::complexValue:
    {
        auto cval = helicsGetComplex (vs.value);
        vFed->publish (vs.id, cval);
    }
    break;
    case valueTypes_t::int64Value:
    {
        try
        {
            int64_t val = std::stoll (vs.value);
            vFed->publish (vs.id, val);
        }
        catch (std::invalid_argument &)
        {
            std::cerr << " unable to convert " << vs.value << " to integer";
        }
        break;
    }
    case valueTypes_t::vectorValue:
        break;
    }
}

void argumentParser (int argc, char *argv[], po::variables_map &vm_map)
{
    po::options_description cmd_only ("command line only");
    po::options_description config ("configuration");
    po::options_description hidden ("hidden");

    // clang-format off
    // input boost controls
    cmd_only.add_options () 
		("help,h", "produce help message")
		("version,v","helics version number")
		("config-file", po::value<std::string> (),"specify a configuration file to use");


    config.add_options ()
		("broker,b", po::value<std::string> (),"address to connect the broker to")
		("name,n", po::value<std::string> (),"name of the player federate")
		("type,t",po::value<std::string>(),"type of the publication to use")
		("core,c",po::value<std::string> (),"name of the core to connect to")
		("stop", po::value<double>(), "the time to stop the player")
		("timedelta", po::value<double>(), "the time delta of the federate")
		("coreinit,i", po::value<std::string>(), "the core initializion string");

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
        std::cout << 0.1 << '\n';
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
