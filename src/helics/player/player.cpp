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

static void playerArgumentParser (int argc, const char *const *argv, po::variables_map &vm_map);

// static const std::regex creg
// (R"raw((-?\d+(\.\d+)?|\.\d+)[\s,]*([^\s]*)(\s+[cCdDvVsSiIfF]?\s+|\s+)([^\s]*))raw");

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
static inline bool vComp (const ValueSetter &v1, const ValueSetter &v2) { return (v1.time < v2.time); }
static inline bool mComp (const MessageHolder &m1, const MessageHolder &m2) { return (m1.sendTime < m2.sendTime); }

player::player (int argc, char *argv[])
{
    FederateInfo fi ("player");
    fi.loadInfoFromArgs (argc, argv);
    fed = std::make_shared<CombinationFederate> (fi);
    fed->setFlag (SOURCE_ONLY_FLAG);
    boost::program_options::variables_map vm_map;
    playerArgumentParser (argc, argv, vm_map);
    loadArguments (vm_map);
}

player::player (const FederateInfo &fi) : fed (std::make_shared<CombinationFederate> (fi))
{
    fed->setFlag (SOURCE_ONLY_FLAG);
}

player::player (std::shared_ptr<Core> core, const FederateInfo &fi)
    : fed (std::make_shared<CombinationFederate> (std::move (core), fi))
{
    fed->setFlag (SOURCE_ONLY_FLAG);
}

player::player (const std::string &jsonString) : fed (std::make_shared<CombinationFederate> (jsonString))
{
    fed->setFlag (SOURCE_ONLY_FLAG);
    if (jsonString.size () < 200)
    {
        masterFileName = jsonString;
    }
    loadJsonFile (jsonString);
}

player::~player () = default;

void player::addMessage (Time sendTime,
                         const std::string &src,
                         const std::string &dest,
                         const std::string &payload)
{
    messages.resize (messages.size () + 1);
    messages.back ().sendTime = sendTime;
    messages.back ().mess.data = payload;
    messages.back ().mess.src = src;
    messages.back ().mess.dest = dest;
    messages.back ().mess.time = sendTime;
}

void player::addMessage (Time sendTime,
                         Time actionTime,
                         const std::string &src,
                         const std::string &dest,
                         const std::string &payload)
{
    messages.resize (messages.size () + 1);
    messages.back ().sendTime = sendTime;
    messages.back ().mess.data = payload;
    messages.back ().mess.src = src;
    messages.back ().mess.dest = dest;
    messages.back ().mess.time = actionTime;
}

void player::loadFile (const std::string &filename)
{
    auto ext = filesystem::path (filename).extension ().string ();
    if ((ext == ".json") || (ext == ".JSON"))
    {
        loadJsonFile (filename);
    }
    else
    {
        loadTextFile (filename);
    }
}

void player::loadTextFile (const std::string &filename)
{
    using namespace stringOps;
    std::ifstream infile (filename);
    std::string str;

    int mcnt = 0;
    int pcnt = 0;
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
        if ((str[fc] == 'm') || (str[fc] == 'M'))
        {
            ++mcnt;
        }
        else
        {
            ++pcnt;
        }
    }
    int pIndex = static_cast<int> (points.size ());
    points.resize (points.size () + pcnt);
    int mIndex = static_cast<int> (messages.size ());
    messages.resize (messages.size () + mcnt);
    // now start over and actual do the loading
    infile.close ();
    infile.open (filename);

    int lcount = 0;
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
        /* time key type value units*/
        auto blk = splitlineBracket (str, ",\t ", default_bracket_chars, delimiter_compression::on);

        trimString (blk[0]);
        if ((blk[0].front () == 'm') || (blk[0].front () == 'M'))
        {
            // deal with messages
            switch (blk.size ())
            {
            case 5:
                try
                {
                    messages[mIndex].sendTime = helics::Time (std::stod (blk[1]));
                }
                catch (const std::invalid_argument &ia)
                {
                    std::cerr << "ill formed time on line " << lcount << '\n';
                    continue;
                }
                messages[mIndex].mess.src = blk[2];
                messages[mIndex].mess.dest = blk[3];
                messages[mIndex].mess.time = messages[mIndex].sendTime;
                messages[mIndex].mess.data = decode (std::move (blk[4]));
                break;
            case 6:
                try
                {
                    messages[mIndex].sendTime = helics::Time (std::stod (blk[1]));
                }
                catch (const std::invalid_argument &ia)
                {
                    std::cerr << "ill formed time on line " << lcount << '\n';
                    continue;
                }
                messages[mIndex].mess.src = blk[3];
                messages[mIndex].mess.dest = blk[4];
                try
                {
                    messages[mIndex].mess.time = helics::Time (std::stod (blk[2]));
                }
                catch (const std::invalid_argument &ia)
                {
                    std::cerr << "ill formed message time on line " << lcount << '\n';
                    continue;
                }
                messages[mIndex].mess.data = decode (std::move (blk[5]));
                break;
            default:
                std::cerr << "unknown message format line " << lcount << '\n';
                break;
            }
            ++mIndex;
        }
        else
        {
            if (blk.size () == 3)
            {
                try
                {
                    points[pIndex].time = helics::Time (std::stod (blk[0]));
                }
                catch (const std::invalid_argument &ia)
                {
                    std::cerr << "ill formed time on line " << lcount << '\n';
                    continue;
                }
                points[pIndex].pubName = blk[1];
                points[pIndex].value = blk[2];
                ++pIndex;
            }
            else if (blk.size () == 4)
            {
                try
                {
                    points[pIndex].time = helics::Time (std::stod (trim (blk[0])));
                }
                catch (const std::invalid_argument &ia)
                {
                    std::cerr << "ill formed time on line " << lcount << '\n';
                    continue;
                }
                points[pIndex].pubName = blk[1];
                points[pIndex].type = blk[2];
                points[pIndex].value = blk[3];
                ++pIndex;
            }
            else
            {
                std::cerr << "unknown publish format line " << lcount << '\n';
            }
        }
    }
}

