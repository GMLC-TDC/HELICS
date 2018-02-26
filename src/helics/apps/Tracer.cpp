/*
Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.
*/

#include "../application_api/Filters.hpp"
#include "../application_api/Subscriptions.hpp"
#include "../application_api/ValueFederate.hpp"
#include "../application_api/queryFunctions.hpp"
#include "../common/stringOps.h"
#include "../core/helicsVersion.hpp"
#include <algorithm>
#include <iostream>
#include <map>
#include <memory>
#include <regex>
#include <set>
#include <stdexcept>
#include <boost/filesystem.hpp>
#include "../common/argParser.h"

#include "../common/JsonProcessingFunctions.hpp"
#include "PrecHelper.hpp"
#include "Tracer.hpp"
#include <thread>

namespace filesystem = boost::filesystem;

namespace helics
{
Tracer::Tracer (FederateInfo &fi) : fed (std::make_shared<CombinationFederate> (fi))
{
    fed->setFlag (OBSERVER_FLAG);
}

static const ArgDescriptors InfoArgs{
    {"stop", "the time to stop tracing"},
    {"tags",ArgDescriptor::arg_type_t::vector_string,"tags to record, this argument may be specified any number of times"},
    {"sourceclone", ArgDescriptor::arg_type_t::vector_string, "existing endpoints to capture generated packets from, this argument may be specified multiple time"},
    {"destclone", ArgDescriptor::arg_type_t::vector_string, "existing endpoints to capture all packets with the specified endpoint as a destination, this argument may be specified multiple time"},
    {"clone", ArgDescriptor::arg_type_t::vector_string, "existing endpoints to clone all packets to and from"},
    {"capture", ArgDescriptor::arg_type_t::vector_string,"capture all the publications and endpoints of a particular federate capture=\"fed1;fed2\"  supports multiple arguments or a comma separated list"},
};

Tracer::Tracer (int argc, char *argv[])
{
    variable_map vm_map;
    auto res = argumentParser(argc, argv, vm_map, InfoArgs, "input");
    if (res == versionReturn)
    {
        std::cout << helics::versionString() << '\n';
    }
    if (res < 0)
    {
        deactivated = true;
        return;
    }
    FederateInfo fi ("tracer");

    fi.loadInfoFromArgs (argc, argv);
    fed = std::make_shared<CombinationFederate> (fi);
    fed->setFlag (OBSERVER_FLAG);

    loadArguments (vm_map);
}

Tracer::Tracer (const std::shared_ptr<Core> &core, const FederateInfo &fi)
    : fed (std::make_shared<CombinationFederate> (core, fi))
{
    fed->setFlag (OBSERVER_FLAG);
}

Tracer::Tracer (const std::string &jsonString) : fed (std::make_shared<CombinationFederate> (jsonString))
{
    fed->setFlag (OBSERVER_FLAG);
    loadJsonFile (jsonString);
}

Tracer::~Tracer() = default;

int Tracer::loadFile (const std::string &filename)
{
    auto ext = filesystem::path (filename).extension ().string ();
    if ((ext == ".json") || (ext == ".JSON"))
    {
        return loadJsonFile (filename);
    }
    else
    {
        return loadTextFile (filename);
    }
}

int Tracer::loadJsonFile (const std::string &jsonString)
{
    fed->registerInterfaces (jsonString);

    auto subCount = fed->getSubscriptionCount ();
    for (int ii = 0; ii < subCount; ++ii)
    {
        subscriptions.emplace_back (fed.get (), ii);
        subids.emplace (subscriptions.back ().getID (), static_cast<int> (subscriptions.size ()) - 1);
        subkeys.emplace (subscriptions.back ().getName (), static_cast<int> (subscriptions.size ()) - 1);
    }

    auto doc = loadJsonString (jsonString);

    auto tags = doc["tag"];
    if (tags.isArray ())
    {
        for (const auto &tag : tags)
        {
            addSubscription (tag.asString ());
        }
    }
    else if (tags.isString ())
    {
        addSubscription (tags.asString ());
    }
    auto sourceClone = doc["sourceclone"];
    if (sourceClone.isArray ())
    {
        for (const auto &sc : sourceClone)
        {
            addSourceEndpointClone (sc.asString ());
        }
    }
    else if (sourceClone.isString ())
    {
        addSourceEndpointClone (sourceClone.asString());
    }
    auto destClone = doc["destclone"];
    if (destClone.isArray ())
    {
        for (const auto &dc : destClone)
        {
            addDestEndpointClone (dc.asString ());
        }
    }
    else if (destClone.isString ())
    {
        addDestEndpointClone (destClone.asString ());
    }
    auto clones = doc["clone"];
    if (clones.isArray ())
    {
        for (const auto &clone : clones)
        {
            addSourceEndpointClone (clone.asString ());
            addDestEndpointClone (clone.asString ());
        }
    }
    else if (clones.isString ())
    {
        addSourceEndpointClone (clones.asString ());
        addDestEndpointClone (clones.asString ());
    }
    auto captures = doc["capture"];
    if (captures.isArray ())
    {
        for (const auto &capture : captures)
        {
            addCapture (capture.asString ());
        }
    }
    else if (captures.isString ())
    {
        addCapture (captures.asString ());
    }

    return 0;
}

int Tracer::loadTextFile (const std::string &textFile)
{
    using namespace stringOps;

    std::ifstream infile (textFile);
    std::string str;
    int lc = 0;
    while (std::getline (infile, str))
    {
        ++lc;
        if (str.empty ())
        {
            continue;
        }
        auto fc = str.find_first_not_of (" \t\n\r\0");
        if ((fc == std::string::npos) || (str[fc] == '#'))
        {
            continue;
        }
        auto blk = splitlineQuotes (str, ",\t ", default_quote_chars, delimiter_compression::on);

        switch (blk.size ())
        {
        case 1:
            addSubscription (removeQuotes (blk[0]));
            break;
        case 2:
            if ((blk[0] == "subscription") || (blk[0] == "s") || (blk[0] == "sub") || (blk[0] == "tag"))
            {
                addSubscription (removeQuotes (blk[1]));
            }
            else if ((blk[0] == "endpoint") || (blk[0] == "ept") || (blk[0] == "e"))
            {
               //mainly here so the same files work with recorder as tracer so ignore this line
            }
            else if ((blk[0] == "sourceclone") || (blk[0] == "source") || (blk[0] == "src"))
            {
                addSourceEndpointClone (removeQuotes (blk[1]));
            }
            else if ((blk[0] == "destclone") || (blk[0] == "dest") || (blk[0] == "destination"))
            {
                addDestEndpointClone (removeQuotes (blk[1]));
            }
            else if (blk[0] == "capture")
            {
                addCapture (removeQuotes (blk[1]));
            }
            else if (blk[0] == "clone")
            {
                addSourceEndpointClone (removeQuotes (blk[1]));
                addDestEndpointClone (removeQuotes (blk[1]));
            }
            else
            {
                std::cerr << "Unable to process line " << lc << ':' << str << '\n';
            }
            break;
        case 3:
            if (blk[0] == "clone")
            {
                if ((blk[1] == "source") || (blk[1] == "src"))
                {
                    addSourceEndpointClone (removeQuotes (blk[2]));
                }
                else if ((blk[1] == "dest") || (blk[1] == "destination"))
                {
                    addDestEndpointClone (removeQuotes (blk[2]));
                }
                else
                {
                    std::cerr << "Unable to process line " << lc << ':' << str << '\n';
                }
            }
            else
            {
                std::cerr << "Unable to process line " << lc << ':' << str << '\n';
            }
            break;
        default:
            break;
        }
    }
    infile.close ();
    return 0;
}

void Tracer::initialize ()
{
    generateInterfaces ();

    fed->enterInitializationState ();
    captureForCurrentTime (-1.0);

    fed->enterExecutionState ();
    captureForCurrentTime (0.0);
}

void Tracer::finalize () { fed->finalize (); }

void Tracer::generateInterfaces ()
{
    for (auto &tag : subkeys)
    {
        if (tag.second == -1)
        {
            addSubscription (tag.first);
        }
    }
    
    loadCaptureInterfaces ();
}

void Tracer::loadCaptureInterfaces ()
{
    for (auto &capt : captureInterfaces)
    {
        auto res = waitForInit (fed.get (), capt);
        if (res)
        {
            auto pubs = vectorizeQueryResult (fed->query (capt, "publications"));
            for (auto &pub : pubs)
            {
                addSubscription (pub);
            }
        }
    }
}

void Tracer::captureForCurrentTime (Time currentTime)
{
    for (auto &sub : subscriptions)
    {
        if (sub.isUpdated ())
        {
            auto val = sub.getValue<std::string> ();
            std::cout << '[' << currentTime << ']' << sub.getKey() << '=';
            if (val.size() < 150)
            {
                std::cout  << val << '\n';
            }
            else
            {
                std::cout << "block[" << val.size() << "]\n";
            }
            
        }
    }

  
    // get the clone endpoints
    if (cloneEndpoint)
    {
        while (cloneEndpoint->hasMessage ())
        {
            auto mess = cloneEndpoint->getMessage();
            std::cout << '[' << currentTime << "]message from " << mess->source << " to " << mess->original_dest << "::";
            if (mess->data.size()<50)
            {
                std::cout << mess->data.to_string() << '\n';
            }
            else
            {
                std::cout << "size " <<mess->data.size()<< '\n';
            }
        }
    }
}

std::string Tracer::encode (const std::string &str2encode) { return str2encode; }

/*run the Player*/
void Tracer::run ()
{
    run (autoStopTime);
    fed->finalize ();
}
/** run the Player until the specified time*/
void Tracer::run (Time runToTime)
{
    initialize ();
   
    Time nextPrintTime = 10.0;
    try
    {
        while (true)
        {
            auto T = fed->requestTime (runToTime);
            if (T < runToTime)
            {
                captureForCurrentTime (T);
               
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
}
/** add a subscription to record*/
void Tracer::addSubscription (const std::string &key)
{
    auto res = subkeys.find (key);
    if ((res == subkeys.end ()) || (res->second == -1))
    {
        subscriptions.push_back (helics::Subscription (fed.get (), key));
        auto index = static_cast<int> (subscriptions.size ()) - 1;
        auto id = subscriptions.back ().getID ();
        subids[id] = index;  // this is a new element
        subkeys[key] = index;  // this is a potential replacement
    }
}

void Tracer::addSourceEndpointClone (const std::string &sourceEndpoint)
{
    if (!cFilt)
    {
        cFilt = std::make_unique<CloningFilter> (fed.get ());
        cloneEndpoint = std::make_unique<Endpoint> (fed.get (), "cloneE");
        cFilt->addDeliveryEndpoint (cloneEndpoint->getName ());
    }
    cFilt->addSourceTarget (sourceEndpoint);
}

void Tracer::addDestEndpointClone (const std::string &destEndpoint)
{
    if (!cFilt)
    {
        cFilt = std::make_unique<CloningFilter> (fed.get ());
        cloneEndpoint = std::make_unique<Endpoint> (fed.get (), "cloneE");
        cFilt->addDeliveryEndpoint (cloneEndpoint->getName ());
    }
    cFilt->addDestinationTarget (destEndpoint);
}

void Tracer::addCapture (const std::string &captureDesc) { captureInterfaces.push_back (captureDesc); }

int Tracer::loadArguments (boost::program_options::variables_map &vm_map)
{
    if (vm_map.count ("input") == 0)
    {
        return -1;
    }

    if (!filesystem::exists (vm_map["input"].as<std::string> ()))
    {
        std::cerr << vm_map["input"].as<std::string> () << "is not a valid input file \n";
        return -3;
    }
    loadFile (vm_map["input"].as<std::string> ());

    // get the extra tags from the arguments
    if (vm_map.count ("tags") > 0)
    {
        auto argTags = vm_map["tags"].as<std::vector<std::string>> ();
        for (const auto &tag : argTags)
        {
            auto taglist = stringOps::splitlineQuotes (tag);
            for (const auto &tagname : taglist)
            {
                subkeys.emplace (stringOps::removeQuotes (tagname), -1);
            }
        }
    }

    // capture the all the publications from a particular federate
    if (vm_map.count ("capture") > 0)
    {
        auto captures = vm_map["capture"].as<std::vector<std::string>> ();
        for (const auto &capt : captures)
        {
            auto captFeds = stringOps::splitlineQuotes (capt);
            for (auto &captFed : captFeds)
            {
                auto actCapt = stringOps::removeQuotes (captFed);
                captureInterfaces.push_back (actCapt);
            }
        }
    }

    if (vm_map.count ("clone") > 0)
    {
        auto clones = vm_map["clone"].as<std::vector<std::string>> ();
        for (const auto &clone : clones)
        {
            addDestEndpointClone (clone);
            addSourceEndpointClone (clone);
        }
    }

    if (vm_map.count ("sourceclone") > 0)
    {
        auto clones = vm_map["sourceclone"].as<std::vector<std::string>> ();
        for (const auto &clone : clones)
        {
            addSourceEndpointClone (clone);
        }
    }

    if (vm_map.count ("destclone") > 0)
    {
        auto clones = vm_map["destclone"].as<std::vector<std::string>> ();
        for (const auto &clone : clones)
        {
            addDestEndpointClone (clone);
        }
    }

    if (vm_map.count("stop") > 0)
    {
        autoStopTime = loadTimeFromString(vm_map["stop"].as<std::string>());
    }
    return 0;
}
}
