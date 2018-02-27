/*
Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.
*/
#include "Player.hpp"
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
namespace apps
{
static inline bool vComp (const ValueSetter &v1, const ValueSetter &v2) { return (v1.time < v2.time); }
static inline bool mComp (const MessageHolder &m1, const MessageHolder &m2) { return (m1.sendTime < m2.sendTime); }

static const ArgDescriptors InfoArgs{
    {"datatype",  "type of the publication data type to use"},
    {"local", ArgDescriptor::arg_type_t::flag_type, "specify otherwise unspecified endpoints and publications as local( i.e.the keys will be prepended with the player name"},
    {"separator", "specify the separator for local publications and endpoints"},
    {"timeunits", "the default units on the timestamps used in file based input"},
    {"stop",  "the time to stop the player"}
};

Player::Player (int argc, char *argv[])
{
    variable_map vm_map;
    auto res = argumentParser(argc, argv, vm_map, InfoArgs,"input");
    if (res == versionReturn)
    {
        std::cout << helics::versionString<< '\n';
    }
    if (res == helpReturn)
    {
        FederateInfo helpTemp(argc, argv);
    }
    if (res < 0)
    {
        deactivated = true;
        return;
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

Player::Player (const std::shared_ptr<Core> &core, const FederateInfo &fi)
    : fed (std::make_shared<CombinationFederate> (core, fi))
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
        if (units == timeUnits::ns) //ns
        {
            return helics::Time(std::stoll(str), timeUnits::ns);
        }
        else
        {
            return loadTimeFromString(str,units);
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

    auto doc = loadJsonString(jsonFile);
    

    if (doc.isMember("player"))
    {
        auto playerConfig = doc["player"];
        if (playerConfig.isMember("stop"))
        {
            stopTime = loadJsonTime(playerConfig["stop"]);
        }
        if (playerConfig.isMember("local"))
        {
            useLocal = playerConfig["local"].asBool();
        }
        if (playerConfig.isMember("separator"))
        {
            auto sep = playerConfig["separator"].asString();
            if (sep.size() > 0)
            {
                fed->setSeparator(sep[0]);
            }
           
        }
        if (playerConfig.isMember("timeunits"))
        {
            if (playerConfig["timeunits"].asString() == "ns")
            {
                timeMultiplier = 1e-9;
            }
        }
        if (playerConfig.isMember("file"))
        {
            if (playerConfig["file"].isArray())
            {
                for (decltype(playerConfig.size()) ii=0;ii<playerConfig.size();++ii)
                {
                    loadFile(playerConfig["file"][ii].asString());
                }
            }
            else
            {
                loadFile(playerConfig["file"].asString());
            }
        }
    }
    auto pointArray = doc["points"];
    if (pointArray.isArray ())
    {
        points.reserve (points.size () + pointArray.size ());
        for (const auto &pointElement : pointArray)
        {
            Time ptime;
            if (pointElement.isMember ("time"))
            {
                ptime=loadJsonTime(pointElement["time"], units);
            }
            else if (pointElement.isMember("t"))
            {
                ptime = loadJsonTime(pointElement["t"],units);
            }
            else
            {
                std::cout << "time not specified\n";
                continue;
            }
            defV val;
            if (pointElement.isMember ("value"))
            {
                auto M = pointElement["value"];
                if (M.isInt64 ())
                {
                    val = M.asInt64 ();
                }
                else if (M.isDouble())
                {
                    val = M.asDouble();
                }
                else
                {
                    val = M.asString();
                }
            }
            else if (pointElement.isMember ("v"))
            {
                auto M = pointElement["v"];
                if (M.isInt64())
                {
                    val = M.asInt64();
                }
                else if (M.isDouble())
                {
                    val = M.asDouble();
                }
                else
                {
                    val = M.asString();
                }
            }
            std::string type;
            if (pointElement.isMember("type"))
            {
                type = pointElement["type"].asString();
            }
            std::string key;
            if (pointElement.isMember("key"))
            {
                key = pointElement["key"].asString();
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

    auto messageArray = doc["messages"];
    if (messageArray.isArray())
    {
       messages.reserve(messages.size() + messageArray.size());
        for (const auto &messageElement : messageArray)
        {
            Time ptime;
            if (messageElement.isMember("time"))
            {
                ptime = loadJsonTime(messageElement["time"],units);
            }
            else if (messageElement.isMember("t"))
            {
                ptime = loadJsonTime(messageElement["t"], units);
            }
            else
            {
                std::cout << "time not specified\n";
                continue;
            }
            std::string src;
            if (messageElement.isMember("src"))
            {
                src = messageElement["src"].asString();
            }
            if (messageElement.isMember("source"))
            {
                src = messageElement["source"].asString();
            }
            std::string dest;
            if (messageElement.isMember("dest"))
            {
                dest = messageElement["dest"].asString();
            }
            if (messageElement.isMember("destination"))
            {
                dest = messageElement["destination"].asString();
            }
            Time sendTime = ptime;
            std::string type;
            if (messageElement.isMember("sendtime"))
            {
                ptime = loadJsonTime(messageElement["sendtime"],units);
            }
            
            messages.resize(messages.size() + 1);
            messages.back().sendTime = sendTime;
            messages.back().mess.source = src;
            messages.back().mess.dest = dest;
            messages.back().mess.time = ptime;
            if (messageElement.isMember("data"))
            {
                if (messageElement.isMember("encoding"))
                {
                    if (messageElement["encoding"].asString() == "base64")
                    {
                        messages.back().mess.data = decode(messageElement["data"].asString());
                    }
                    else
                    {
                        messages.back().mess.data = messageElement["data"].asString();
                    }
                }
                else
                {
                    messages.back().mess.data = messageElement["data"].asString();
                }
            }
            else if (messageElement.isMember("message"))
            {
                if (messageElement.isMember("encoding"))
                {
                    if (messageElement["encoding"].asString() == "base64")
                    {
                        messages.back().mess.data = decode(messageElement["message"].asString());
                    }
                    else
                    {
                        messages.back().mess.data = messageElement["message"].asString();
                    }
                }
                else
                {
                    messages.back().mess.data = messageElement["message"].asString();
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

void Player::addPublication(const std::string &key, helics_type_t type, const std::string &pubUnits)
{
    // skip already existing publications
    if (pubids.find(key) != pubids.end())
    {
        std::cerr << "publication already exists\n";
    }
    if (!useLocal)
    {
        publications.push_back(Publication(GLOBAL, fed.get(), key, type, pubUnits));
    }
    else
    {
        auto kp = key.find_first_of("./");
        if (kp == std::string::npos)
        {
            publications.push_back(Publication(fed.get(), key, type, pubUnits));
        }
        else
        {
            publications.push_back(Publication(GLOBAL, fed.get(), key, type, pubUnits));
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
        fed->setSeparator(vm_map["separator"].as<std::string>()[0]);
    }
    if (vm_map.count("timeunits"))
    {
        try
        {
            units = timeUnitsFromString(vm_map["timeunits"].as<std::string>());
            timeMultiplier = toSecondMultiplier(units);
        }
        catch (...)
        {
            std::cerr << vm_map["timeunits"].as<std::string>() << " is not recognized as a valid unit of time \n";
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
   
    if (vm_map.count ("stop") > 0)
    {
        stopTime = loadTimeFromString(vm_map["stop"].as<std::string> ());
    }
    return 0;
}

}  // namespace apps
} // namespace helics

