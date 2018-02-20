/*
Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.
*/

#include "helics/core/CoreFactory.hpp"
#include "helics/application_api/Filters.hpp"
#include <iostream>
#include <thread>
#include "helics/common/argParser.h"

static const helics::ArgDescriptors InfoArgs{
    {"target,t", "the federate at to target"},
    {"endpoint,e", "name of the endpoint to filter"},
    {"delay", "the time to delay the message"},
{"filtertype","the type of filter to implement (delay,randomdrop,randomdelay"},
{"dropprob",helics::ArgDescriptor::arg_type_t::double_type,"the probability a message will be dropped, only used with filtertype=randomdrop"}

    //name is captured in the argument processor for federateInfo
};

int main (int argc, char *argv[])
{
    //process the command line arguments
    helics::variable_map vm;
    auto parseResult = argumentParser(argc, argv, vm, InfoArgs);
    if (parseResult != 0)
    {
        return 0;
    }

	std::string targetfederate = "fed";
	if (vm.count("target") > 0)
	{
		targetfederate = vm["target"].as<std::string>();
	}
    std::string targetEndpoint = "endpoint";
    if (vm.count("endpoint") > 0) {
        targetEndpoint = vm["endpoint"].as<std::string>();
    }
    std::string target = targetfederate + "/" + targetEndpoint;
    
    std::string filtType = "delay";
    if (vm.count("filtertype") > 0) {
        targetEndpoint = vm["filtertype"].as<std::string>();
    }

    helics::defined_filter_types ftype = helics::defined_filter_types::delay;
    if (filtType == "randomdrop")
    {
        ftype = helics::defined_filter_types::randomDrop;
    }
    else if (filtType == "randomdelay")
    {
        ftype = helics::defined_filter_types::randomDelay;
    }
    else if (filtType != "delay")
    {
        std::cerr << "invalid filter type specified valid types are \"delay\",\"random drop\",\"random delay\"\n";
        return (-2);
    }
    
   
    auto core = helics::CoreFactory::create(argc, argv);
	std::cout << " registering filter '"<< "' for " << target<<'\n';

    //create a source filter object with type, the fed pointer and a target endpoint
    auto filt = helics::make_source_filter(ftype, core.get(), target);
    
    // get a few specific parameters related to the particular filter
    switch (ftype)
    {
    case helics::defined_filter_types::delay:
    {
        std::string delay = "1.0";
        if (vm.count("delay") > 0) {
            delay = vm["delay"].as<std::string>();
            filt->setString("delay", delay);
        }
        break;
    }
    case helics::defined_filter_types::randomDrop:
    {
        double dropprob = 0.33;
        if (vm.count("dropprob") > 0) {
            dropprob = vm["dropprob"].as<double>();
        }
        filt->set("dropprob", dropprob);
    }
        break;
    case helics::defined_filter_types::randomDelay:
        filt->setString("distribution", "uniform");
        if (vm.count("delay") > 0)
        {
            filt->setString("max", vm["delay"].as<std::string>());
        }
    }
    /*setup and run
    */
    core->setCoreReadyToInit();
    
    //just do a wait loop while the core is still processing so the filters have time to work
    while (core->isConnected())
    {
        std::this_thread::yield();
    }
    return 0;
}
