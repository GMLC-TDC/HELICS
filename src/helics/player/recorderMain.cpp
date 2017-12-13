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

#include <boost/algorithm/string.hpp>

#include "PrecHelper.h"
#include <thread>

class ValueCapture
{
  public:
    helics::Time time;
    helics::subscription_id_t id;
    bool first = false;
    std::string value;
    ValueCapture () = default;
    ValueCapture (helics::Time t1, helics::subscription_id_t id1, const std::string &val)
        : time (t1), id (id1), value (val){};
};

class ValueStats
{
  public:
    helics::Time time = helics::Time::minVal ();
    std::string lastVal;
    std::string key;
    int cnt = 0;
};

namespace po = boost::program_options;
namespace filesystem = boost::filesystem;

void argumentParser (int argc, const char *const *argv, po::variables_map &vm_map);

int main (int argc, char *argv[])
{
    std::ifstream infile;
    po::variables_map vm;
    argumentParser (argc, argv, vm);

    std::vector<ValueCapture> points;
    if (vm.count ("input") == 0)
    {
        return 0;
    }

    if (!filesystem::exists (vm["input"].as<std::string> ()))
    {
        std::cerr << vm["input"].as<std::string> () << "is not a valid input file \n";
        return -3;
    }
    infile.open (vm["input"].as<std::string> ().c_str ());
    std::string str;
    std::set<std::string> tags;

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
        auto cloc = str.find_last_of (',');
        if (cloc != std::string::npos)
        {
            auto vtype = getType (str.substr (cloc + 1, std::string::npos));
            if (vtype == valueTypes_t::unknownValue)
            {
                std::cerr << "unrecognized type " << str.substr (cloc + 1, std::string::npos) << "\n";
                return -4;
            }
            auto tag = str.substr (0, cloc);
            tag.erase (tag.find_last_not_of (" \t\n\0") + 1);
            tag.erase (0, tag.find_first_not_of (" \t\n\0"));
            tags.emplace (tag);
        }
        else
        {
            auto tag = str;
            tag.erase (tag.find_last_not_of (" \t\n\0") + 1);
            tag.erase (0, tag.find_first_not_of (" \t\n\0"));
            tags.emplace (str);
        }
    }
    std::cout << tags.size () << " tags processed\n";

    infile.close ();

    std::map<helics::subscription_id_t, std::pair<std::string, std::string>> subids;

    std::vector<helics::Subscription> subscriptions;
    std::string name = "recorder";
    if (vm.count ("name") > 0)
    {
        name = vm["name"].as<std::string> ();
    }

    std::string corename;
    if (vm.count ("core") > 0)
    {
        corename = vm["core"].as<std::string> ();
    }

    helics::FederateInfo fi (name);
    try
    {
        fi.coreType = helics::coreTypeFromString (corename);
    }
    catch (std::invalid_argument &ia)
    {
        std::cerr << "Unrecognized core type\n";
        return (-1);
    }
    fi.coreInitString = "2";
    if (vm.count ("coreinit") > 0)
    {
        fi.coreInitString.push_back (' ');
        fi.coreInitString = vm["coreinit"].as<std::string> ();
    }
    if (vm.count ("broker") > 0)
    {
        fi.coreInitString += " --broker=";
        fi.coreInitString += vm["broker"].as<std::string> ();
    }
    fi.observer = true;
    if (vm.count ("timedelta") > 0)
    {
        fi.timeDelta = vm["timedelta"].as<double> ();
    }

    helics::Time stopTime = helics::Time::maxVal ();
    if (vm.count ("stop") > 0)
    {
        stopTime = vm["stop"].as<double> ();
    }
    auto vFed = std::make_unique<helics::ValueFederate> (fi);

    // get the extra tags from the arguments
    if (vm.count ("tags") > 0)
    {
        auto argTags = vm["tags"].as<std::vector<std::string>> ();
        for (const auto &tag : argTags)
        {
            std::vector<std::string> taglist;
            boost::split (taglist, tag, boost::is_any_of (",;"));
            for (const auto &tagname : taglist)
            {
                tags.insert (tagname);
            }
        }
    }

    std::string prevTag;
    for (auto &tname : tags)
    {
        subscriptions.push_back (helics::Subscription (vFed.get (), tname));
    }
    // capture the all the publications from a particular federate
    if (vm.count ("capture") > 0)
    {
        auto captures = vm["capture"].as<std::vector<std::string>> ();
        for (const auto &capt : captures)
        {
            std::vector<std::string> captFeds;
            boost::split (captFeds, capt, boost::is_any_of (",;"));
            for (const auto &captFed : captFeds)
            {
                auto res = waitForInit (vFed.get (), captFed);
                if (res)
                {
                    auto pubs = vectorizeQueryResult (vFed->query (captFed, "publications"));
                    for (auto &pub : pubs)
                    {
                        subscriptions.emplace_back (vFed.get (), pub);
                    }
                }
            }
        }
    }

    std::vector<ValueStats> vStat;
    vStat.reserve (subids.size ());
    for (auto &val : subids)
    {
        vStat.emplace_back (ValueStats ());
        vStat.back ().key = val.second.first;
    }

    std::string mapfile;
    if (vm.count ("mapfile") > 0)
    {
        mapfile = vm["mapfile"].as<std::string> ();
    }

    std::vector<ValueCapture> vcap;

    vcap.reserve (100000);
    std::cout << "entering execution mode\n";
    vFed->enterExecutionState ();
    helics::Time nextPrintTime = 10.0;
    int ii = 0;
    for (auto &sub : subscriptions)
    {
        if (sub.isUpdated ())
        {
            auto val = sub.getValue<std::string> ();
            vcap.emplace_back (-1.0, sub.getID (), val);
            if (vStat[ii].cnt == 0)
            {
                vcap.back ().first = true;
            }
            ++vStat[ii].cnt;
            vStat[ii].lastVal = val;
            vStat[ii].time = -1.0;
        }
        ++ii;
    }
    if (!mapfile.empty ())
    {
        std::ofstream out (mapfile);
        for (auto &stat : vStat)
        {
            out << stat.key << "\t" << stat.cnt << '\t' << static_cast<double> (stat.time) << '\t' << stat.lastVal
                << '\n';
        }
        out.flush ();
    }
    try
    {
        while (true)
        {
            auto T = vFed->requestTime (stopTime);
            if (T < stopTime)
            {
                ii = 0;
                for (auto &sub : subscriptions)
                {
                    if (sub.isUpdated ())
                    {
                        auto val = sub.getValue<std::string> ();
                        vcap.emplace_back (T, sub.getID (), val);
                        ++vStat[ii].cnt;
                        vStat[ii].lastVal = val;
                        vStat[ii].time = T;
                    }
                    ++ii;
                }
                if (!mapfile.empty ())
                {
                    std::ofstream out (mapfile);
                    for (auto &stat : vStat)
                    {
                        out << stat.key << "\t" << stat.cnt << '\t' << static_cast<double> (stat.time) << '\t'
                            << stat.lastVal << '\n';
                    }
                    out.flush ();
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
        subids.emplace (sub.getID (), std::make_pair (sub.getKey (), sub.getType ()));
    }
    vFed->finalize ();

    std::string outFileName = "out.txt";
    if (vm.count ("output") > 0)
    {
        outFileName = vm["output"].as<std::string> ();
    }
    std::ofstream outFile (outFileName);
    outFile << "#time \ttag\t value\t type*\n";
    for (auto &v : vcap)
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
		("version,v","helics version number")
		("config-file", po::value<std::string> (),"specify a configuration file to use");


    config.add_options()
        ("broker,b", po::value<std::string>(), "address of the broker to connect")
        ("name,n", po::value<std::string>(), "name of the player federate")
        ("core,c", po::value<std::string>(), "type of the core to connect to")
        ("stop", po::value<double>(), "the time to stop recording")
        ("tags",po::value<std::vector<std::string>>(),"tags to record this argument may be specified any number of times")
        ("timedelta", po::value<double>(), "the time delta of the federate")
        ("capture", po::value < std::vector<std::string>>(),"capture all the publications of a particular federate capture=\"fed1;fed2\"  supports multiple arguments or a comma separated list")
		("output,o",po::value<std::string>(),"the output file for recording the data")
		("coreinit,i", po::value<std::string>(), "the core initialization string")
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