void player::loadJsonFile (const std::string &jsonFile)
{
    fed->registerInterfaces (jsonFile);

    auto pubCount = fed->getSubscriptionCount ();
    for (int ii = 0; ii < pubCount; ++ii)
    {
        publications.emplace_back (fed.get (), ii);
        pubids[publications.back ().getName ()] = static_cast<int> (publications.size () - 1);
    }
    auto eptCount = fed->getEndpointCount ();
    for (int ii = 0; ii < eptCount; ++ii)
    {
        endpoints.emplace_back (fed.get (), ii);
        eptids[endpoints.back ().getName ()] = static_cast<int> (endpoints.size () - 1);
    }

    using json = nlohmann::json;
    json JF;
    try
    {
        if (jsonFile.size () > 200)
        {
            JF.parse (jsonFile);
        }
        else
        {
            std::ifstream file (jsonFile);
            if (!file.is_open ())
            {
                JF.parse (jsonFile);
            }
            else
            {
                file >> JF;
            }
        }
    }
    catch (const json::exception &je)
    {
        std::cerr << je.what () << '\n';
        return;
    }

    auto pointArray = JF["points"];
    if (pointArray.is_array ())
    {
        points.reserve (points.size () + pointArray.size ());
        for (const auto &pointElement : pointArray)
        {
            Time ptime;
            if (pointElement.count ("time") > 0)
            {
                ptime = pointElement["time"].get<double> ();
            }
            else if (pointElement.count ("t") > 0)
            {
                ptime = pointElement["t"].get<double> ();
            }
            else
            {
                std::cout << "time not specified\n";
                continue;
            }
            defV val;
            if (pointElement.count ("value") > 0)
            {
                auto M = pointElement["value"];
                if (M.is_number_integer ())
                {
                    val = M.get<int64_t> ();
                }
                else if (M.is_number_float ())
                {
                    val = M.get<double> ();
                }
                else
                {
                    val = M.get<std::string> ();
                }
            }
            else if (pointElement.count ("v") > 0)
            {
                auto M = pointElement["v"];
                if (M.is_number_integer ())
                {
                    val = M.get<int64_t> ();
                }
                else if (M.is_number_float ())
                {
                    val = M.get<double> ();
                }
                else
                {
                    val = M.get<std::string> ();
                }
            }
            std::string type;
            if (pointElement.count ("type") > 0)
            {
                type = pointElement["type"].get<std::string> ();
            }
            std::string key;
            if (pointElement.count ("key") > 0)
            {
                key = pointElement["key"].get<std::string> ();
            }
            else
            {
                std::cout << "key not specified\n";
                continue;
            }
            points.resize (points.size () + 1);
            points.back ().time = ptime;
            points.back ().pubName = key;
            points.back ().value = val;
            if (!type.empty ())
            {
                points.back ().type = type;
            }
        }
    }

    auto messageArray = JF["messages"];
    if (messageArray.is_array())
    {
       messages.reserve(messages.size() + messageArray.size());
        for (const auto &messageElement : messageArray)
        {
            Time ptime;
            if (messageElement.count("time") > 0)
            {
                ptime = messageElement["time"].get<double>();
            }
            else if (messageElement.count("t") > 0)
            {
                ptime = messageElement["t"].get<double>();
            }
            else
            {
                std::cout << "time not specified\n";
                continue;
            }
            std::string src;
            if (messageElement.count("src") > 0)
            {
                src = messageElement["src"].get<std::string>();
            }
            if (messageElement.count("source") > 0)
            {
                src = messageElement["source"].get<std::string>();
            }
            std::string dest;
            if (messageElement.count("dest") > 0)
            {
                dest = messageElement["dest"].get<std::string>();
            }
            if (messageElement.count("destination") > 0)
            {
                dest = messageElement["destination"].get<std::string>();
            }
            Time sendTime = ptime;
            std::string type;
            if (messageElement.count("sendtime") > 0)
            {
                ptime = messageElement["sendtime"].get<double>();
            }
            
            messages.resize(messages.size() + 1);
            messages.back().sendTime = sendTime;
            messages.back().mess.src = src;
            messages.back().mess.dest = dest;
            messages.back().mess.time = ptime;
            if (messageElement.count("data") > 0)
            {
                if (messageElement.count("encoding") > 0)
                {
                    if (messageElement["encoding"] == "base64")
                    {
                        messages.back().mess.data = decode(messageElement["data"].get<std::string>());
                    }
                    else
                    {
                        messages.back().mess.data = messageElement["data"].get<std::string>();
                    }
                }
                else
                {
                    messages.back().mess.data = messageElement["data"].get<std::string>();
                }
            }
            else if (messageElement.count("message") > 0)
            {
                if (messageElement.count("encoding") > 0)
                {
                    if (messageElement["encoding"] == "base64")
                    {
                        messages.back().mess.data = decode(messageElement["message"].get<std::string>());
                    }
                    else
                    {
                        messages.back().mess.data = messageElement["message"].get<std::string>();
                    }
                }
                else
                {
                    messages.back().mess.data = messageElement["message"].get<std::string>();
                }
            }
           
        }
    }
}

