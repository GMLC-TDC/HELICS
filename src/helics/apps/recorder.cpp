/*

Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "../application_api/Subscriptions.hpp"
#include "../application_api/ValueFederate.hpp"
#include "../application_api/queryFunctions.hpp"
#include "../application_api/Filters.hpp"
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
#include "../common/stringOps.h"

#include "PrecHelper.h"
#include <thread>
#include "recorder.h"
#include "json.hpp"

namespace po = boost::program_options;
namespace filesystem = boost::filesystem;

static int recorderArgumentParser (int argc, const char *const *argv, po::variables_map &vm_map);

namespace helics
{
    Recorder::Recorder(FederateInfo &fi) : fed(std::make_shared<CombinationFederate>(fi))
    {
        fed->setFlag(OBSERVER_FLAG);
    }

    Recorder::Recorder(int argc, char *argv[])
    {
        boost::program_options::variables_map vm_map;
        int res = recorderArgumentParser(argc, argv, vm_map);
        if (res != 0)
        {
            deactivated = true;
            return;
        }
        FederateInfo fi("recorder");
        
        fi.loadInfoFromArgs( argc, argv);
        fed = std::make_shared<CombinationFederate>(fi);
        fed->setFlag(OBSERVER_FLAG);
        
        loadArguments(vm_map);
    }


    Recorder::Recorder(std::shared_ptr<Core> core, const FederateInfo &fi) :fed(std::make_shared<CombinationFederate>(std::move(core), fi))
    {
        fed->setFlag(OBSERVER_FLAG);
    }

    Recorder::Recorder(const std::string &jsonString) :fed(std::make_shared<CombinationFederate>(jsonString))
    {
        fed->setFlag(OBSERVER_FLAG);
        loadJsonFile(jsonString);
    }

  
    Recorder::~Recorder()
    {
        saveFile(outFileName);
    }

    int Recorder::loadFile(const std::string &filename)
    {
        auto ext = filesystem::path(filename).extension().string();
        if ((ext == ".json") || (ext == ".JSON"))
        {
            return loadJsonFile(filename);
        }
        else
        {
            return loadTextFile(filename);
        }
    }


    int Recorder::loadJsonFile(const std::string &jsonString)
    {
        fed->registerInterfaces(jsonString);

        auto pubCount = fed->getSubscriptionCount();
        for (int ii = 0; ii < pubCount; ++ii)
        {
            subscriptions.emplace_back(fed.get(), ii);
            subids.emplace(subscriptions.back().getID(), static_cast<int>(subscriptions.size())-1);
            subkeys.emplace(subscriptions.back().getName(), static_cast<int>(subscriptions.size()) - 1);
        }
        auto eptCount = fed->getEndpointCount();
        for (int ii = 0; ii < eptCount; ++ii)
        {
            endpoints.emplace_back(fed.get(), ii);
            eptNames[endpoints.back().getName()] = static_cast<int> (endpoints.size() - 1);
            eptids.emplace(endpoints.back().getID(), static_cast<int> (endpoints.size() - 1));
        }

        using json = nlohmann::json;
        json JF;
        try
        {
            if (jsonString.size() > 200)
            {
                JF.parse(jsonString);
            }
            else
            {
                std::ifstream file(jsonString);
                if (!file.is_open())
                {
                    JF.parse(jsonString);
                }
                else
                {
                    file >> JF;
                }
            }
            auto tags = JF["tag"];
            if (tags.is_array())
            {
                for (const std::string &tag : tags)
                {
                    addSubscription(tag);
                }
            }
            else if (tags.is_string())
            {
                addSubscription(tags);
            }
            auto sourceClone = JF["sourceclone"];
            if (sourceClone.is_array())
            {
                for (const std::string &sc : sourceClone)
                {
                    addSourceEndpointClone(sc);
                }
            }
            else if (sourceClone.is_string())
            {
                addSourceEndpointClone(sourceClone);
            }
            auto destClone = JF["destclone"];
            if (destClone.is_array())
            {
                for (const std::string &dc : destClone)
                {
                    addDestEndpointClone(dc);
                }
            }
            else if(destClone.is_string())
            {
                addDestEndpointClone(destClone);
            }
            auto clones = JF["clone"];
            if (clones.is_array())
            {
                for (const std::string &clone : clones)
                {
                    addSourceEndpointClone(clone);
                    addDestEndpointClone(clone);
                }
            }
            else if (clones.is_string())
            {
                addSourceEndpointClone(clones);
                addDestEndpointClone(clones);
            }
            auto captures = JF["capture"];
            if (captures.is_array())
            {
                for (const std::string &capture : captures)
                {
                    addCapture(capture);
                }
            }
            else if (captures.is_string())
            {
                addCapture(captures);
            }
        }
        catch (const json::exception &je)
        {
            std::cerr << je.what() << '\n';
            return (-1);
        }
        return 0;
    }

   int Recorder::loadTextFile(const std::string &textFile)
    {
       using namespace stringOps;

        std::ifstream infile(textFile);
        std::string str;
        int lc = 0;
        while (std::getline(infile, str))
        {
            ++lc;
            if (str.empty())
            {
                continue;
            }
            auto fc = str.find_first_not_of(" \t\n\r\0");
            if ((fc == std::string::npos) || (str[fc] == '#'))
            {
continue;
            }
            auto blk = splitlineQuotes(str, ",\t ", default_quote_chars, delimiter_compression::on);

            switch (blk.size())
            {
            case 1:
                addSubscription(removeQuotes(blk[0]));
                break;
            case 2:
                if ((blk[0] == "subscription") || (blk[0] == "s") || (blk[0] == "sub") || (blk[0] == "tag"))
                {
                    addSubscription(removeQuotes(blk[1]));
                }
                else if ((blk[0] == "endpoint") || (blk[0] == "ept") || (blk[0] == "e"))
                {
                    addEndpoint(removeQuotes(blk[1]));
                }
                else if ((blk[0] == "sourceclone") || (blk[0] == "source") || (blk[0] == "src"))
                {
                    addSourceEndpointClone(removeQuotes(blk[1]));
                }
                else if ((blk[0] == "destclone") || (blk[0] == "dest") || (blk[0] == "destination"))
                {
                    addDestEndpointClone(removeQuotes(blk[1]));
                }
                else if (blk[0] == "capture")
                {
                    addCapture(removeQuotes(blk[1]));
                }
                else if (blk[0] == "clone")
                {
                    addSourceEndpointClone(removeQuotes(blk[1]));
                    addDestEndpointClone(removeQuotes(blk[1]));
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
                        addSourceEndpointClone(removeQuotes(blk[2]));
                    }
                    else if ((blk[1] == "dest") || (blk[1] == "destination"))
                    {
                        addDestEndpointClone(removeQuotes(blk[2]));
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
        infile.close();
        return 0;
    }

    void Recorder::writeJsonFile(const std::string &filename)
    {
        using json = nlohmann::json;
        json JF;
        if (!points.empty())
        {
            JF["points"] = json::array();
            for (auto &v : points)
            {
                json point;
                point["key"] = subscriptions[v.index].getKey();
                point["value"] = v.value;
                point["time"] = static_cast<double>(v.time);

                if (v.first)
                {
                    point["type"] = subscriptions[v.index].getType();
                }
                JF["points"].push_back(point);
            }
        }

        if (!messages.empty())
        {
            JF["messages"] = json::array();
            for (auto &mess : messages)
            {
                json message;
                message["time"] = static_cast<double>(mess->time);
                message["src"] = mess->source;
                if ((!mess->original_source.empty()) && (mess->original_source != mess->source))
                {
                    message["original_source"] = mess->original_source;
                }
                if ((mess->dest.size()<7) || (mess->dest.compare(mess->dest.size() - 6, 6, "cloneE") != 0))
                {
                    message["dest"] = mess->dest;
                    message["orig_dest"] = mess->original_dest;
                }
                else
                {
                    message["dest"] = mess->original_dest;
                }
                message["message"] = encode(mess->data.to_string());
                JF["messages"].push_back(message);
            }
        }
      
        std::ofstream o(filename);
        o << std::setw(4) << JF << std::endl;
    }

    void Recorder::writeTextFile( const std::string &filename)
    {
        std::ofstream outFile(filename.empty() ? outFileName : filename);
        if (!points.empty())
        {
            outFile << "#time \ttag\t value\t type*\n";
        }
        for (auto &v : points)
        {
            if (v.first)
            {
               outFile << static_cast<double> (v.time) << "\t\t" << subscriptions[v.index].getKey() << '\t' << v.value << '\t'
                    << subscriptions[v.index].getType() << '\n';
            }
            else
            {
                outFile << static_cast<double> (v.time) << "\t\t" << subscriptions[v.index].getKey() << '\t' << v.value << '\n';
            }
        }
        if (!messages.empty())
        {
            outFile << "# m\t time \tsource\t dest\t message\n";
        }
        for (auto &m : messages)
        {
            outFile << "m\t" << static_cast<double> (m->time) << '\t' << m->source << '\t';
            if ((m->dest.size()<7)||(m->dest.compare(m->dest.size() - 6, 6, "cloneE") != 0))
            {
                outFile << m->dest;
               
            }
            else
            {
                outFile << m->original_dest;
            }
            outFile << "\t\"" << encode(m->data.to_string())<<"\"\n";
        }
    }

    void Recorder::initialize()
    {
        generateInterfaces();

        vStat.reserve(subkeys.size());
        for (auto &val : subkeys)
        {
            vStat.emplace_back(ValueStats());
            vStat.back().key = val.first;
        }

        fed->enterInitializationState();
        captureForCurrentTime(-1.0);

        fed->enterExecutionState();
        captureForCurrentTime(0.0);
    }

    void Recorder::finalize()
    {
        fed->finalize();
    }

    void Recorder::generateInterfaces()
    {
        for (auto &tag : subkeys)
        {
            if (tag.second == -1)
            {
                addSubscription(tag.first);
            }
        }
        for (auto &ept : eptNames)
        {
            if (ept.second == -1)
            {
                addEndpoint(ept.first);
            }
        }
        loadCaptureInterfaces();
    }

    void Recorder::loadCaptureInterfaces()
    {
        for (auto &capt : captureInterfaces)
        {
            auto res = waitForInit(fed.get(), capt);
            if (res)
            {
                auto pubs = vectorizeQueryResult(fed->query(capt, "publications"));
                for (auto &pub : pubs)
                {
                    addSubscription(pub);
                }
            }
        }
        
    }

    void Recorder::captureForCurrentTime(Time currentTime)
    {
        for (auto &sub : subscriptions)
        {
            if (sub.isUpdated())
            {
                auto val = sub.getValue<std::string>();
                int ii = subids[sub.getID()];
                points.emplace_back(currentTime, ii, val);
                if (vStat[ii].cnt == 0)
                {
                    points.back().first = true;
                }
                ++vStat[ii].cnt;
                vStat[ii].lastVal = val;
                vStat[ii].time = -1.0;
            }
        }

        for (auto &ept : endpoints)
        {
            while (ept.hasMessage())
            {
                messages.push_back(ept.getMessage());
            }
        }
        //get the clone endpoints
        if (cloneEndpoint)
        {
            while (cloneEndpoint->hasMessage())
            {
                messages.push_back(cloneEndpoint->getMessage());
            }
        }
    }


    std::string Recorder::encode(const std::string &str2encode)
    {
        return str2encode;
    }

    /*run the Player*/
    void Recorder::run()
    {
            run(autoStopTime);
            fed->finalize();
    }
    /** run the Player until the specified time*/
    void Recorder::run(Time runToTime)
    {
        initialize();
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
        Time nextPrintTime = 10.0;
        try
        {
            while (true)
            {
                auto T = fed->requestTime(runToTime);
                if (T < runToTime)
                {
                    captureForCurrentTime(T);
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
        
    }
    /** add a subscription to record*/
    void Recorder::addSubscription(const std::string &key)
    {
        auto res = subkeys.find(key);
        if ((res == subkeys.end())||(res->second==-1))
        {
            subscriptions.push_back(helics::Subscription(fed.get(), key));
            auto index = static_cast<int>(subscriptions.size())-1;
            auto id = subscriptions.back().getID();
            subids[id]= index; //this is a new element
            subkeys[key]=index; //this is a potential replacement
        }
    }
    /** add an endpoint*/
    void Recorder::addEndpoint(const std::string &endpoint)
    {
        auto res = eptNames.find(endpoint);
        if ((res == eptNames.end()) || (res->second == -1))
        {
            endpoints.push_back(helics::Endpoint(GLOBAL,fed.get(), endpoint));
            auto index = static_cast<int>(endpoints.size())-1;
            auto id = endpoints.back().getID();
            eptids.emplace(id, index); //this is a new element
            eptNames[endpoint]= index; //this is a potential replacement
        }
    }
    
    void Recorder::addSourceEndpointClone(const std::string &sourceEndpoint)
    {
        if (!cFilt)
        {
            cFilt = std::make_unique<CloningFilter>(fed.get());
            cloneEndpoint = std::make_unique<Endpoint>(fed.get(), "cloneE");
            cFilt->addDeliveryEndpoint(cloneEndpoint->getName());
        }
        cFilt->addSourceTarget(sourceEndpoint);
    }
    
    void Recorder::addDestEndpointClone(const std::string &destEndpoint)
    {
        if (!cFilt)
        {
            cFilt = std::make_unique<CloningFilter>(fed.get());
            cloneEndpoint = std::make_unique<Endpoint>(fed.get(), "cloneE");
            cFilt->addDeliveryEndpoint(cloneEndpoint->getName());
        }
        cFilt->addDestinationTarget(destEndpoint);
    }

    void Recorder::addCapture(const std::string &captureDesc)
    {
        captureInterfaces.push_back(captureDesc);
    }

    std::pair<std::string, std::string> Recorder::getValue(int index) const
    {
        if (isValidIndex(index, points))
        {
            return { subscriptions[points[index].index].getKey(),points[index].value };
        }
        return { std::string(),std::string() };
    }
    
    std::unique_ptr<Message> Recorder::getMessage(int index) const
    {
        if (isValidIndex(index, messages))
        {
            return std::make_unique<Message>(*messages[index]);
        }
        return nullptr;
    }

    /** save the data to a file*/
    void Recorder::saveFile(const std::string &filename)
    {
        auto ext = filesystem::path(filename).extension().string();
        if ((ext == ".json") || (ext == ".JSON"))
        {
            writeJsonFile(filename);
        }
        else
        {
            writeTextFile(filename);
        }
    }

    int Recorder::loadArguments(boost::program_options::variables_map &vm_map)
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
                auto taglist = stringOps::splitlineQuotes(tag);
                for (const auto &tagname : taglist)
                {
                    subkeys.emplace(stringOps::removeQuotes(tagname),-1);
                }
            }
        }
        // get the extra tags from the arguments
        if (vm_map.count("endpoints") > 0)
        {
            auto argEpt = vm_map["endpoints"].as<std::vector<std::string>>();
            for (const auto &ept : argEpt)
            {
                auto eptlist = stringOps::splitlineQuotes(ept);
                for (const auto &eptname : eptlist)
                {
                    eptNames.emplace(stringOps::removeQuotes(eptname), -1);
                }
            }
        }

        // capture the all the publications from a particular federate
        if (vm_map.count("capture") > 0)
        {
            auto captures = vm_map["capture"].as<std::vector<std::string>>();
            for (const auto &capt : captures)
            {
                auto captFeds = stringOps::splitlineQuotes(capt);
                for (auto &captFed : captFeds)
                {
                    auto actCapt = stringOps::removeQuotes(captFed);
                    captureInterfaces.push_back(actCapt);
                }
            }
        }

        if (vm_map.count("clone")>0)
        {
            auto clones = vm_map["clone"].as<std::vector<std::string>>();
            for (const auto &clone : clones)
            {
                addDestEndpointClone(clone);
                addSourceEndpointClone(clone);
            }
        }

        if (vm_map.count("sourceclone")>0)
        {
            auto clones = vm_map["sourceclone"].as<std::vector<std::string>>();
            for (const auto &clone : clones)
            {
                addSourceEndpointClone(clone);
            }
        }

        if (vm_map.count("destclone")>0)
        {
            auto clones = vm_map["destclone"].as<std::vector<std::string>>();
            for (const auto &clone : clones)
            {
                addDestEndpointClone(clone);
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


int recorderArgumentParser(int argc, const char *const *argv, po::variables_map &vm_map)
{
    po::options_description cmd_only ("command line only");
    po::options_description config ("configuration");
    po::options_description hidden ("hidden");

    // clang-format off
    // input boost controls
    cmd_only.add_options () 
		("help,?", "produce help message")
		("version,v","helics version number")
		("config-file", po::value<std::string> (),"specify a configuration file to use");


    config.add_options()
        ("stop", po::value<double>(), "the time to stop recording")
        ("tags",po::value<std::vector<std::string>>(),"tags to record, this argument may be specified any number of times")
        ("endpoints",po::value<std::vector<std::string>>(),"endpoints to capture, this argument may be specified multiple time")
        ("sourceclone", po::value<std::vector<std::string>>(), "existing endpoints to capture generated packets from, this argument may be specified multiple time")
        ("destclone", po::value<std::vector<std::string>>(), "existing endpoints to capture all packets with the specified endpoint as a destination, this argument may be specified multiple time")
        ("clone", po::value<std::vector<std::string>>(), "existing endpoints to clone all packets to and from")
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
        std::cout << visible << '\n';
        return (-1);
    }

    if (cmd_vm.count ("version") > 0)
    {
        std::cout << helics::getHelicsVersionString () << '\n';
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
