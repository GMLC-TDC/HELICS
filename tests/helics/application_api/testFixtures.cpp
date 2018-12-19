/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include "testFixtures.hpp"
#include <boost/test/unit_test.hpp>

#include "helics/core/BrokerFactory.hpp"
#include "helics/core/CoreFactory.hpp"
#include <cctype>
#include <iostream>

#ifdef HELICS_HAVE_ZEROMQ
#define ZMQTEST "zmq",
#define ZMQTEST2 "zmq_2",
#define ZMQTEST3 "zmq_3",
#define ZMQTEST4 "zmq_4",
#else
#define ZMQTEST
#define ZMQTEST2
#define ZMQTEST3
#define ZMQTEST4
#endif

#ifndef DISABLE_TCP_CORE
#define TCPTEST "tcp",
#define TCPTEST2 "tcp_2",
#define TCPTEST3 "tcp_3",
#define TCPTEST4 "tcp_4",

#define TCPSSTEST "tcp",
#define TCPSSTEST2 "tcp_2",
#else
#define TCPTEST
#define TCPTEST2
#define TCPTEST3
#define TCPTEST4

#define TCPSSTEST
#define TCPSSTEST2
#endif

#ifndef DISABLE_IPC_CORE
#define IPCTEST "ipc",
#define IPCTEST2 "ipc_2",

#else
#define IPCTEST
#define IPCTEST2
#endif

#ifndef DISABLE_UDP_CORE
#define UDPTEST "udp",
#define UDPTEST2 "udp_2",
#define UDPTEST3 "udp_3",
#define UDPTEST4 "udp_4",

#else
#define UDPTEST
#define UDPTEST2
#define UDPTEST3
#define UDPTEST4

#endif

const std::vector<std::string> ztypes = {ZMQTEST ZMQTEST2 ZMQTEST3 ZMQTEST4};
const std::vector<std::string> core_types = {"test", ZMQTEST3 IPCTEST2 TCPTEST "test_2", ZMQTEST UDPTEST "test_3"};

const std::vector<std::string> core_types_2 = {IPCTEST2 TCPTEST2 "test_2", TCPSSTEST2 ZMQTEST2 UDPTEST2};

const std::vector<std::string> core_types_simple = {"test", TCPSSTEST IPCTEST TCPTEST ZMQTEST UDPTEST};
const std::vector<std::string> core_types_single = {"test", TCPSSTEST IPCTEST TCPTEST ZMQTEST UDPTEST "test_3",
                                                    TCPSSTEST2 ZMQTEST3 TCPTEST3 UDPTEST3};
const std::vector<std::string> core_types_all = {"test", TCPSSTEST IPCTEST2 TCPTEST "test_2",
                                                 ZMQTEST UDPTEST TCPSSTEST2 "test_3",
                                                 ZMQTEST3 IPCTEST ZMQTEST2 UDPTEST2 TCPTEST2 UDPTEST3 TCPTEST3
                                                 "test_4",
                                                 ZMQTEST4 TCPTEST4 UDPTEST4};
const std::vector<std::string> core_types_extended = {IPCTEST ZMQTEST2 UDPTEST2 TCPTEST2 UDPTEST3 TCPTEST3
                                                      "test_4",
                                                      ZMQTEST4 TCPTEST4 UDPTEST4};

const std::string defaultNamePrefix = "fed";

bool hasIndexCode (const std::string &type_name)
{
    if (std::isdigit (type_name.back ()) != 0)
    {
        if (*(type_name.end () - 2) == '_')
        {  // this check ignores the setup mode
            return true;
        }
    }
    return false;
}

int getIndexCode (const std::string &type_name) { return static_cast<int> (type_name.back () - '0'); }

auto StartBrokerImp (const std::string &core_type_name, const std::string &initialization_string)
{
    helics::core_type type;
    if (hasIndexCode (core_type_name))
    {
        std::string new_type (core_type_name.begin (), core_type_name.end () - 2);
        type = helics::coreTypeFromString (new_type);
    }
    else
    {
        type = helics::coreTypeFromString (core_type_name);
    }
    std::shared_ptr<helics::Broker> broker;
    switch (type)
    {
    case helics::core_type::TCP:
        broker = helics::BrokerFactory::create (type, initialization_string + " --reuse_address");
        break;
    case helics::core_type::IPC:
    case helics::core_type::INTERPROCESS:
        broker = helics::BrokerFactory::create (type, initialization_string + " --client");
        break;
    default:
        broker = helics::BrokerFactory::create (type, initialization_string);
    }
    return broker;
}

bool FederateTestFixture::hasIndexCode (const std::string &type_name)
{
    if (std::isdigit (type_name.back ()) != 0)
    {
        if (*(type_name.end () - 2) == '_')
        {  // this setup ignores the setup mode
            return true;
        }
    }
    return false;
}

int FederateTestFixture::getIndexCode (const std::string &type_name)
{
    return static_cast<int> (type_name.back () - '0');
}

FederateTestFixture::~FederateTestFixture ()
{
    for (auto &fed : federates)
    {
        if (fed && (!((fed->getCurrentMode () == helics::Federate::modes::finalize) ||
                      (fed->getCurrentMode () == helics::Federate::modes::error))))
        {
            fed->finalize ();
        }
    }
    federates.clear ();
    for (auto &broker : brokers)
    {
        if (ctype.compare (0, 3, "tcp") == 0)
        {
            broker->waitForDisconnect (std::chrono::milliseconds (2000));
        }
        else
        {
            broker->waitForDisconnect (std::chrono::milliseconds (2000));
        }

        if (broker->isConnected ())
        {
            std::cout << "forcing disconnect\n";
            broker->disconnect ();
        }
    }
    brokers.clear ();
    helics::cleanupHelicsLibrary ();
}

void FederateTestFixture::FullDisconnect ()
{
    for (auto &fed : federates)
    {
        if (fed && fed->getCurrentMode () != helics::Federate::modes::finalize)
        {
            fed->finalize ();
        }
    }
    federates.clear ();
    for (auto &broker : brokers)
    {
        if (broker->isConnected ())
        {
            broker->disconnect ();
        }
    }
    brokers.clear ();
    helics::cleanupHelicsLibrary ();
}

std::shared_ptr<helics::Broker> FederateTestFixture::AddBroker (const std::string &core_type_name, int count)
{
    return AddBroker (core_type_name, std::string ("-f ") + std::to_string (count));
}

std::shared_ptr<helics::Broker>
FederateTestFixture::AddBroker (const std::string &core_type_name, const std::string &initialization_string)
{
    std::shared_ptr<helics::Broker> broker;
    if (extraBrokerArgs.empty ())
    {
        broker = StartBrokerImp (core_type_name, initialization_string);
    }
    else
    {
        broker = StartBrokerImp (core_type_name, initialization_string + " " + extraBrokerArgs);
    }
    broker->setLoggingLevel (0);
    brokers.push_back (broker);
    return broker;
}
