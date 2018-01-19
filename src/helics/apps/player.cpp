/*

Copyright (C) 2017-2018, Battelle Memorial Institute
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
#include "../core/helicsVersion.hpp"

namespace po = boost::program_options;
namespace filesystem = boost::filesystem;

static int playerArgumentParser (int argc, const char *const *argv, po::variables_map &vm_map);

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

Player::Player (int argc, char *argv[])
{
    boost::program_options::variables_map vm_map;
    auto res = playerArgumentParser(argc, argv, vm_map);
    if (res != 0)
    {
        deactivated = true;
    }
    FederateInfo fi ("player");
    fi.loadInfoFromArgs (argc, argv);
    fed = std::make_shared<CombinationFederate> (fi);
    fed->setFlag (SOURCE_ONLY_FLAG);
    loadArguments(vm_map);
   
}

Player::Player (const FederateInfo &fi) : fed (std::make_shared<CombinationFederate> (fi))
{
    fed->setFlag (SOURCE_ONLY_FLAG);
}

Player::Player (std::shared_ptr<Core> core, const FederateInfo &fi)
    : fed (std::make_shared<CombinationFederate> (std::move (core), fi))
{
    fed->setFlag (SOURCE_ONLY_FLAG);
}

Player::Player (const std::string &jsonString) : fed (std::make_shared<CombinationFederate> (jsonString))
{
    fed->setFlag (SOURCE_ONLY_FLAG);
    if (jsonString.size () < 200)
    {
        masterFileName = jsonString;
    }
    loadJsonFile (jsonString);
}

Player::~Player () = default;

void Player::addMessage (Time sendTime,
                         const std::string &src,
                         const std::string &dest,
                         const std::string &payload)
{
    messages.resize (messages.size () + 1);
    messages.back ().sendTime = sendTime;
    messages.back ().mess.data = payload;
    messages.back ().mess.source = src;
    messages.back ().mess.dest = dest;
    messages.back ().mess.time = sendTime;
}

void Player::addMessage (Time sendTime,
                         Time actionTime,
                         const std::string &src,
                         const std::string &dest,
                         const std::string &payload)
{
    messages.resize (messages.size () + 1);
    messages.back ().sendTime = sendTime;
    messages.back ().mess.data = payload;
    messages.back ().mess.source = src;
    messages.back ().mess.dest = dest;
    messages.back ().mess.time = actionTime;
}

void Player::loadFile (const std::string &filename)
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


helics::Time Player::extractTime(const std::string &str, int lineNumber) const
{
    try
    {
        if (timeMultiplier == 1e-9) //ns
        {
            return helics::Time(std::stoll(str), timeUnits::ns);
        }
        else
        {
            return helics::Time(std::stod(str));
        }
    }
    catch (const std::invalid_argument &ia)
    {
        std::cerr << "ill formed time on line " << lineNumber << '\n';
        return helics::Time::minVal();
    }
}

void Player::loadTextFile (const std::string &filename)
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
            if (str[fc + 1] == '!')
            {
                /*  //allow configuration inside the regular text file 
                if (playerConfig.find("stop") != playerConfig.end())
                {
                    stopTime = playerConfig["stop"].get<double>();
                }
                if (playerConfig.find("local") != playerConfig.end())
                {
                    useLocal = playerConfig["local"].get<bool>();
                }
                if (playerConfig.find("separator") != playerConfig.end())
                {
                    fed->setSeparator(playerConfig["separator"].get<char>());
                }
                if (playerConfig.find("timeunits") != playerConfig.end())
                {
                    if (playerConfig["timeunits"] == "ns")
                    {
                        timeMultiplier = 1e-9;
                    }
                }
                if (playerConfig.find("file") != playerConfig.end())
                {
                    if (playerConfig["file"].is_array())
                    {
                        for (const auto &fname : playerConfig["file"])
                        {
                            loadFile(fname);
                        }
                    }
                    else
                    {
                        loadFile(playerConfig["file"]);
                    }
                }
                */
            }
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
                if ((messages[mIndex].sendTime = extractTime(blk[1],lcount))==Time::minVal())
                {
                    continue;
                }
               
                messages[mIndex].mess.source = blk[2];
                messages[mIndex].mess.dest = blk[3];
                messages[mIndex].mess.time = messages[mIndex].sendTime;
                messages[mIndex].mess.data = decode (std::move (blk[4]));
                break;
            case 6:
                if ((messages[mIndex].sendTime = extractTime(blk[1], lcount)) == Time::minVal())
                {
                    continue;
                }
               
                messages[mIndex].mess.source = blk[3];
                messages[mIndex].mess.dest = blk[4];
                if ((messages[mIndex].mess.time = extractTime(blk[2], lcount)) == Time::minVal())
                {
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
                if ((points[pIndex].time = extractTime(blk[0], lcount)) == Time::minVal())
                {
                    continue;
                }
                
                points[pIndex].pubName = blk[1];
                points[pIndex].value = blk[2];
                ++pIndex;
            }
            else if (blk.size () == 4)
            {
                if ((points[pIndex].time = extractTime(trim(blk[0]), lcount)) == Time::minVal())
                {
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

void Player::loadJsonFile (const std::string &jsonFile)
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

    if (JF.find("player")!=JF.end())
    {
        auto playerConfig = JF["player"];
        if (playerConfig.find("stop") != playerConfig.end())
        {
            stopTime = playerConfig["stop"].get<double>();
        }
        if (playerConfig.find("local") != playerConfig.end())
        {
            useLocal = playerConfig["local"].get<bool>();
        }
        if (playerConfig.find("separator") != playerConfig.end())
        {
            fed->setSeparator(playerConfig["separator"].get<char>());
        }
        if (playerConfig.find("timeunits") != playerConfig.end())
        {
            if (playerConfig["timeunits"] == "ns")
            {
                timeMultiplier = 1e-9;
            }
        }
        if (playerConfig.find("file") != playerConfig.end())
        {
            if (playerConfig["file"].is_array())
            {
                for (const auto &fname : playerConfig["file"])
                {
                    loadFile(fname);
                }
            }
            else
            {
                loadFile(playerConfig["file"]);
            }
        }
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
                if (timeMultiplier == 1e-9)
                {
                    ptime = helics::Time(pointElement["time"].get<int64_t>(), timeUnits::ns);
                }
                else
                {
                    ptime = pointElement["time"].get<double>();
                }
                
            }
            else if (pointElement.count ("t") > 0)
            {
                if (timeMultiplier == 1e-9)
                {
                    ptime = helics::Time(pointElement["t"].get<int64_t>(), timeUnits::ns);
                }
                else
                {
                    ptime = pointElement["t"].get<double>();
                }
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
                if (timeMultiplier == 1e-9)
                {
                    ptime = helics::Time(messageElement["time"].get<int64_t>(), timeUnits::ns);
                }
                else
                {
                    ptime = messageElement["time"].get<double>();
                }
            }
            else if (messageElement.count("t") > 0)
            {
                if (timeMultiplier == 1e-9)
                {
                    ptime = helics::Time(messageElement["t"].get<int64_t>(), timeUnits::ns);
                }
                else
                {
                    ptime = messageElement["t"].get<double>();
                }
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
                if (timeMultiplier == 1e-9)
                {
                    ptime = helics::Time(messageElement["sendtime"].get<int64_t>(), timeUnits::ns);
                }
                else
                {
                    ptime = messageElement["sendtime"].get<double>();
                }
            }
            
            messages.resize(messages.size() + 1);
            messages.back().sendTime = sendTime;
            messages.back().mess.source = src;
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

std::string Player::decode (std::string &&stringToDecode)
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

void Player::sortTags ()
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
        epts.emplace (ms.mess.source);
    }
}

