/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "player.h"
#include "PrecHelper.h"
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
#include "json.hpp"



namespace po = boost::program_options;
namespace filesystem = boost::filesystem;

static void playerArgumentParser (int argc, const char *const *argv, po::variables_map &vm_map);

static const std::regex creg (R"raw((-?\d+(\.\d+)?|\.\d+)\s*([^\s]*)(\s+[cCdDvVsSiIfF]?\s+|\s+)([^\s]*))raw");

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

    static inline bool vComp(ValueSetter &v1, ValueSetter &v2) { return (v1.time < v2.time); }



    player::player(int argc, char *argv[])
    {
        FederateInfo fi("player");
        fi.loadInfoFromArgs(argc, argv);
        fed = std::make_shared<CombinationFederate>(fi);

        boost::program_options::variables_map vm_map;
        playerArgumentParser(argc, argv,vm_map);
        loadArguments(vm_map);
    }

    player::player(const FederateInfo &fi) : fed(std::make_shared<CombinationFederate>(fi))
    {

    }
   
    player::player(std::shared_ptr<Core> core, const FederateInfo &fi):fed(std::make_shared<CombinationFederate>(std::move(core),fi))
    {

    }

    player::player(const std::string &jsonString):fed(std::make_shared<CombinationFederate>(jsonString))
    {
        if (jsonString.size() < 200)
        {
            masterFileName = jsonString;
        }
        auto pubCount = fed->getSubscriptionCount();
        for (int ii = 0; ii < pubCount; ++ii)
        {
            publications.emplace_back(fed.get(), ii);
            pubids[publications.back().getName()] = static_cast<int>(publications.size() - 1);

        }
        auto eptCount = fed->getEndpointCount();
        for (int ii = 0; ii < eptCount; ++ii)
        {
            endpoints.emplace_back(fed.get(), ii);
            eptids[endpoints.back().getName()] = static_cast<int>(endpoints.size() - 1);

        }
    }

    player::~player()
    {

    }

    void player::loadFile(const std::string &filename)
    {
        std::ifstream infile(filename);

        
        std::string str;

        int lcnt = 0;
        // count the lines
        while (std::getline(infile, str))
        {
            if (str.empty())
            {
                continue;
            }
            auto fc = str.find_first_not_of(" \t\n\r\0");
            if ((fc == std::string::npos) || (str[fc] == '#'))
            {
                continue;
            }
            ++lcnt;
        }
        points.resize(lcnt);
        // now start over and actual do the loading
        infile.close();
        infile.open(filename);
        int icnt = 0;
        std::smatch m;
        while (std::getline(infile, str))
        {
            if (str.empty())
            {
                continue;
            }
            auto fc = str.find_first_not_of(" \t\n\r\0");
            if ((fc == std::string::npos) || (str[fc] == '#'))
            {
                continue;
            }
            std::regex_search(str, m, creg);
            if (m.size() == 6)
            {
                points[icnt].time = helics::Time(std::stod(m[1]));
                points[icnt].pubName = m[3];
                points[icnt].type = m[4];
                points[icnt].value = m[5];
                ++icnt;
            }
        }
        // collapse tags to the reduced list
        for (auto &vs : points)
        {
            tags.emplace(vs.pubName, vs.type);
        }

        std::string prevTag;
        for (auto &tname : tags)
        {
            if (tname.first == prevTag)
            {
                continue;  // skip subsequent tags with different types
            }
            publications.push_back(helics::Publication(helics::GLOBAL, fed.get(), tname.first,
                helics::getTypeFromString(tname.second)));
            prevTag = tname.first;
            pubids[tname.first] = static_cast<int> (publications.size()) - 1;
        }
        // load up the ids
        for (auto &vs : points)
        {
            vs.index = pubids[vs.pubName];
        }
    }

    /*run the player*/
    void player::run()
    {
        std::sort(points.begin(), points.end(), vComp);

        int pointIndex = 0;
        std::cout << "entering init State\n";
        fed->enterInitializationState();
        std::cout << "entered init State\n";
        while (points[pointIndex].time < helics::timeZero)
        {
            publications[points[pointIndex].index].publish(points[pointIndex].value);
            ++pointIndex;
        }

        fed->enterExecutionState();
        std::cout << "entered exec State\n";
        while (points[pointIndex].time == helics::timeZero)
        {
            publications[points[pointIndex].index].publish(points[pointIndex].value);
            ++pointIndex;
        }
        helics::Time nextPrintTime = 10.0;
        while (pointIndex < static_cast<int> (points.size()))
        {
            if (points[pointIndex].time > stopTime)
            {
                break;
            }
            auto newTime = fed->requestTime(points[pointIndex].time);

            while (points[pointIndex].time <= newTime)
            {
                publications[points[pointIndex].index].publish(points[pointIndex].value);
                ++pointIndex;
                if (pointIndex == static_cast<int> (points.size()))
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
        fed->finalize();
    }

    void player::run(Time stopTime_input)
    {

    }

    void player::addPublication(const std::string &key, helicsType_t type, const std::string &units)
    {
        publications.push_back(Publication(GLOBAL, fed.get(), key, type, units));
    }

    int player::loadArguments(boost::program_options::variables_map &vm_map)
    {
        if (vm_map.count("input") == 0)
        {
            return (-1);
        }

        if (vm_map.count("datatype") > 0)
        {
            defType = helics::getTypeFromString(vm_map["datatype"].as<std::string>());
            if (defType == helics::helicsType_t::helicsInvalid)
            {
                std::cerr << vm_map["datatype"].as<std::string>() << " is not recognized as a valid type \n";
                return -3;
            }
        }

        if (!filesystem::exists(vm_map["input"].as<std::string>()))
        {
            std::cerr << vm_map["input"].as<std::string>() << " does not exist \n";
            return -3;
        }
        loadFile(vm_map["input"].as<std::string>());

      
        std::cout << "read file " << points.size() << " points for " << tags.size() << " tags \n";
       
        stopTime = Time::maxVal();
        if (vm_map.count("stop") > 0)
        {
            stopTime = vm_map["stop"].as<double>();
        }
        return 0;
    }

} //namespace helics

void playerArgumentParser (int argc, const char *const *argv, po::variables_map &vm_map)
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
		("datatype",po::value<std::string>(),"type of the publication data type to use")
		("stop", po::value<double>(), "the time to stop the player");

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

