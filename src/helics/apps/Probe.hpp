/*
Copyright (c) 2017-2023,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once
#include "../application_api/Endpoints.hpp"
#include "../application_api/HelicsPrimaryTypes.hpp"
#include "../application_api/Publications.hpp"
#include "helicsApp.hpp"

#include <deque>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

namespace helics::apps {

/** class implementing a probe federate, which will connect with all other probes in the federation
 * and send message back and forth at each timestep
 */
class HELICS_CXX_EXPORT Probe: public App {
  public:
    /** default constructor*/
    Probe() = default;
    /** construct from command line arguments in a vector
@param args The vector of string, the remaining arguments are returned in the args
*/
    explicit Probe(std::vector<std::string> args);
    /** construct from command line arguments
@param argc the number of arguments
@param argv the strings in the input
*/
    Probe(int argc, char* argv[]);
    /** construct from a federate info object
@param name the name of the source object (can be empty to use defaults from fi)
@param fi a pointer info object containing information on the desired federate configuration
*/
    Probe(std::string_view name, const FederateInfo& fi);
    /**constructor taking a federate information structure and using the given core
@param name the name of the source object (can be empty to use defaults from fi)
@param core a pointer to core object which the federate can join
@param fi  a federate information structure
*/
    Probe(std::string_view name, const std::shared_ptr<Core>& core, const FederateInfo& fi);

    /**constructor taking a federate information structure and using the given core
@param name the name of the federate (can be empty to use defaults from fi)
@param core a coreApp object that can be joined
@param fi  a federate information structure
*/
    Probe(std::string_view name, CoreApp& core, const FederateInfo& fi);
    /**constructor taking a file with the required information
@param name the name of the source object (can be empty to use defaults from fi)
@param configString file a JSON or TOML file or string containing configuration informatino
*/
    Probe(std::string_view name, const std::string& configString);

    /** move construction*/
    Probe(Probe&& other_source) = default;

    /** move assignment*/
    Probe& operator=(Probe&& fed) = default;

    /** initialize the source federate
@details connect all sources with a generator
*/
    virtual void initialize() override;

    /** run the source until the specified time
@param stopTime_input the desired stop time
*/
    virtual void runTo(Time stopTime_input) override;
    /** get the number of connections made*/
    int getConnections() const { return connections; }
    /** get the number of messages received*/
    int getMessageCount() const { return messagesReceived; }

  private:
    void runProbe();

    Endpoint endpoint;  //!< the actual endpoint objects
    int connections{0};  //!< count the number of connections
    int messagesReceived{0};  //!< count the number of messages received
};
}  // namespace helics::apps
