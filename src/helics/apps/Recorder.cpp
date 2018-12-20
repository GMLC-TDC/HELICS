/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#include "Recorder.hpp"

#include "../application_api/Filters.hpp"
#include "../application_api/queryFunctions.hpp"
#include "../common/stringOps.h"

#include "../common/JsonProcessingFunctions.hpp"
#include "../common/argParser.h"
#include "../common/base64.h"
#include "../common/fmt_format.h"
#include "../common/fmt_ostream.h"
#include "../common/logger.h"
#include "PrecHelper.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <regex>
#include <set>
#include <stdexcept>
#include <thread>
#include "boost/filesystem.hpp"

namespace filesystem = boost::filesystem;

namespace helics
{
namespace apps
{
Recorder::Recorder (const std::string &appName, FederateInfo &fi) : App (appName, fi)
{
    fed->setFlagOption (helics_flag_observer);
}

static const ArgDescriptors InfoArgs{
  {"tags", ArgDescriptor::arg_type_t::vector_string,
   "tags to record, this argument may be specified any number of times"},
  {"endpoints", ArgDescriptor::arg_type_t::vector_string,
   "endpoints to capture, this argument may be specified multiple time"},
  {"sourceclone", ArgDescriptor::arg_type_t::vector_string,
   "existing endpoints to capture generated packets from, this argument may be specified multiple time"},
  {"destclone", ArgDescriptor::arg_type_t::vector_string,
   "existing endpoints to capture all packets with the specified endpoint as a destination, this argument may be "
   "specified multiple time"},
  {"clone", ArgDescriptor::arg_type_t::vector_string, "existing endpoints to clone all packets to and from"},
  {"capture", ArgDescriptor::arg_type_t::vector_string,
   "capture all the publications of a particular federate capture=\"fed1;fed2\"  supports multiple arguments or a "
   "comma separated list"},
  {"output,o", "the output file for recording the data"},
  {"allow_iteration", ArgDescriptor::arg_type_t::flag_type, "allow iteration on values"},
  {"verbose", ArgDescriptor::arg_type_t::flag_type, "print all value results to the screen"},
  {"marker", "print a statement indicating time advancement every <arg> period during the simulation"},
  {"mapfile", "write progress to a map file for concurrent progress monitoring"}};

Recorder::Recorder (int argc, char *argv[]) : App ("recorder", argc, argv)
{
    variable_map vm_map;
    if (!deactivated)
    {
        fed->setFlagOption (helics_flag_observer);
        argumentParser (argc, argv, vm_map, InfoArgs);
        loadArguments (vm_map);
        if (!masterFileName.empty ())
        {
            loadFile (masterFileName);
        }
    }
    else
    {
        argumentParser (argc, argv, vm_map, InfoArgs);
    }
}

Recorder::Recorder (const std::string &appName, const std::shared_ptr<Core> &core, const FederateInfo &fi)
    : App (appName, core, fi)
{
    fed->setFlagOption (helics_flag_observer);
}

Recorder::Recorder (const std::string &appName, const std::string &jsonString) : App (appName, jsonString)
{
    fed->setFlagOption (helics_flag_observer);
    Recorder::loadJsonFile (jsonString);
}

Recorder::~Recorder () { saveFile (outFileName); }

void Recorder::loadJsonFile (const std::string &jsonString)
{
    loadJsonFileConfiguration ("recorder", jsonString);

    auto subCount = fed->getInputCount ();
    for (int ii = 0; ii < subCount; ++ii)
    {
        subscriptions.emplace_back (fed->getInput (ii));
        subids.emplace (subscriptions.back ().getHandle (), static_cast<int> (subscriptions.size ()) - 1);
        subkeys.emplace (subscriptions.back ().getTarget (), static_cast<int> (subscriptions.size ()) - 1);
    }
    auto eptCount = fed->getEndpointCount ();
    for (int ii = 0; ii < eptCount; ++ii)
    {
        endpoints.emplace_back (fed->getEndpoint (ii));
        eptNames[endpoints.back ().getName ()] = static_cast<int> (endpoints.size () - 1);
        eptids.emplace (endpoints.back ().getHandle (), static_cast<int> (endpoints.size () - 1));
    }

    auto doc = loadJson (jsonString);

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
        addSourceEndpointClone (sourceClone.asString ());
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
}

void Recorder::loadTextFile (const std::string &textFile)
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
                addEndpoint (removeQuotes (blk[1]));
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
}

void Recorder::writeJsonFile (const std::string &filename)
{
    Json_helics::Value doc;
    if (!points.empty ())
    {
        doc["points"] = Json_helics::Value (Json_helics::arrayValue);
        for (auto &v : points)
        {
            Json_helics::Value point;
            point["key"] = subscriptions[v.index].getTarget ();
            point["value"] = v.value;
            point["time"] = static_cast<double> (v.time);
            if (v.iteration > 0)
            {
                point["iteration"] = v.iteration;
            }
            if (v.first)
            {
                point["type"] = subscriptions[v.index].getPublicationType ();
            }
            doc["points"].append (point);
        }
    }

    if (!messages.empty ())
    {
        doc["messages"] = Json_helics::Value (Json_helics::arrayValue);
        for (auto &mess : messages)
        {
            Json_helics::Value message;
            message["time"] = static_cast<double> (mess->time);
            message["src"] = mess->source;
            if ((!mess->original_source.empty ()) && (mess->original_source != mess->source))
            {
                message["original_source"] = mess->original_source;
            }
            if ((mess->dest.size () < 7) || (mess->dest.compare (mess->dest.size () - 6, 6, "cloneE") != 0))
            {
                message["dest"] = mess->dest;
                message["orig_dest"] = mess->original_dest;
            }
            else
            {
                message["dest"] = mess->original_dest;
            }
            if (isBinaryData (mess->data))
            {
                message["encoding"] = "base64";
                message["message"] = encode (mess->data.to_string ());
            }
            else
            {
                message["message"] = mess->data.to_string ();
            }
            doc["messages"].append (message);
        }
    }

    std::ofstream o (filename);
    o << doc << std::endl;
}

void Recorder::writeTextFile (const std::string &filename)
{
    std::ofstream outFile (filename.empty () ? outFileName : filename);
    if (!points.empty ())
    {
        outFile << "#time \ttag\t value\t type*\n";
    }
    for (auto &v : points)
    {
        if (v.first)
        {
            outFile << static_cast<double> (v.time) << "\t\t" << subscriptions[v.index].getTarget () << '\t'
                    << v.value << '\t' << subscriptions[v.index].getPublicationType () << '\n';
        }
        else
        {
            if (v.iteration > 0)
            {
                outFile << static_cast<double> (v.time) << ':' << v.iteration << "\t\t"
                        << subscriptions[v.index].getTarget () << '\t' << v.value << '\n';
            }
            else
            {
                outFile << static_cast<double> (v.time) << "\t\t" << subscriptions[v.index].getTarget () << '\t'
                        << v.value << '\n';
            }
        }
    }
    if (!messages.empty ())
    {
        outFile << "# m\t time \tsource\t dest\t message\n";
    }
    for (auto &m : messages)
    {
        outFile << "m\t" << static_cast<double> (m->time) << '\t' << m->source << '\t';
        if ((m->dest.size () < 7) || (m->dest.compare (m->dest.size () - 6, 6, "cloneE") != 0))
        {
            outFile << m->dest;
        }
        else
        {
            outFile << m->original_dest;
        }
        if (isBinaryData (m->data))
        {
            outFile << "\t\"" << encode (m->data.to_string ()) << "\"\n";
        }
        else
        {
            outFile << "\t\"" << m->data.to_string () << "\"\n";
        }
    }
}

void Recorder::initialize ()
{
    generateInterfaces ();

    vStat.resize (subids.size ());
    for (auto &val : subkeys)
    {
        vStat[val.second].key = val.first;
    }

    fed->enterInitializingMode ();
    captureForCurrentTime (-1.0);

    fed->enterExecutingMode ();
    captureForCurrentTime (0.0);
}

void Recorder::generateInterfaces ()
{
    for (auto &tag : subkeys)
    {
        if (tag.second == -1)
        {
            addSubscription (tag.first);
        }
    }
    for (auto &ept : eptNames)
    {
        if (ept.second == -1)
        {
            addEndpoint (ept.first);
        }
    }
    loadCaptureInterfaces ();
}

void Recorder::loadCaptureInterfaces ()
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

void Recorder::captureForCurrentTime (Time currentTime, int iteration)
{
    static auto logger = LoggerManager::getLoggerCore ();
    for (auto &sub : subscriptions)
    {
        if (sub.isUpdated ())
        {
            auto val = sub.getValue<std::string> ();
            int ii = subids[sub.getHandle ()];
            points.emplace_back (currentTime, ii, val);
            if (iteration > 0)
            {
                points.back ().iteration = iteration;
            }
            if (verbose)
            {
                std::string valstr;
                if (val.size () < 150)
                {
                    if (iteration > 0)
                    {
                        valstr = fmt::format ("[{}:{}]value {}={}", currentTime, iteration, sub.getTarget (), val);
                    }
                    else
                    {
                        valstr = fmt::format ("[{}]value {}={}", currentTime, sub.getTarget (), val);
                    }
                }
                else
                {
                    if (iteration > 0)
                    {
                        valstr = fmt::format ("[{}:{}]value {}=block[{}]", currentTime, iteration,
                                              sub.getTarget (), val.size ());
                    }
                    else
                    {
                        valstr =
                          fmt::format ("[{}]value {}=block[{}]", currentTime, sub.getTarget (), val.size ());
                    }
                }
                logger->addMessage (std::move (valstr));
            }
            if (vStat[ii].cnt == 0)
            {
                points.back ().first = true;
            }
            ++vStat[ii].cnt;
            vStat[ii].lastVal = val;
            vStat[ii].time = -1.0;
        }
    }

    for (auto &ept : endpoints)
    {
        while (ept.hasMessage ())
        {
            auto mess = ept.getMessage ();
            if (verbose)
            {
                std::string messstr;
                if (mess->data.size () < 50)
                {
                    messstr = fmt::format ("[{}]message from {} to {}::{}", currentTime, mess->source, mess->dest,
                                           mess->data.to_string ());
                }
                else
                {
                    messstr = fmt::format ("[{}]message from {} to {}:: size {}", currentTime, mess->source,
                                           mess->dest, mess->data.size ());
                }
                logger->addMessage (std::move (messstr));
            }
            messages.push_back (std::move (mess));
        }
    }
    // get the clone endpoints
    if (cloneEndpoint)
    {
        while (cloneEndpoint->hasMessage ())
        {
            messages.push_back (cloneEndpoint->getMessage ());
        }
    }
}

std::string Recorder::encode (const std::string &str2encode)
{
    return std::string ("b64[") +
           utilities::base64_encode (reinterpret_cast<const unsigned char *> (str2encode.c_str ()),
                                     static_cast<int> (str2encode.size ())) +
           ']';
}

/** run the Player until the specified time*/
void Recorder::runTo (Time runToTime)
{
    initialize ();
    if (!mapfile.empty ())
    {
        std::ofstream out (mapfile);
        for (auto &stat : vStat)
        {
            //    out << stat.key << "\t" << stat.cnt << '\t' << static_cast<double> (stat.time) << '\t' <<
            //    stat.lastVal
            //        << '\n';
            fmt::print (out, "{}\t{}\t{}\t{}\n", stat.key, stat.cnt, static_cast<double> (stat.time),
                        stat.lastVal);
        }
        out.flush ();
    }
    Time nextPrintTime = (nextPrintTimeStep > timeZero) ? nextPrintTimeStep : Time::maxVal ();
    try
    {
        while (true)
        {
            helics::Time T;
            int iteration = 0;
            if (allow_iteration)
            {
                auto ItRes = fed->requestTimeIterative (runToTime, iteration_request::iterate_if_needed);
                if (ItRes.state == iteration_result::next_step)
                {
                    iteration = 0;
                }
                T = ItRes.grantedTime;
                captureForCurrentTime (T, iteration);
                ++iteration;
            }
            else
            {
                T = fed->requestTime (runToTime);
                captureForCurrentTime (T);
            }
            if (!mapfile.empty ())
            {
                std::ofstream out (mapfile);
                for (auto &stat : vStat)
                {
                    fmt::print (out, "{}\t{}\t{}\t{}\n", stat.key, stat.cnt, static_cast<double> (stat.time),
                                stat.lastVal);
                }
                out.flush ();
            }
            if (T >= runToTime)
            {
                break;
            }
            if ((T >= nextPrintTime) && (nextPrintTimeStep > timeZero))
            {
                std::cout << "processed for time " << static_cast<double> (T) << "\n";
                nextPrintTime += nextPrintTimeStep;
            }
        }
    }
    catch (...)
    {
    }
}
/** add a subscription to record*/
void Recorder::addSubscription (const std::string &key)
{
    auto res = subkeys.find (key);
    if ((res == subkeys.end ()) || (res->second == -1))
    {
        subscriptions.emplace_back (fed->registerSubscription (key));
        auto index = static_cast<int> (subscriptions.size ()) - 1;
        auto id = subscriptions.back ().getHandle ();
        subids[id] = index;  // this is a new element
        subkeys[key] = index;  // this is a potential replacement
    }
}
/** add an endpoint*/
void Recorder::addEndpoint (const std::string &endpoint)
{
    auto res = eptNames.find (endpoint);
    if ((res == eptNames.end ()) || (res->second == -1))
    {
        endpoints.emplace_back (GLOBAL, fed.get (), endpoint);
        auto index = static_cast<int> (endpoints.size ()) - 1;
        auto id = endpoints.back ().getHandle ();
        eptids.emplace (id, index);  // this is a new element
        eptNames[endpoint] = index;  // this is a potential replacement
    }
}

void Recorder::addSourceEndpointClone (const std::string &sourceEndpoint)
{
    if (!cFilt)
    {
        cFilt = std::make_unique<CloningFilter> (fed.get ());
        cloneEndpoint = std::make_unique<Endpoint> (fed.get (), "cloneE");
        cFilt->addDeliveryEndpoint (cloneEndpoint->getName ());
    }
    cFilt->addSourceTarget (sourceEndpoint);
}

void Recorder::addDestEndpointClone (const std::string &destEndpoint)
{
    if (!cFilt)
    {
        cFilt = std::make_unique<CloningFilter> (fed.get ());
        cloneEndpoint = std::make_unique<Endpoint> (fed.get (), "cloneE");
        cFilt->addDeliveryEndpoint (cloneEndpoint->getName ());
    }
    cFilt->addDestinationTarget (destEndpoint);
}

void Recorder::addCapture (const std::string &captureDesc) { captureInterfaces.push_back (captureDesc); }

std::pair<std::string, std::string> Recorder::getValue (int index) const
{
    if (isValidIndex (index, points))
    {
        return {subscriptions[points[index].index].getTarget (), points[index].value};
    }
    return {std::string (), std::string ()};
}

std::unique_ptr<Message> Recorder::getMessage (int index) const
{
    if (isValidIndex (index, messages))
    {
        return std::make_unique<Message> (*messages[index]);
    }
    return nullptr;
}

/** save the data to a file*/
void Recorder::saveFile (const std::string &filename)
{
    auto ext = filesystem::path (filename).extension ().string ();
    if ((ext == ".json") || (ext == ".JSON"))
    {
        writeJsonFile (filename);
    }
    else
    {
        writeTextFile (filename);
    }
}

int Recorder::loadArguments (boost::program_options::variables_map &vm_map)
{
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
    // get the extra tags from the arguments
    if (vm_map.count ("endpoints") > 0)
    {
        auto argEpt = vm_map["endpoints"].as<std::vector<std::string>> ();
        for (const auto &ept : argEpt)
        {
            auto eptlist = stringOps::splitlineQuotes (ept);
            for (const auto &eptname : eptlist)
            {
                eptNames.emplace (stringOps::removeQuotes (eptname), -1);
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
    if (vm_map.count ("allow_iteration") > 0)
    {
        allow_iteration = true;
    }
    if (vm_map.count ("verbose") > 0)
    {
        verbose = true;
    }
    if (vm_map.count ("marker") > 0)
    {
        nextPrintTimeStep = loadTimeFromString (vm_map["marker"].as<std::string> ());
    }
    if (vm_map.count ("mapfile") > 0)
    {
        mapfile = vm_map["mapfile"].as<std::string> ();
    }

    outFileName = "out.txt";
    if (vm_map.count ("output") > 0)
    {
        outFileName = vm_map["output"].as<std::string> ();
    }
    return 0;
}
}  // namespace apps
}  // namespace helics