std::string player::decode (std::string &&stringToDecode)
{
    if ((stringToDecode.compare (1, 3, "64[") == 0) && (stringToDecode.back () == '['))
    {
        stringToDecode.pop_back ();
        return utilities::base64_decode_to_string (stringToDecode, 4);
    }
    if ((stringToDecode.compare (4, 3, "64[") == 0) && (stringToDecode.back () == '['))
    {  // to allow for base64[ or BASE64[
        stringToDecode.pop_back ();
        return utilities::base64_decode_to_string (stringToDecode, 7);
    }

    if (!stringToDecode.empty ())
    {
        if ((stringToDecode.front () == '"') || (stringToDecode.front () == '\''))
        {
            return stringOps::removeQuotes (stringToDecode);
        }
    }
    return std::move (stringToDecode);
}

void player::sortTags ()
{
    std::sort (points.begin (), points.end (), vComp);
    std::sort (messages.begin (), messages.end (), mComp);
    // collapse tags to the reduced list
    for (auto &vs : points)
    {
        auto fnd = tags.find (vs.pubName);
        if (fnd != tags.end ())
        {
            if (fnd->second.empty ())
            {
                tags[vs.pubName] = vs.type;
            }
        }
        else
        {
            tags.emplace (vs.pubName, vs.type);
        }
    }

    for (auto &ms : messages)
    {
        epts.emplace (ms.mess.src);
    }
}

/** helper function to generate the publications*/
void player::generatePublications ()
{
    for (auto &tname : tags)
    {
        // skip already existing publications
        if (pubids.find (tname.first) != pubids.end ())
        {
            continue;
        }
        publications.push_back (
          helics::Publication (helics::GLOBAL, fed.get (), tname.first, helics::getTypeFromString (tname.second)));
        pubids[tname.first] = static_cast<int> (publications.size ()) - 1;
    }
}

/** helper function to generate the publications*/
void player::generateEndpoints ()
{
    for (auto &ename : epts)
    {
        // skip already existing publications
        if (eptids.find (ename) != eptids.end ())
        {
            continue;
        }
        endpoints.push_back (Endpoint (GLOBAL, fed.get (), ename));
        eptids[ename] = static_cast<int> (endpoints.size ()) - 1;
    }
}

void player::cleanUpPointList ()
{
    // load up the indexes
    for (auto &vs : points)
    {
        vs.index = pubids[vs.pubName];
    }
    /** load the indices for the message*/
    for (auto &ms : messages)
    {
        ms.index = eptids[ms.mess.src];
    }
}

void player::initialize ()
{
    auto state = fed->currentState ();
    if (state == Federate::op_states::startup)
    {
        sortTags ();
        generatePublications ();
        generateEndpoints ();
        cleanUpPointList ();
        fed->enterInitializationState ();
    }
}

/*run the player*/
void player::run ()
{
    run (stopTime);
    fed->finalize ();
}

