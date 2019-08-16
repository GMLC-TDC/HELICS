/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once
#include "../core/Broker.hpp"
#include <memory>
#include <thread>

namespace helics
{
class ActionMessage;
class helicsCLI11App;

/** helper class defining some common functionality for brokers and cores that use different
communication methods*/
class BrokerServer
{
  protected:
  public:
    /** default constructor*/
    BrokerServer () noexcept;
    /** construct from command line arguments
    @param argc the number of arguments
    @param argv the strings in the input
    */
    BrokerServer (int argc, char *argv[]);
    /** construct from command line arguments parsed as a single string
    @param argString a merged string with all the arguments
    */
    explicit BrokerServer (const std::string &argString);
    /** destructor*/
    ~BrokerServer ();
    /** start the broker servers*/
    void startServers ();

  private:
    /** generate an argument processing app*/
    std::unique_ptr<helicsCLI11App> generateArgProcessing ();
    /** start the servers*/
    void startZMQserver ();

  private:
    bool zmq_server{false};
    bool zmq_ss_server{false};
    bool tcp_server{false};
    bool udp_server{false};
    bool mpi_server{false};
    std::string configFile_;
    std::vector<std::thread> serverloops;

  public:
};
}  // namespace helics
