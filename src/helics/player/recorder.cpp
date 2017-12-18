/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "../application_api/Subscriptions.hpp"
#include "../application_api/ValueFederate.h"
#include "../application_api/queryFunctions.h"
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
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

#include "PrecHelper.h"
#include <thread>
#include "recorder.h"

namespace po = boost::program_options;
namespace filesystem = boost::filesystem;

static void recorderArgumentParser (int argc, const char *const *argv, po::variables_map &vm_map);

namespace helics
{
    recorder::recorder(FederateInfo &fi) : fed(std::make_shared<CombinationFederate>(fi))
    {

    }

    recorder::recorder(int argc, char *argv[])
    {
        FederateInfo fi("player");
        loadFederateInfo(fi, argc, argv);
        fed = std::make_shared<CombinationFederate>(fi);

        boost::program_options::variables_map vm_map;
        recorderArgumentParser(argc, argv, vm_map);
        loadArguments(vm_map);
    }


    recorder::recorder(std::shared_ptr<Core> core, const FederateInfo &fi) :fed(std::make_shared<CombinationFederate>(std::move(core), fi))
    {

    }

    recorder::recorder(const std::string &jsonString) :fed(std::make_shared<CombinationFederate>(jsonString))
    {
        //TODO:: PT load the information to record from a json string
    }

  
    recorder::~recorder()
    {
        saveFile(outFileName);
    }

    int recorder::loadFile(const std::string &filename)
    {
        std::ifstream infile(filename);
        std::string str;
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
            auto cloc = str.find_last_of(',');
            if (cloc != std::string::npos)
            {
                auto vtype = getType(str.substr(cloc + 1, std::string::npos));
                if (vtype == helics::helicsType_t::helicsInvalid)
                {
                    std::cerr << "unrecognized type " << str.substr(cloc + 1, std::string::npos) << "\n";
                    return -4;
                }
                auto tag = str.substr(0, cloc);
                tag.erase(tag.find_last_not_of(" \t\n\0") + 1);
                tag.erase(0, tag.find_first_not_of(" \t\n\0"));
                tags.emplace(tag);
            }
            else
            {
                auto tag = str;
                tag.erase(tag.find_last_not_of(" \t\n\0") + 1);
                tag.erase(0, tag.find_first_not_of(" \t\n\0"));
                tags.emplace(str);
            }
        }
        std::cout << tags.size() << " tags processed\n";

