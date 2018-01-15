/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "echo.h"
#include "PrecHelper.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <stdexcept>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include "PrecHelper.h"
#include "json.hpp"

#include "../common/base64.h"
#include "../common/stringOps.h"

namespace po = boost::program_options;
namespace filesystem = boost::filesystem;

static void echoArgumentParser (int argc, const char *const *argv, po::variables_map &vm_map);

// static const std::regex creg
// (R"raw((-?\d+(\.\d+)?|\.\d+)[\s,]*([^\s]*)(\s+[cCdDvVsSiIfF]?\s+|\s+)([^\s]*))raw");



namespace helics
{

echo::echo (int argc, char *argv[])
{
    FederateInfo fi ("echo");
    fi.loadInfoFromArgs (argc, argv);
    fed = std::make_shared<MessageFederate> (fi);
    boost::program_options::variables_map vm_map;
    echoArgumentParser (argc, argv, vm_map);
    loadArguments (vm_map);
}

echo::echo (const FederateInfo &fi) : fed (std::make_shared<MessageFederate> (fi))
{

}

echo::echo (std::shared_ptr<Core> core, const FederateInfo &fi)
    : fed (std::make_shared<MessageFederate> (std::move (core), fi))
{

}

echo::echo (const std::string &jsonString) : fed (std::make_shared<MessageFederate> (jsonString))
{
    loadFile (jsonString);
}

echo::~echo () = default;

void echo::loadFile (const std::string &filename)
{

    fed->registerInterfaces (filename);

    auto eptCount = fed->getEndpointCount ();
    for (int ii = 0; ii < eptCount; ++ii)
    {
        endpoints.emplace_back (fed.get (), ii);
    }

}


void echo::initialize ()
{
    auto state = fed->getCurrentState ();
    if (state == Federate::op_states::startup)
    {
        fed->enterInitializationState ();
    }
}


/*run the echo*/
void echo::run ()
{
    run (stopTime);
    fed->finalize ();
}

void echo::run (Time stopTime_input)
{
    auto state = fed->getCurrentState ();
    if (state == Federate::op_states::startup)
    {
        initialize ();
    }
    if (state != Federate::op_states::execution)
    {
     
        fed->enterExecutionState ();
        // send the stuff at timeZero
        
    }
    else
    {
        //auto ctime = fed->getCurrentTime ();
       
    }

    helics::Time nextPrintTime = 10.0;
    bool moreToSend = true;
    Time nextSendTime = timeZero;
    while (moreToSend)
    {
        nextSendTime = Time::maxVal ();
      
        if (nextSendTime > stopTime_input)
        {
            break;
        }
        if (nextSendTime == Time::maxVal ())
        {
            moreToSend = false;
            continue;
        }
        auto newTime = fed->requestTime (nextSendTime);
        
        if (newTime >= nextPrintTime)
        {
            std::cout << "processed time " << static_cast<double> (newTime) << "\n";
            nextPrintTime += 10.0;
        }
    }
}


void echo::addEndpoint (const std::string &endpointName, const std::string &endpointType)
{
    endpoints.push_back (Endpoint (GLOBAL, fed.get (), endpointName, endpointType));
}

int echo::loadArguments (boost::program_options::variables_map &vm_map)
{
    if (vm_map.count ("input") == 0)
    {
        return (-1);
    }

    if (!filesystem::exists (vm_map["input"].as<std::string> ()))
    {
        std::cerr << vm_map["input"].as<std::string> () << " does not exist \n";
        return -3;
    }
    loadFile (vm_map["input"].as<std::string> ());

    stopTime = Time::maxVal ();
    if (vm_map.count ("stop") > 0)
    {
        stopTime = vm_map["stop"].as<double> ();
    }
    return 0;
}

}  // namespace helics

void echoArgumentParser (int argc, const char *const *argv, po::variables_map &vm_map)
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
        ("delay", po::value<double>(),"the delay with which the echo app echos message")
		("stop", po::value<double>(), "the time to stop the echo");

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
        po::store (po::command_line_parser (argc, argv).options (cmd_line).positional (p).allow_unregistered().run (), cmd_vm);
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

    po::store (po::command_line_parser (argc, argv).options (cmd_line).allow_unregistered().positional (p).run (), vm_map);

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
