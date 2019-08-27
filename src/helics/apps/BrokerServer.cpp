/*
Copyright � 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "BrokerServer.hpp"

#include "../common/JsonProcessingFunctions.hpp"

#include "../core/ActionMessage.hpp"
#include "../core/BrokerFactory.hpp"
#include "../core/NetworkBrokerData.hpp"
#include "../core/helicsCLI11.hpp"
#include "../core/networkDefaults.hpp"

#ifdef ENABLE_ZMQ_CORE
#include "../common/zmqContextManager.h"
#include "../core/zmq/ZmqCommsCommon.h"
#endif

#include <random>

static std::string random_string (std::string::size_type length)
{
    static constexpr auto chars = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

    thread_local static std::mt19937 rg{std::random_device{}()};
    thread_local static std::uniform_int_distribution<std::string::size_type> pick (0, 61);

    std::string s;

    s.reserve (length);

    while (length--)
        s.push_back (chars[pick (rg)]);

    return s;
}

namespace helics
{
namespace apps
{
BrokerServer::BrokerServer () noexcept : zmq_server{true}, server_name_{random_string (5)} {}

BrokerServer::BrokerServer (int argc, char *argv[]) : server_name_{random_string (5)}
{
    auto app = generateArgProcessing ();
    app->parse (argc, argv);
}

BrokerServer::BrokerServer (std::vector<std::string> args) : server_name_{random_string (5)}
{
    auto app = generateArgProcessing ();
    app->parse (args);
}

BrokerServer::BrokerServer (const std::string &configFile)
    : server_name_{random_string (5)}, configFile_ (configFile)
{
}

BrokerServer::~BrokerServer () { closeServers (); }

void BrokerServer::startServers ()
{
    if (!configFile_.empty ())
    {
        config_ = std::make_unique<Json::Value> (loadJson (configFile_));
    }
    else
    {
        config_ = std::make_unique<Json::Value> ();
    }

#ifdef ENABLE_ZMQ_CORE
    if (zmq_server)
    {
        serverloops_.emplace_back ([this] () { startZMQserver (); });
    }
    if (zmq_ss_server)
    {
    }
#endif
}

bool BrokerServer::hasActiveBrokers () const { return BrokerFactory::brokersActive (); }
/** force terminate all running brokers*/
void BrokerServer::forceTerminate ()
{
    closeServers ();
    auto brokerList = BrokerFactory::getAllBrokers ();
    for (auto &brk : brokerList)
    {
        if (!brk)
        {
            continue;
        }
        if (brk->isConnected ())
        {
            brk->disconnect ();
        }
    }
}

void BrokerServer::closeServers ()
{
#ifdef ENABLE_ZMQ_CORE
    if (zmq_server)
    {
        closeZMQserver ();
    }
#endif
    exitall.store (true);

    for (auto &t : serverloops_)
    {
        t.join ();
    }
    serverloops_.clear ();
}

std::unique_ptr<helicsCLI11App> BrokerServer::generateArgProcessing ()
{
    auto app = std::make_unique<helicsCLI11App> (
      "The Broker server is a helics broker coordinator that can generate brokers on request", "broker_server");

    app->add_flag ("--zmq,-z", zmq_server, "start a broker-server for the zmq comms in helics");
    app->add_flag ("--zmqss", zmq_ss_server, "start a broker-server for the zmq single socket comms in helics");
    app->add_flag ("--tcp,-t", tcp_server, "start a broker-server for the tcp comms in helics");
    app->add_flag ("--udp,-u", udp_server, "start a broker-server for the udp comms in helics");
    app->add_flag ("--mpi", mpi_server, "start a broker-server for the mpi comms in helics");

    app->add_option ("config,--config,--server-config", configFile_, "load a config file for the broker server");
    return app;
}
/** find an existing broker or start a new one*/
static std::pair<std::shared_ptr<Broker>, bool>
findBroker (const ActionMessage &rx, core_type ctype, int startPort)
{
    std::string brkname;
    std::string brkinit;
    bool newbrk{false};
    auto &strs = rx.getStringData ();
    if (strs.size () > 0)
    {
        brkname = strs[0];
    }
    if (strs.size () > 1)
    {
        brkinit = strs[1] + " --external --localport=" + std::to_string (startPort);
    }
    else
    {
        brkinit = "--external --localport=" + std::to_string (startPort);
    }
    std::shared_ptr<Broker> brk;
    if (brkname.empty ())
    {
        brk = BrokerFactory::findJoinableBrokerOfType (ctype);
        if (!brk)
        {
            brk = BrokerFactory::create (ctype, brkinit);
            newbrk = true;
            brk->connect ();
        }
    }
    else
    {
        brk = BrokerFactory::findBroker (brkname);
        if (!brk)
        {
            brk = BrokerFactory::create (ctype, brkname, brkinit);
            newbrk = true;
            brk->connect ();
        }
    }
    return {brk, newbrk};
}

static ActionMessage generateReply (const ActionMessage &, std::shared_ptr<Broker> &brk)
{
    ActionMessage rep (CMD_PROTOCOL);
    rep.messageID = NEW_BROKER_INFORMATION;
    rep.name = brk->getIdentifier ();
    auto brkptr = extractInterfaceandPortString (brk->getAddress ());
    rep.setString (0, std::string ("?:") + brkptr.second);
    return rep;
}

using portData = std::vector<std::tuple<int, bool, std::shared_ptr<Broker>>>;