        infile.close();
        return 0;
    }

    /*run the player*/
    void recorder::run()
    {

    }
    /** run the player until the specified time*/
    void recorder::run(helics::Time stopTime)
    {
        std::vector<ValueStats> vStat;

        vStat.reserve(subids.size());
        for (auto &val : subids)
        {
            vStat.emplace_back(ValueStats());
            vStat.back().key = val.second.first;
        }
        points.reserve(100000);
        std::cout << "entering execution mode\n";
        fed->enterExecutionState();
        helics::Time nextPrintTime = 10.0;
        int ii = 0;
        for (auto &sub : subscriptions)
        {
            if (sub.isUpdated())
            {
                auto val = sub.getValue<std::string>();
                points.emplace_back(-1.0, sub.getID(), val);
                if (vStat[ii].cnt == 0)
                {
                   points.back().first = true;
                }
                ++vStat[ii].cnt;
                vStat[ii].lastVal = val;
                vStat[ii].time = -1.0;
            }
            ++ii;
        }
        if (!mapfile.empty())
        {
            std::ofstream out(mapfile);
            for (auto &stat : vStat)
            {
                out << stat.key << "\t" << stat.cnt << '\t' << static_cast<double> (stat.time) << '\t' << stat.lastVal
                    << '\n';
            }
            out.flush();
        }

        try
        {
            while (true)
            {
                auto T = fed->requestTime(stopTime);
                if (T < stopTime)
                {
                    ii = 0;
                    for (auto &sub : subscriptions)
                    {
                        if (sub.isUpdated())
                        {
                            auto val = sub.getValue<std::string>();
                            points.emplace_back(T, sub.getID(), val);
                            ++vStat[ii].cnt;
                            vStat[ii].lastVal = val;
                            vStat[ii].time = T;
                        }
                        ++ii;
                    }
                    if (!mapfile.empty())
                    {
                        std::ofstream out(mapfile);
                        for (auto &stat : vStat)
                        {
                            out << stat.key << "\t" << stat.cnt << '\t' << static_cast<double> (stat.time) << '\t'
                                << stat.lastVal << '\n';
                        }
                        out.flush();
                    }
                }
                else
                {
                    break;
                }
                if (T >= nextPrintTime)
                {
                    std::cout << "processed for time " << static_cast<double> (T) << "\n";
                    nextPrintTime += 10.0;
                }
            }
        }
        catch (...)
        {
        }

        for (auto &sub : subscriptions)
        {
            subids.emplace(sub.getID(), std::make_pair(sub.getKey(), sub.getType()));
        }
        fed->finalize();
        
    }
    /** add a subscription to capture*/
    void recorder::addSubscription(const std::string &key)
    {

    }
    /** add an endpoint*/
    void recorder::addEndpoint(const std::string &endpoint)
    {

    }
    
    void recorder::addSourceEndpointClone(const std::string &sourceEndpoint)
    {

    }
    
    void recorder::addDestEndpointClone(const std::string &destEndpoint)
    {

    }

    /** save the data to a file*/
    void recorder::saveFile(const std::string &filename)
    {
        std::ofstream outFile(filename.empty()?outFileName:filename);
        outFile << "#time \ttag\t value\t type*\n";
        for (auto &v : points)
        {
            if (v.first)
            {
                outFile << static_cast<double> (v.time) << "\t\t" << subids[v.id].first << '\t' << v.value << '\t'
                    << subids[v.id].second << '\n';
            }
            else
            {
                outFile << static_cast<double> (v.time) << "\t\t" << subids[v.id].first << '\t' << v.value << '\n';
            }
        }
    }

    int recorder::loadArguments(boost::program_options::variables_map &vm_map)
    {
        if (vm_map.count("input") == 0)
        {
            return -1;
        }

        if (!filesystem::exists(vm_map["input"].as<std::string>()))
        {
            std::cerr << vm_map["input"].as<std::string>() << "is not a valid input file \n";
            return -3;
        }
        loadFile(vm_map["input"].as<std::string>());

        // get the extra tags from the arguments
        if (vm_map.count("tags") > 0)
        {
            auto argTags = vm_map["tags"].as<std::vector<std::string>>();
            for (const auto &tag : argTags)
            {
                std::vector<std::string> taglist;
                boost::split(taglist, tag, boost::is_any_of(",;"));
                for (const auto &tagname : taglist)
                {
                    tags.insert(tagname);
                }
            }
        }

        std::string prevTag;
        for (auto &tname : tags)
        {
            subscriptions.push_back(helics::Subscription(fed.get(), tname));
        }
        // capture the all the publications from a particular federate
        if (vm_map.count("capture") > 0)
        {
            auto captures = vm_map["capture"].as<std::vector<std::string>>();
            for (const auto &capt : captures)
            {
                std::vector<std::string> captFeds;
                boost::split(captFeds, capt, boost::is_any_of(",;"));
                for (const auto &captFed : captFeds)
                {
                    auto res = waitForInit(fed.get(), captFed);
                    if (res)
                    {
                        auto pubs = vectorizeQueryResult(fed->query(captFed, "publications"));
                        for (auto &pub : pubs)
                        {
                            subscriptions.emplace_back(fed.get(), pub);
                        }
                    }
                }
            }
        }
        if (vm_map.count("mapfile") > 0)
        {
            mapfile = vm_map["mapfile"].as<std::string>();
        }

        outFileName = "out.txt";
        if (vm_map.count("output") > 0)
        {
            outFileName = vm_map["output"].as<std::string>();
        }

        return 0;
    }
    
}


void recorderArgumentParser(int argc, const char *const *argv, po::variables_map &vm_map)
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


    config.add_options()
        ("stop", po::value<double>(), "the time to stop recording")
        ("tags",po::value<std::vector<std::string>>(),"tags to record this argument may be specified any number of times")
        ("capture", po::value < std::vector<std::string>>(),"capture all the publications of a particular federate capture=\"fed1;fed2\"  supports multiple arguments or a comma separated list")
		("output,o",po::value<std::string>(),"the output file for recording the data")
		("mapfile", po::value<std::string>(), "write progress to a memory mapped file");

    hidden.add_options () ("input", po::value<std::string> (), "input file");
    // clang-format on

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
    // check to make sure we have some input file or the capture is specified
    if ((vm_map.count ("input") == 0) && (vm_map.count ("capture") == 0) && (vm_map.count ("tags") == 0))
    {
        std::cerr << " no input file, tags, or captures specified\n";
        std::cerr << visible << '\n';
        return;
    }
}