/** helper function to generate the publications*/
void Player::generatePublications ()
{
    for (auto &tname : tags)
    {
        if (pubids.find(tname.first) == pubids.end())
        {
            addPublication(tname.first, helics::getTypeFromString(tname.second));
        }
    }
}

/** helper function to generate the publications*/
void Player::generateEndpoints ()
{
    for (auto &ename : epts)
    {
        if (eptids.find(ename) == eptids.end())
        {
            addEndpoint(ename);
        }
    }
}

void Player::cleanUpPointList ()
{
    // load up the indexes
    for (auto &vs : points)
    {
        vs.index = pubids[vs.pubName];
    }
    /** load the indices for the message*/
    for (auto &ms : messages)
    {
        ms.index = eptids[ms.mess.source];
    }
}

void Player::initialize ()
{
    auto state = fed->getCurrentState ();
    if (state == Federate::op_states::startup)
    {
        sortTags ();
        generatePublications ();
        generateEndpoints ();
        cleanUpPointList ();
        fed->enterInitializationState ();
    }
}


void Player::finalize()
{
    fed->finalize();
}

void Player::sendInformation(Time sendTime)
{
    if (!points.empty())
    {
        while (points[pointIndex].time <= sendTime)
        {
            publications[points[pointIndex].index].publish(points[pointIndex].value);
            ++pointIndex;
            if (pointIndex >= points.size())
            {
                break;
            }
        }
    }
    if (!messages.empty())
    {
        while (messages[messageIndex].sendTime <=sendTime)
        {
            endpoints[messages[messageIndex].index].send(messages[messageIndex].mess);
            ++messageIndex;
            if (messageIndex >= messages.size())
            {
                break;
            }
        }
    }
}

