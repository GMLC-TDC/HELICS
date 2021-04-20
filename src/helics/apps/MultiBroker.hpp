/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once
#include "../core/CoreBroker.hpp"
#include "../network/NetworkBroker.hpp"

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace helics {
class ActionMessage;
class CommsInterface;

// small function to call to force symbol linkages
bool allowMultiBroker();

/** helper class defining some common functionality for brokers and cores that use different
communication methods*/
class MultiBroker: public CoreBroker {
  protected:
    std::atomic<int> disconnectionStage{0};  //!< the stage of disconnection
    std::vector<std::unique_ptr<CommsInterface>> comms;  //!< the actual comms objects
    std::unique_ptr<CommsInterface>
        masterComm;  //!< the primary comms object or the one that links with the master
    NetworkBrokerData netInfo{
        interface_type::tcp};  //!< structure containing the networking information
    std::string configFile;  //!< the name of the config file in use
    std::atomic<bool> brokerInitialized{false};  //!< atomic protecting local initialization
    core_type type{core_type::MULTI};  //!< the core type of the master controller
    std::vector<std::pair<route_id, int>> routingTable;  // index of the routes
  public:
    /** default constructor*/
    MultiBroker() noexcept;

    /** construct from command line arguments
    @param brokerName the name of the broker
    */
    explicit MultiBroker(const std::string& brokerName);

    /** destructor*/
    ~MultiBroker();

  private:
    virtual bool brokerConnect() override;
    virtual void brokerDisconnect() override;
    virtual bool tryReconnect() override;
    /** generate a CLI11 Application for subprocesses for processing of command line arguments*/
    virtual std::shared_ptr<helicsCLI11App> generateCLI() override;

  protected:
    /** generate the local address information*/
    virtual std::string generateLocalAddressString() const override;

  public:
    virtual void transmit(route_id rid, const ActionMessage& cmd) override;
    virtual void transmit(route_id rid, ActionMessage&& cmd) override;

    virtual void addRoute(route_id rid, int interfaceId, const std::string& routeInfo) override;

    virtual void removeRoute(route_id rid) override;
};
}  // namespace helics
