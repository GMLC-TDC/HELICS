/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "MultiBroker.hpp"

#include "../core/helicsCLI11JsonConfig.hpp"
#include "../core/BrokerFactory.hpp"
#include "../core/helicsCLI11.hpp"
#include "../network/CommsInterface.hpp"
#include "../network/NetworkBrokerData.hpp"

#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#ifdef ENABLE_TCP_CORE
#    include "../network/tcp/TcpCommsSS.h"
#endif

namespace helics {

static auto mfact =
    BrokerFactory::addBrokerType<MultiBroker>("multi", static_cast<int>(core_type::MULTI));

bool allowMultiBroker()
{
    return true;
}

static void loadTypeSpecificArgs(
    helics::core_type ctype,
    CommsInterface* comm,
    std::vector<std::string> args)
{
    if (comm == nullptr) {
        return;
    }
    (void)args;
    switch (ctype) {
        case core_type::TCP_SS:
#ifdef ENABLE_TCP_CORE
        {
            auto cm = dynamic_cast<tcp::TcpCommsSS*>(comm);
            helicsCLI11App tsparse;
            tsparse.add_option_function<std::vector<std::string>>(
                "--connections",
                [cm](const std::vector<std::string>& conns) { cm->addConnections(conns); },
                "target link connections");
            tsparse.allow_extras();
            tsparse.helics_parse(std::move(args));
        }
#endif
        break;
        case core_type::MPI:
            break;
        default:
            break;
    }
}

static std::unique_ptr<CommsInterface>
    generateComms(const std::string& type, const std::string& initString = std::string{})
{
    auto ctype = coreTypeFromString(type);

    NetworkBrokerData nbdata;
    auto parser = nbdata.commandLineParser("127.0.0.1");
    parser->helics_parse(initString);

    std::unique_ptr<CommsInterface> comm;

    if (comm) {
        comm->loadNetworkInfo(nbdata);
    }
    return comm;
}

MultiBroker::MultiBroker(const std::string& brokerName): CoreBroker(brokerName) {}

MultiBroker::MultiBroker() noexcept {}

void MultiBroker::loadComms()
{
    masterComm = generateComms("def");
    masterComm->setCallback(
        [this](ActionMessage&& M) { BrokerBase::addActionMessage(std::move(M)); });
}

MultiBroker::~MultiBroker()
{
    BrokerBase::haltOperations = true;
    int exp = 2;
    while (!disconnectionStage.compare_exchange_weak(exp, 3)) {
        if (exp == 0) {
            MultiBroker::brokerDisconnect();
            exp = 1;
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }
    masterComm.reset(); // need to ensure the comms are deleted before the callbacks become invalid
    BrokerBase::joinAllThreads();
}

bool MultiBroker::brokerConnect()
{
    if ((netInfo.brokerName.empty()) && (netInfo.brokerAddress.empty())) {
        CoreBroker::setAsRoot();
    }
    masterComm = CommFactory::create(type);
    masterComm->setCallback(
        [this](ActionMessage&& M)
        {
            BrokerBase::addActionMessage(std::move(M));
        });
    masterComm->setLoggingCallback(BrokerBase::getLoggingCallback());
    masterComm->setName(getIdentifier());
    masterComm->loadNetworkInfo(netInfo);
    masterComm->setTimeout(networkTimeout.to_ms());

    auto res = masterComm->connect();
    BrokerFactory::addAssociatedBrokerType(getIdentifier(), type);
    return res;
}

void MultiBroker::brokerDisconnect() {
    int exp = 0;
    if (disconnectionStage.compare_exchange_strong(exp, 1)) {
        if (masterComm) {
            masterComm->disconnect();
        }
        for (auto& comm : comms) {
            comm->disconnect();
        }
        disconnectionStage = 2;
    }
}

bool MultiBroker::tryReconnect()
{
    return masterComm->reconnect();
}

std::shared_ptr<helicsCLI11App> MultiBroker::generateCLI()
{
    auto app = CoreBroker::generateCLI();
    CLI::App_p netApp = netInfo.commandLineParser("127.0.0.1", false);
    app->add_subcommand(netApp);
    app->addTypeOption();
    app->setDefaultCoreType(type);
    auto* app_p = app.get();
    app->final_callback([this, app_p]() {
        configFile = app_p->get_parent()->get_option("--config")->as<std::string>();
        type = app_p->getCoreType();
    });
    return app;
}

std::string MultiBroker::generateLocalAddressString() const
{
    switch (type)
    {
        case core_type::INPROC:
        case core_type::IPC:
        case core_type::INTERPROCESS:
        case core_type::TEST:
            return getIdentifier();
        default:
            break;
    }
    auto netcomm = dynamic_cast<NetworkCommsInterface*>(masterComm.get());
    if (netcomm) {
        return netcomm->getAddress();
    }
    return getIdentifier();
}

void MultiBroker::transmit(route_id rid, const ActionMessage& cmd)
{
    if (rid == parent_route_id) {
        if (masterComm) {
            masterComm->transmit(rid, cmd);
            return;
            }
    }
    else if (comms.empty())
    {
        if (masterComm) {
            masterComm->transmit(rid, cmd);
            return;
        }
    }
    else
    {

    }
}

void MultiBroker::transmit(route_id rid, ActionMessage&& cmd)
{
    if (rid == parent_route_id) {
        if (masterComm) {
            masterComm->transmit(rid, cmd);
            return;
        }
    } else if (comms.empty()) {
        if (masterComm) {
            masterComm->transmit(rid, cmd);
            return;
        }
    } else {
    }
}

void MultiBroker::addRoute(route_id rid, const std::string& routeInfo)
{
    masterComm->addRoute(rid, routeInfo);
}

void MultiBroker::removeRoute(route_id rid)
{
    masterComm->removeRoute(rid);
}

} // namespace helics