/*run the Player*/
void Player::run ()
{
    run (stopTime);
    fed->disconnect();
}

void Player::run (Time stopTime_input)
{
    auto state = fed->getCurrentState ();
    if (state == Federate::op_states::startup)
    {
        initialize ();
    }
    if (state < Federate::op_states::execution)
    {
        sendInformation(-Time::epsilon());

        fed->enterExecutionState ();
        // send the stuff at timeZero
        sendInformation(timeZero);
        
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
        sendInformation(newTime);
        
        if (newTime >= nextPrintTime)
        {
            std::cout << "processed time " << static_cast<double> (newTime) << "\n";
            nextPrintTime += 10.0;
        }
    }
}

void Player::addPublication(const std::string &key, helics_type_t type, const std::string &units)
{
    // skip already existing publications
    if (pubids.find(key) != pubids.end())
    {
        std::cerr << "publication already exists\n";
    }
    if (!useLocal)
    {
        publications.push_back(Publication(GLOBAL, fed.get(), key, type, units));
    }
    else
    {
        auto kp = key.find_first_of("./");
        if (kp == std::string::npos)
        {
            publications.push_back(Publication(fed.get(), key, type, units));
        }
        else
        {
            publications.push_back(Publication(GLOBAL, fed.get(), key, type, units));
        }
    }
    pubids[key] = static_cast<int> (publications.size ()) - 1;
}

void Player::addEndpoint(const std::string &endpointName, const std::string &endpointType)
{
    // skip already existing publications
    if (eptids.find(endpointName) != eptids.end())
    {
        std::cerr << "Endpoint already exists\n";
    }
    if (!useLocal)
    {
        endpoints.push_back(Endpoint(GLOBAL, fed.get(), endpointName, endpointType));
    }
    else
    {
        auto kp = endpointName.find_first_of("./");
        if (kp == std::string::npos)
        {
            endpoints.push_back(Endpoint(fed.get(), endpointName, endpointType));
        }
        else
        {
            endpoints.push_back(Endpoint(GLOBAL, fed.get(), endpointName, endpointType));
        }
    }
    eptids[endpointName] = static_cast<int> (endpoints.size ()) - 1;
}

int Player::loadArguments(boost::program_options::variables_map &vm_map)
{

    if (vm_map.count("datatype") > 0)
    {
        defType = helics::getTypeFromString(vm_map["datatype"].as<std::string>());
        if (defType == helics::helics_type_t::helicsInvalid)
        {
            std::cerr << vm_map["datatype"].as<std::string>() << " is not recognized as a valid type \n";
            return -3;
        }
    }
    if (vm_map.count("local"))
    {
        useLocal = true;
    }
    if (vm_map.count("separator"))
    {
        fed->setSeparator(vm_map["separator"].as<char>());
    }
    if (vm_map.count("timeunits"))
    {
        if (vm_map["timeunits"].as<std::string>() == "ns")
        {
            timeMultiplier = 1e-9;
        }
    }
    std::string file;
    if (vm_map.count("input") == 0)
    {
        if (!fileLoaded)
        {
            if (filesystem::exists("helics.json"))
            {
                file = "helics.json";
            }
        }
    }
    if (filesystem::exists (vm_map["input"].as<std::string> ()))
    {
        file = vm_map["input"].as<std::string>();
    }
    if (!file.empty())
    {
        loadFile(file);
    }
   

    stopTime = Time::maxVal ();
    if (vm_map.count ("stop") > 0)
    {
        stopTime = vm_map["stop"].as<double> ();
    }
    return 0;
}

}  // namespace helics

int playerArgumentParser (int argc, const char *const *argv, po::variables_map &vm_map)
{
    po::options_description cmd_only ("command line only");
    po::options_description config ("configuration");
    po::options_description hidden ("hidden");

    // clang-format off
    // input boost controls
    cmd_only.add_options () 
		("help,?", "produce help message")
		("version,v","HELICS version number")
		("config-file", po::value<std::string> (),"specify a configuration file to use");


    config.add_options ()
		("datatype",po::value<std::string>(),"type of the publication data type to use")
        ("local","specify otherwise unspecified endpoints and publications as local( i.e.the keys will be prepended with the player name")
        ("separator",po::value<char>(),"specify the separator for local publications and endpoints")
        ("timeunits",po::value<std::string>(),"the units on the timestamp used in file based input")
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
        po::store (po::command_line_parser (argc, argv).options (cmd_line).allow_unregistered().positional (p).run (), cmd_vm);
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
        std::cout << visible <<'\n';
        return (-1);
    }

    if (cmd_vm.count ("version") > 0)
    {
        std::cout << helics::helicsVersionString () << '\n';
        return (-1);
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
    return 0;

}
