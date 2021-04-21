/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "MultiBroker.hpp"

#include "../core/BrokerFactory.hpp"
#include "../core/helicsCLI11.hpp"
#include "../core/helicsCLI11JsonConfig.hpp"
#include "../network/CommsInterface.hpp"
#include "../network/NetworkBrokerData.hpp"
#include "../network/NetworkCommsInterface.hpp"

#include <atomic>
#include <mutex>
#include <thread>

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

/*
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
            auto* cm = dynamic_cast<tcp::TcpCommsSS*>(comm);
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
        default:
            break;
    }
}
*/
MultiBroker::MultiBroker(const std::string& brokerName): CoreBroker(brokerName) {}

MultiBroker::MultiBroker() noexcept {}

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
    masterComm.reset();  // need to ensure the comms are deleted before the callbacks become invalid
    BrokerBase::joinAllThreads();
}

bool MultiBroker::brokerConnect()
{
    std::shared_ptr<helicsCLI11App> app;
    std::string configString = "--config='" + configFile + '\'';
    if (!configFile.empty()) {
        app = netInfo.commandLineParser("");
        app->addTypeOption();
        app->allow_config_extras(CLI::config_extras_mode::error);
    } else if (type == core_type::MULTI) {
        type = core_type::DEFAULT;
    }
    try {
        if (type == core_type::MULTI) {
            app->get_config_formatter_base()->section("master");
            app->setDefaultCoreType(type);
            app->parse(configString);
            type = app->getCoreType();
        }
        if (type != core_type::MULTI) {
            if ((netInfo.brokerName.empty()) && (netInfo.brokerAddress.empty())) {
                CoreBroker::setAsRoot();
            }
            masterComm = CommFactory::create(type);
            masterComm->setCallback(
                [this](ActionMessage&& M) { BrokerBase::addActionMessage(std::move(M)); });
            masterComm->setLoggingCallback(BrokerBase::getLoggingCallback());
            masterComm->setName(getIdentifier());
            masterComm->loadNetworkInfo(netInfo);
            masterComm->setTimeout(networkTimeout.to_ms());

            bool res = masterComm->connect();
            if (!res) {
                return false;
            }
            BrokerFactory::addAssociatedBrokerType(getIdentifier(), type);
        }
        bool moreComms = (!configFile.empty());
        if (moreComms) {
            // remove options that are used to specify a broker
            app->remove_option(app->get_option("--broker"));
            app->remove_option(app->get_option("--brokerport"));
            app->remove_option(app->get_option("--brokername"));
            app->remove_option(app->get_option("--brokeraddress"));
            app->remove_option(app->get_option("--autobroker"));
        }
        uint16_t index = 0;
        while (moreComms) {
            netInfo = NetworkBrokerData();  // to reset the networkBrokerData
            app->get_config_formatter_base()->section("comms")->index(index);
            app->setDefaultCoreType(core_type::MULTI);
            app->parse(configString);
            type = app->getCoreType();
            if (type != core_type::MULTI) {
                auto comm = CommFactory::create(type);
                comm->setCallback([this, index](ActionMessage&& M) {
                    if (M.action() == CMD_REG_BROKER) {
                        M.setExtraData(index + 1);
                    }
                    BrokerBase::addActionMessage(std::move(M));
                });
                comm->setLoggingCallback(BrokerBase::getLoggingCallback());
                comm->setName(getIdentifier());
                comm->loadNetworkInfo(netInfo);
                comm->setTimeout(networkTimeout.to_ms());

                bool res = comm->connect();
                comms.push_back(std::move(comm));
                if (!res) {
                    brokerDisconnect();
                    return false;
                }
                BrokerFactory::addAssociatedBrokerType(getIdentifier(), type);
            } else {
                moreComms = false;
            }
            ++index;
        }
    }
    catch (const CLI::Error& e) {
        std::ostringstream ss;
        app->exit(e, ss, ss);
        CoreBroker::loggerFunction(helics_log_level_error, getIdentifier(), ss.str());
        brokerDisconnect();
        return false;
    }
    catch (...) {
        brokerDisconnect();
        return false;
    }
    return true;
}

void MultiBroker::brokerDisconnect()
{
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
    // this is a null flag option for forcing the callback to run
    app->add_flag("-_", "")->group("")->force_callback();
    auto* app_p = app.get();
    app->final_callback([this, app_p]() {
        auto* copt = app_p->get_parent()->get_option("--config");
        if (copt->count() > 0) {
            configFile = app_p->get_parent()->get_option("--config")->as<std::string>();
        }
        type = app_p->getCoreType();
    });
    return app;
}

std::string MultiBroker::generateLocalAddressString() const
{
    switch (type) {
        case core_type::INPROC:
        case core_type::IPC:
        case core_type::INTERPROCESS:
        case core_type::TEST:
            return getIdentifier();
        default:
            break;
    }
    auto* netcomm = dynamic_cast<NetworkCommsInterface*>(masterComm.get());
    if (netcomm != nullptr) {
        return netcomm->getAddress();
    }
    return getIdentifier();
}

void MultiBroker::transmit(route_id rid, const ActionMessage& cmd)
{
    if (rid == parent_route_id || comms.empty()) {
        if (masterComm) {
            masterComm->transmit(rid, cmd);
            return;
        }
    } else {
        for (const auto& rtable : routingTable) {
            if (rtable.first == rid) {
                if (rtable.second == 0) {
                    masterComm->transmit(rid, cmd);
                } else {
                    comms[rtable.second - 1]->transmit(rid, cmd);
                }
                return;
            }
        }
    }
}

void MultiBroker::transmit(route_id rid, ActionMessage&& cmd)
{
    if (rid == parent_route_id || comms.empty()) {
        if (masterComm) {
            masterComm->transmit(rid, cmd);
            return;
        }
    } else {
        for (const auto& rtable : routingTable) {
            if (rtable.first == rid) {
                if (rtable.second == 0) {
                    masterComm->transmit(rid, cmd);
                } else {
                    comms[rtable.second - 1]->transmit(rid, cmd);
                }
                return;
            }
        }
    }
}

void MultiBroker::addRoute(route_id rid, int interfaceId, const std::string& routeInfo)
{
    if (interfaceId <= 0) {
        if (masterComm) {
            masterComm->addRoute(rid, routeInfo);
            routingTable.emplace_back(rid, 0);
        }
    } else {
        if (isValidIndex(interfaceId - 1, comms)) {
            comms[interfaceId - 1]->addRoute(rid, routeInfo);
            routingTable.emplace_back(rid, interfaceId);
        }
    }
}

void MultiBroker::removeRoute(route_id rid)
{
    for (auto it = routingTable.begin(); it != routingTable.end(); ++it) {
        if (it->first == rid) {
            routingTable.erase(it);
            return;
        }
    }
}

}  // namespace helics
