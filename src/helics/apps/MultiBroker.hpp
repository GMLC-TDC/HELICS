/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once
#include "../core/CoreBroker.hpp"

#include <memory>
#include <string>
#include <vector>

namespace helics {
class ActionMessage;
class CommsInterface;

/** helper class defining some common functionality for brokers and cores that use different
communication methods*/
class MultiBroker: public CoreBroker {
  protected:
    std::atomic<int> disconnectionStage{0}; //!< the stage of disconnection
    std::vector<std::unique_ptr<CommsInterface>> comms; //!< the actual comms objects
    std::unique_ptr<CommsInterface> masterComm; //!< the primary comms object or the one that links with the master
    std::atomic<bool> brokerInitialized{false}; //!< atomic protecting local initialization
  public:
    /** default constructor*/
    MultiBroker() noexcept;
    
    /** construct from command line arguments
    @param brokerName the name of the broker
    @param argc the number of arguments
    @param argv the strings in the input
    */
    MultiBroker(const std::string &brokerName, int argc, char* argv[]);

    /** construct from command line arguments parsed as a single string
    @param brokerName the name of the broker
    @param argString a merged string with all the arguments
    */
    MultiBroker(const std::string &brokerName, const std::string& configFile);
    /** construct from command line arguments
    @param argc the number of arguments
    @param argv the strings in the input
    */
    MultiBroker(int argc, char* argv[]);
    /** construct from command line arguments parsed as a single string
    @param argString a merged string with all the arguments
    */
    explicit MultiBroker(const std::string& argString);

    /** destructor*/
    ~MultiBroker();

  private:
    virtual bool brokerConnect() override;
    virtual void brokerDisconnect() override;
    virtual bool tryReconnect() override;
    /** disconnect the comm object*/
    void commDisconnect();
    void loadComms();

  public:
    virtual void transmit(route_id rid, const ActionMessage& cmd) override;
    virtual void transmit(route_id rid, ActionMessage&& cmd) override;

    virtual void addRoute(route_id rid, const std::string& routeInfo) override;
};
} // namespace helics
