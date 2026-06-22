/*
Copyright (c) 2017-2026,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include <ctime>
#include <fstream>
#include <helics/helics98.hpp>
#include <iostream>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>
#include <vector>

using json = nlohmann::json;

enum LogLevel { DEBUG, INFO, WARNING, ERROR };

class Logger {
  private:
    std::ofstream logFile;

    std::string levelToString(LogLevel level)
    {
        switch (level) {
            case DEBUG:
                return "DEBUG";
            case INFO:
                return "INFO";
            case WARNING:
                return "WARNING";
            case ERROR:
                return "ERROR";
            default:
                return "UNKNOWN";
        }
    }

  public:
    Logger(const std::string& fileName)
    {
        logFile.open(fileName, std::ios::app);
        if (!logFile.is_open()) {
            std::cerr << "Error opening log file: " << fileName << std::endl;
        }
    }

    ~Logger() { logFile.close(); }

    void log(LogLevel level, const std::string& message)
    {
        time_t now = time(0);
        tm* timeInfo = localtime(&now);
        char timestamp[20];
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", timeInfo);
        std::ostringstream logEntry;
        logEntry << "[" << timestamp << "]" << levelToString(level) << ":" << message;
        std::cout << "\r                                                  \r";
        std::cout << logEntry.str();
        if (logFile.is_open()) {
            logFile << logEntry.str() << std::endl;
        }
    }
};

class fedPub {
  public:
    json pubVal;
    helicscpp::Publication pub;
    fedPub(helicscpp::Publication p)
    {
        pub = p;
        pubVal = json::parse(pub.getInfo());
    }
};

class fedSub {
  public:
    json subVal;
    helicscpp::Input sub;
    fedSub(helicscpp::Input s) { sub = s; }
};

class fedEp {
  public:
    json epOutVal;
    json epInVal;
    helicscpp::Endpoint ep;
    fedEp(helicscpp::Endpoint e) { ep = e; }
};

class HelicsCppFederate {
  private:
    Logger* fedLog = nullptr;
    json helicsFederateCfg;
    helicscpp::CombinationFederate* federate = nullptr;
    helicscpp::Broker* broker = nullptr;
    HelicsTime endTime;
    std::vector<fedPub> pubs;
    std::vector<fedSub> subs;
    std::vector<fedEp> eps;
    void runBroker(void)
    {
        broker = new helicscpp::Broker(std::string("zmq"),
                                       std::string("broker"),
                                       std::string("-f 1 --loglevel=ERROR"));
        if (!broker->isConnected()) {
            throw std::runtime_error("Creating HELICS Broker failed!");
        }
    }
    void registerWithHelics(void)
    {
        fedLog->log(LogLevel::INFO, std::string("Creating HELICS Broker.\n"));
        runBroker();
        fedLog->log(LogLevel::INFO, std::string("Registering with HELICS Broker.\n"));
        federate = new helicscpp::CombinationFederate(helicsFederateCfg.dump());
        fedLog->log(LogLevel::INFO, std::string("Successful Registration with HELICS Broker.\n"));
        int pubCount = federate->getPublicationCount();
        int subCount = federate->getInputCount();
        int epCount = federate->getEndpointCount();
        for (int i = 0; i < pubCount; ++i) {
            fedPub* fp = new fedPub(federate->getPublication(i));
            pubs.push_back(*fp);
        }
        for (int i = 0; i < subCount; ++i) {
            fedSub* fs = new fedSub(federate->getInput(i));
            subs.push_back(*fs);
        }
        for (int i = 0; i < epCount; ++i) {
            fedEp* fe = new fedEp(federate->getEndpoint(i));
            eps.push_back(*fe);
        }
    }

    void disconnectFromHelics(void)
    {
        fedLog->log(LogLevel::INFO, std::string("Disconnecting from HELICS Broker.\n"));
        if (federate->getCurrentMode() != HelicsFederateState::HELICS_STATE_FINALIZE ||
            federate->getCurrentMode() != HelicsFederateState::HELICS_STATE_ERROR) {
            federate->finalize();
        }
        broker->disconnect();
        helicsCleanupLibrary();
        helicsCloseLibrary();
        free(federate);
        free(broker);
        federate = nullptr;
    }

    void processInput(fedSub sub)
    {
        std::stringstream complexSS;
        json helicsPayLoad;
        std::string subType(sub.sub.getType());
        sub.subVal.clear();
        sub.subVal = json::parse(sub.sub.getString());
        // fedLog->log(LogLevel::INFO, string("Input Updated!"));
        helicsPayLoad["HELICS_Time"] = federate->getCurrentTime();
        helicsPayLoad["Input_Target"] = sub.sub.getTarget();
        helicsPayLoad["Value"] = sub.subVal;
        // fedLog->log(LogLevel::INFO, format("{}\n", helicsPayLoad.dump(4)));
        helicsPayLoad.clear();
        // sub.sub.clearUpdate();
    }

    void processInputMessage(fedEp ep)
    {
        std::stringstream complexSS;
        json helicsPayLoad;
        ep.epInVal.clear();
        if (ep.ep.hasMessage()) {
            int msgCount = ep.ep.pendingMessageCount();
            // fedLog->log(LogLevel::INFO, string("Endpoint Received Messages!\n"));
            helicscpp::Message msg;
            for (int i = 0; i < msgCount; ++i) {
                msg = ep.ep.getMessage();
                ep.epInVal.clear();
                ep.epInVal = json::parse(msg.c_str());
                helicsPayLoad.clear();
                helicsPayLoad["HELICS_Time"] = federate->getCurrentTime();
                helicsPayLoad["endpoint"] = msg.destination();
                helicsPayLoad["source"] = msg.source();
                helicsPayLoad["Value"] = ep.epInVal;
                // fedLog->log(LogLevel::INFO, format("{}\n", helicsPayLoad.dump(4)));
                // msg.clear();
                // msg.release();
            }
        }
    }

    void processPublication(fedPub pub)
    {
        std::stringstream complexSS;
        std::string pubType(pub.pub.getType());
        json helicsPayLoad;
        int i = 0;
        std::string strVal = "a";
        helicsPayLoad.clear();
        if (pub.pubVal.empty()) {
            pub.pubVal = json::parse(pub.pub.getInfo());
        }
        // fedLog->log(LogLevel::INFO, string("Publishing Publication!\n"));
        helicsPayLoad["HELICS_Time"] = federate->getCurrentTime();
        helicsPayLoad["publication"] = pub.pub.getName();
        helicsPayLoad["Value"] = pub.pubVal;
        // fedLog->log(LogLevel::INFO, format("{}\n", helicsPayLoad.dump(4)));
        pub.pub.publish(pub.pubVal.dump(4));
    }

    void processOutputMessage(fedEp ep)
    {
        std::string destination = std::string(ep.ep.getDefaultDestination());
        if (!destination.empty()) {
            std::stringstream complexSS;
            json helicsPayLoad;
            helicscpp::Message msg(ep.ep);
            complexSS.clear();
            if (ep.epOutVal.empty()) {
                ep.epOutVal = json::parse(ep.ep.getInfo());
            }
            helicsPayLoad.clear();
            msg.data(ep.epOutVal.dump(4));
            ep.ep.sendMessage(msg);
            // fedLog->log(LogLevel::INFO, string("Sending Message!\n"));
            helicsPayLoad["HELICS_Time"] = federate->getCurrentTime();
            helicsPayLoad["source"] = ep.ep.getName();
            helicsPayLoad["destination"] = ep.ep.getDefaultDestination();
            helicsPayLoad["Value"] = ep.epOutVal;
            // fedLog->log(LogLevel::INFO, format("{}\n", helicsPayLoad.dump(4)));
            // msg.clear();
            // msg.release();
        }
    }

  public:
    HelicsCppFederate(std::string federateConfigFile, HelicsTime duration)
    {
        fedLog = new Logger("HelicsCpp98FederateLog.txt");
        helicsFederateCfg.clear();
        federate = nullptr;
        std::ifstream fstm(federateConfigFile);
        helicsFederateCfg = json::parse(fstm);
        endTime = duration;
        registerWithHelics();
    }

    void runFederate(void)
    {
        HelicsTime currentTime = 0.0;
        federate->enterExecutingMode();
        fedLog->log(LogLevel::INFO, std::string("Entering Execution Mode."));
        while (currentTime <= endTime) {
            if (federate->getCurrentMode() == HelicsFederateState::HELICS_STATE_EXECUTION) {
                for (auto& sub : subs) {
                    if (sub.sub.isUpdated()) {
                        processInput(sub);
                    }
                }
                for (auto& ep : eps) {
                    processInputMessage(ep);
                }
                for (auto& pub : pubs) {
                    processPublication(pub);
                }
                for (auto& ep : eps) {
                    processOutputMessage(ep);
                }
                if (currentTime < endTime) {
                    currentTime = federate->requestTime(currentTime + 1.0);
                    int completionStatus = static_cast<int>(currentTime * 100 / endTime);
                    fedLog->log(LogLevel::INFO,
                                std::string("Cpp Federate Status: ") +
                                    std::to_string(completionStatus) + "% complete.");
                } else {
                    fedLog->log(LogLevel::INFO,
                                std::string("Cpp Federate Status: Simulation complete.\n"));
                    break;
                }
            } else {
                break;
            }
        }
        disconnectFromHelics();
    }
};

std::string usage(std::string programName)
{
    std::stringstream usageStream;
    std::string rv;
    usageStream.clear();
    usageStream << "Usage: " << programName
                << " federateConfigFile duration (optional)publishIterations\n";
    usageStream << "\tArguments:\n";
    usageStream << "\t\tfederateConfigFile: The file path for the helics federate config file.\n";
    usageStream << "\t\tduration: The run duration for the federate in seconds.\n";
    usageStream
        << "\t\tpublishIterations: An option argument for setting the number of times the federate will publish/send for each publication/endpoint in a single timestep. The default is 1."
        << std::endl;
    rv = usageStream.str();
    usageStream.clear();
    return rv;
}

int main(int argc, char* argv[])
{
    std::string federateConfigFile;
    HelicsTime duration;
    int publishIterations = 1;
    if (argc != 3) {
        std::stringstream errMsg;
        errMsg.clear();
        errMsg
            << "Incorrect number arguments provided. helicsCppFederate takes 2 positional arguments.\n";
        errMsg << usage(std::string(argv[0]));
        throw std::runtime_error(errMsg.str());
    } else {
        federateConfigFile = std::string(argv[1]);
        duration = static_cast<HelicsTime>(std::stod(std::string(argv[2])));
    }
    HelicsCppFederate federate(federateConfigFile, duration);
    federate.runFederate();
}
