/*
Copyright © 2017-2018, Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include "Source.hpp"
#include "PrecHelper.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <stdexcept>
#include <boost/filesystem.hpp>
#include "../common/argParser.h"

#include "../common/JsonProcessingFunctions.hpp"

#include "../common/base64.h"
#include "../common/stringOps.h"
#include "../core/helicsVersion.hpp"

namespace filesystem = boost::filesystem;

namespace helics
{
namespace apps
{
using namespace std::string_literals;
static const ArgDescriptors InfoArgs{
    { "stop", "the time to stop recording" }
};

Source::Source (int argc, char *argv[])
{

    variable_map vm_map;
    auto res = argumentParser(argc, argv, vm_map, InfoArgs, "input"s);
    if (res == versionReturn)
    {
        std::cout << helics::versionString << '\n';
    }
    if (res < 0)
    {
        deactivated = true;
        return;
    }
    FederateInfo fi("source");

    fi.loadInfoFromArgs(argc, argv);
    fed = std::make_shared<CombinationFederate>(fi);
    fed->setFlag(SOURCE_ONLY_FLAG);

    loadArguments(vm_map);
}

Source::Source (const FederateInfo &fi) : fed (std::make_shared<CombinationFederate> (fi))
{
    fed->setFlag (SOURCE_ONLY_FLAG);
}

Source::Source (const std::shared_ptr<Core> &core, const FederateInfo &fi)
    : fed (std::make_shared<CombinationFederate> (core, fi))
{
    fed->setFlag (SOURCE_ONLY_FLAG);
}

Source::Source (const std::string &jsonString) : fed (std::make_shared<CombinationFederate> (jsonString))
{
    fed->setFlag (SOURCE_ONLY_FLAG);

    loadJsonFile (jsonString);
}

Source::~Source () = default;



void Source::loadFile (const std::string &jsonFile)
{
    fed->registerInterfaces (jsonFile);

  //  auto pubCount = fed->getSubscriptionCount ();
  // for (int ii = 0; ii < pubCount; ++ii)
  //  {
   //     publications.emplace_back (fed.get (), ii);
   //    pubids[publications.back ().getName ()] = static_cast<int> (publications.size () - 1);
  // }
  //  auto eptCount = fed->getEndpointCount ();
  //  for (int ii = 0; ii < eptCount; ++ii)
  //  {
  //      endpoints.emplace_back (fed.get (), ii);
   //     eptids[endpoints.back ().getName ()] = static_cast<int> (endpoints.size () - 1);
  //  }

    auto doc = loadJsonString(jsonFile);




}


/*run the source*/
void Source::run ()
{
    run (stopTime);
    fed->finalize ();
}

void Source::run (Time stopTime_input)
{
    auto state = fed->getCurrentState ();
    if (state == Federate::op_states::startup)
    {
        initialize ();
    }
    if (state != Federate::op_states::execution)
    {
        // send stuff before timeZero



        fed->enterExecutionState ();
        // send the stuff at timeZero


    }
    else
    {
//        auto ctime = fed->getCurrentTime ();


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

void Source::addSource(const std::string & /*key*/, helics_type_t /*type*/, const std::string &/*units*/)
{
    // skip already existing publications
 //   if (pubids.find (key) != pubids.end ())
  //  {
   //     std::cerr << "publication already exists\n";
   // }
  //  publications.push_back (Publication (GLOBAL, fed.get (), key, type, units));
//    pubids[key] = static_cast<int> (publications.size ()) - 1;
}


int Source::loadArguments (boost::program_options::variables_map &vm_map)
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

 //   std::cout << "read file " << points.size () << " points for " << tags.size () << " tags \n";

    stopTime = Time::maxVal ();
    if (vm_map.count ("stop") > 0)
    {
        stopTime = vm_map["stop"].as<double> ();
    }
    return 0;
}

}  // namespace apps
} // namespace helics