static int getOpenPort (portData &pd)
{
    for (auto &pdi : pd)
    {
        if (!std::get<1> (pdi))
        {
            return std::get<0> (pdi);
        }
    }
    for (auto &pdi : pd)
    {
        if (!std::get<2> (pdi)->isConnected ())
        {
            std::get<2> (pdi) = nullptr;
            std::get<1> (pdi) = false;
            return std::get<0> (pdi);
        }
    }
    return -1;
}

static void assignPort (portData &pd, int pnumber, std::shared_ptr<Broker> &brk)
{
    for (auto &pdi : pd)
    {
        if (std::get<0> (pdi) == pnumber)
        {
            std::get<1> (pdi) = true;
            std::get<2> (pdi) = brk;
            break;
        }
    }
}

void BrokerServer::startZMQserver ()
{
    std::cerr << "starting zmq broker server\n";
#ifdef ENABLE_ZMQ_CORE
    std::string ext_interface = "tcp://*";
    int port = DEFAULT_ZMQ_BROKER_PORT_NUMBER + 1;
    std::chrono::milliseconds timeout (20000);
    if (config_->isMember ("zmq"))
    {
        auto V = (*config_)["zmq"];
        replaceIfMember (V, "interface", ext_interface);
        replaceIfMember (V, "port", port);
    }
    auto ctx = ZmqContextManager::getContextPointer ();
    zmq::socket_t repSocket (ctx->getContext (), ZMQ_REP);
    repSocket.setsockopt (ZMQ_LINGER, 500);
    auto bindsuccess = hzmq::bindzmqSocket (repSocket, ext_interface, port, timeout);
    if (!bindsuccess)
    {
        repSocket.close ();
        std::cout << "ZMQ server failed to start\n";
        return;
    }
    zmq::pollitem_t poller;
    poller.socket = static_cast<void *> (repSocket);
    poller.events = ZMQ_POLLIN;

    portData pdata;
    for (int ii = 0; ii < 20; ++ii)
    {
        pdata.emplace_back (DEFAULT_ZMQ_BROKER_PORT_NUMBER + 4 + ii * 2, false, nullptr);
    }

    int rc = 0;
    while (rc >= 0)
    {
        rc = zmq::poll (&poller, 1, std::chrono::milliseconds (5000));
        if (rc < 0)
        {
            std::cerr << "ZMQ broker connection error (2)" << std::endl;
            break;
        }
        if (rc > 0)
        {
            zmq::message_t msg;
            repSocket.recv (&msg);
            auto sz = msg.size ();
            if (sz < 25)
            {
                if (std::string (static_cast<char *> (msg.data ()), msg.size ()) ==
                    std::string ("close_server:") + server_name_)
                {
                    //      std::cerr << "received close server message" << std::endl;
                    repSocket.send (msg);
                    break;
                }
                else
                {
                    //    std::cerr << "received unrecognized message (ignoring)"
                    //              << std::string (static_cast<char *> (msg.data ()), msg.size ()) << std::endl;
                    repSocket.send ("ignored");
                    continue;
                }
            }
            else
            {
                ActionMessage rxcmd (static_cast<char *> (msg.data ()), msg.size ());
                //   std::cout << "received data length " << msg.size () << std::endl;
                switch (rxcmd.action ())
                {
                case CMD_PROTOCOL:
                case CMD_PROTOCOL_PRIORITY:
                case CMD_PROTOCOL_BIG:
                    switch (rxcmd.messageID)
                    {
                    case REQUEST_PORTS:
                    {
                        auto pt = getOpenPort (pdata);
                        if (pt > 0)
                        {
                            auto nbrk = findBroker (rxcmd, core_type::ZMQ, pt);
                            if (nbrk.second)
                            {
                                assignPort (pdata, pt, nbrk.first);
                            }
                            auto mess = generateReply (rxcmd, nbrk.first);
                            auto str = mess.to_string ();
                            repSocket.send (str.data (), str.size ());
                        }
                        else
                        {
                            ActionMessage rep (CMD_PROTOCOL);
                            rep.messageID = DELAY;
                            auto str = rep.to_string ();
                            repSocket.send (str.data (), str.size ());
                        }
                    }
                    break;
                    }
                    break;
                default:
                    std::cout << "received unknown message " << msg.size () << std::endl;
                    repSocket.send ("ignored");
                    break;
                }
            }
        }
        if (exitall.load ())
        {
            //    std::cerr << "exit all active" << std::endl;
            break;
        }
    }
    repSocket.close ();
    std::cerr << "exiting zmq broker server" << std::endl;

#endif
}

void BrokerServer::closeZMQserver ()
{
#ifdef ENABLE_ZMQ_CORE
    auto ctx = ZmqContextManager::getContextPointer ();
    zmq::socket_t reqSocket (ctx->getContext (), ZMQ_REQ);
    reqSocket.setsockopt (ZMQ_LINGER, 300);
    std::string ext_interface = "tcp://127.0.0.1";
    int port = DEFAULT_ZMQ_BROKER_PORT_NUMBER + 1;
    if (config_->isMember ("zmq"))
    {
        auto V = (*config_)["zmq"];
        replaceIfMember (V, "interface", ext_interface);
        replaceIfMember (V, "port", port);
    }
    try
    {
        reqSocket.connect (helics::makePortAddress (ext_interface, port));
        reqSocket.send (std::string ("close_server:") + server_name_);
        reqSocket.close ();
    }
    catch (const zmq::error_t &)
    {
    }

#endif
}
}  // namespace apps
}  // namespace helics
