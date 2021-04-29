/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "../application_api/Endpoints.hpp"
#include "helicsApp.hpp"

#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace helics {
namespace apps {
    /** class implementing an Echo object, which will generate endpoint interfaces and send a data
message back to the source at the with a specified delay
@details  the Echo class is NOT threadsafe in general,  don't try to use it from multiple threads
without external protection, that will result in undefined behavior.  One exception is the
setEchoDelay function is threadsafe, and const methods will not cause problems but may not give
consistent answers if used from multiple threads unless protected.
*/
    class HELICS_CXX_EXPORT Echo: public App {
      public:
        /** default constructor*/
        Echo() = default;
        /** construct from command line arguments
   @param args a vector of command line arguments
   */
        explicit Echo(std::vector<std::string> args);
        /** construct from command line arguments
    @param argc the number of arguments
    @param argv the strings in the input
    */
        Echo(int argc, char* argv[]);
        /** construct from a federate info object
    @param name the name of the echo app, can be empty to get name from fi or default
    @param fi a pointer info object containing information on the desired federate configuration
    */
        explicit Echo(const std::string& name, const FederateInfo& fi);
        /**constructor taking a federate information structure and using the given core
    @param name the name of the echo app, can be empty to get name from fi or default
    @param core a pointer to core object which the federate can join
    @param fi  a federate information structure
    */
        Echo(const std::string& name, const std::shared_ptr<Core>& core, const FederateInfo& fi);
        /**constructor taking a federate information structure and using the given core
    @param name the name of the federate (can be empty to use defaults from fi)
    @param core a coreApp object that can be joined
    @param fi  a federate information structure
    */
        Echo(const std::string& name, CoreApp& core, const FederateInfo& fi);
        /**constructor taking a file with the required information
    @param name the name of the echo app, can be empty to get name from fi or default
    @param jsonString file or JSON string defining the federate information and other configuration
    */
        Echo(const std::string& name, const std::string& jsonString);

        /** move construction*/
        Echo(Echo&& other_echo) noexcept;
        /** move assignment*/
        Echo& operator=(Echo&& other_echo) noexcept;

        /** run the Echo federate until the specified time
    @param stopTime_input the desired stop time
    */
        virtual void runTo(Time stopTime_input) override;

        /** add an endpoint to the Player
    @param endpointName the name of the endpoint
    @param endpointType the named type of the endpoint
    */
        void addEndpoint(const std::string& endpointName, const std::string& endpointType = "");

        /** get the number of points loaded*/
        auto echoCount() const { return echoCounter; }
        /** set the delay time
    Function is threadsafe
     */
        void setEchoDelay(Time delay);

        /** get the number of endpoints*/
        auto endpointCount() const { return endpoints.size(); }

      private:
        /** load information from a JSON file*/
        virtual void loadJsonFile(const std::string& jsonFile) override;
        /** echo an actual message from an endpoint*/
        void echoMessage(const Endpoint& ept, Time currentTime);

      private:
        /** process remaining command line arguments*/
        void processArgs();
        std::vector<Endpoint> endpoints;  //!< the actual endpoint objects
        Time delayTime = timeZero;  //!< respond to each message with the specified delay
        size_t echoCounter = 0;  //!< the current message index
        std::mutex delayTimeLock;  // mutex protecting delayTime
    };
}  // namespace apps
}  // namespace helics