void player::run (Time stopTime_input)
{
    auto state = fed->currentState ();
    if (state == Federate::op_states::startup)
    {
        initialize ();
    }
    if (state != Federate::op_states::execution)
    {
        // send stuff before timeZero
        if (!points.empty ())
        {
            while (points[pointIndex].time < helics::timeZero)
            {
                publications[points[pointIndex].index].publish (points[pointIndex].value);
                ++pointIndex;
                if (pointIndex >= points.size ())
                {
                    break;
                }
            }
        }
        if (!messages.empty ())
        {
            while (messages[messageIndex].sendTime < helics::timeZero)
            {
                endpoints[messages[messageIndex].index].send (messages[messageIndex].mess);
                ++messageIndex;
                if (messageIndex >= messages.size ())
                {
                    break;
                }
            }
        }

        fed->enterExecutionState ();
        // send the stuff at timeZero
        if (!points.empty ())
        {
            while (points[pointIndex].time == helics::timeZero)
            {
                publications[points[pointIndex].index].publish (points[pointIndex].value);
                ++pointIndex;
                if (pointIndex >= points.size ())
                {
                    break;
                }
            }
        }
        if (!messages.empty ())
        {
            while (messages[messageIndex].sendTime == helics::timeZero)
            {
                endpoints[messages[messageIndex].index].send (messages[messageIndex].mess);
                ++messageIndex;
                if (messageIndex >= messages.size ())
                {
                    break;
                }
            }
        }
    }
    else
    {
        auto ctime = fed->getCurrentTime ();
        if (pointIndex < points.size ())
        {
            while (points[pointIndex].time <= ctime)
            {
                ++pointIndex;
                if (pointIndex >= points.size ())
                {
                    break;
                }
            }
        }
        if (messageIndex < messages.size ())
        {
            while (messages[messageIndex].sendTime <= ctime)
            {
                ++messageIndex;
                if (messageIndex >= messages.size ())
                {
                    break;
                }
            }
        }
    }

    helics::Time nextPrintTime = 10.0;
    bool moreToSend = true;
    Time nextSendTime = timeZero;
    while (moreToSend)
    {
        nextSendTime = Time::maxVal ();
        if (pointIndex < points.size ())
        {
            nextSendTime = std::min (nextSendTime, points[pointIndex].time);
        }
        if (messageIndex < messages.size ())
        {
            nextSendTime = std::min (nextSendTime, messages[messageIndex].sendTime);
        }
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
        if (pointIndex < points.size ())
        {
            while (points[pointIndex].time <= newTime)
            {
                publications[points[pointIndex].index].publish (points[pointIndex].value);
                ++pointIndex;
                if (pointIndex >= points.size ())
                {
                    break;
                }
            }
        }
        if (messageIndex < messages.size ())
        {
            while (messages[messageIndex].sendTime <= newTime)
            {
                endpoints[messages[messageIndex].index].send (messages[messageIndex].mess);
                ++messageIndex;
                if (messageIndex >= messages.size ())
                {
                    break;
                }
            }
        }
        if (newTime >= nextPrintTime)
        {
            std::cout << "processed time " << static_cast<double> (newTime) << "\n";
            nextPrintTime += 10.0;
        }
    }
}

void player::addPublication (const std::string &key, helicsType_t type, const std::string &units)
{
    // skip already existing publications
    if (pubids.find (key) != pubids.end ())
    {
        std::cerr << "publication already exists\n";
    }
    publications.push_back (Publication (GLOBAL, fed.get (), key, type, units));
    pubids[key] = static_cast<int> (publications.size ()) - 1;
}

void player::addEndpoint (const std::string &endpointName, const std::string &endpointType)
{
    // skip already existing publications
    if (eptids.find (endpointName) != eptids.end ())
    {
        std::cerr << "Endpoint already exists\n";
    }
    endpoints.push_back (Endpoint (GLOBAL, fed.get (), endpointName, endpointType));
    eptids[endpointName] = static_cast<int> (endpoints.size ()) - 1;
}

int player::loadArguments (boost::program_options::variables_map &vm_map)
{
    if (vm_map.count ("input") == 0)
    {
        return (-1);
    }

    if (vm_map.count ("datatype") > 0)
    {
        defType = helics::getTypeFromString (vm_map["datatype"].as<std::string> ());
        if (defType == helics::helicsType_t::helicsInvalid)
        {
            std::cerr << vm_map["datatype"].as<std::string> () << " is not recognized as a valid type \n";
            return -3;
        }
    }

    if (!filesystem::exists (vm_map["input"].as<std::string> ()))
    {
        std::cerr << vm_map["input"].as<std::string> () << " does not exist \n";
        return -3;
    }
    loadFile (vm_map["input"].as<std::string> ());

    std::cout << "read file " << points.size () << " points for " << tags.size () << " tags \n";

    stopTime = Time::maxVal ();
    if (vm_map.count ("stop") > 0)
    {
        stopTime = vm_map["stop"].as<double> ();
    }
    return 0;
}

}  // namespace helics

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
