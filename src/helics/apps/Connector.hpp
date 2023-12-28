/*
Copyright (c) 2017-2023,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "helicsApp.hpp"

#include <string_view>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace helics::apps {
    enum class InterfaceDirection
    {
        TO_FROM=-1,
        BIDIRECTIONAL=0,
        FROM_TO=1
    };

    struct Connection {
        std::string interface1;
        std::string interface2;
        InterfaceDirection direction;
        std::vector<std::string_view> tags;
    };

    /** class implementing a Connector object, which is capable of automatically connecting interfaces in HELICS
@details  the Conncector class is not thread-safe,  don't try to use it from multiple threads without
external protection, that will result in undefined behavior
*/
    class HELICS_CXX_EXPORT Connector: public App {
      public:
        /** default constructor*/
        Connector() = default;
        /** construct from command line arguments in a vector
   @param args the command line arguments to pass in a reverse vector
   */
        explicit Connector(std::vector<std::string> args);
        /** construct from command line arguments
    @param argc the number of arguments
    @param argv the strings in the input
    */
        Connector(int argc, char* argv[]);
        /** construct from a federate info object
    @param name the name of the federate (can be empty to use defaults from fi)
    @param fi a pointer info object containing information on the desired federate configuration
    */
        explicit Connector(std::string_view name, const FederateInfo& fi);
        /**constructor taking a federate information structure and using the given core
    @param name the name of the federate (can be empty to use defaults from fi)
    @param core a pointer to core object which the federate can join
    @param fi  a federate information structure
    */
        Connector(std::string_view name, const std::shared_ptr<Core>& core, const FederateInfo& fi);
        /**constructor taking a federate information structure and using the given core
    @param name the name of the federate (can be empty to use defaults from fi)
    @param core a coreApp object that can be joined
    @param fi  a federate information structure
    */
        Connector(std::string_view name, CoreApp& core, const FederateInfo& fi);
        /**constructor taking a file with the required information
    @param appName the name of the app
    @param configString JSON, TOML or text file or JSON string defining the federate information and
    other configuration
    */
        Connector(std::string_view appName, const std::string& configString);

        /** move construction*/
        Connector(Connector&& other_player) = default;
        /** move assignment*/
        Connector& operator=(Connector&& fed) = default;

        /** initialize the Player federate
    @details generate all the publications and organize the points, the final publication count will
    be available after this time and the Player will enter the initialization mode, which means it
    will not be possible to add more publications calling run will automatically do this if
    necessary
    */
        virtual void initialize() override;

        /** run the Player until the specified time
    @param stopTime_input the desired stop time
    */
        virtual void runTo(Time stopTime_input) override;

        /** add a connection to a connector
    @param name the identifier of the first interface
    @param type the type of the publication
    @param pubUnits the units associated with the publication
    */
        void addConnection(std::string_view interface1,
                            std::string_view interface2,
                            InterfaceDirection direction=InterfaceDirection::BIDIRECTIONAL,
            std::vector<std::string> tags = {});

        /** add a tag for later reference return a string_view reference for the tag*/
        std::string_view Connector::addTag(const std::string &tagName);
        /** get the number of points loaded*/
        auto connectionCount() const { return connections.size(); }
        /** get the number of messages loaded*/
        auto madeConnections() const { return 0; }
        
      private:
        std::unique_ptr<helicsCLI11App> generateParser();
        /** process remaining command line arguments*/
        void processArgs();
        /** load from a jsonString
    @param jsonString either a JSON filename or a string containing JSON
    */
        virtual void loadJsonFile(const std::string& jsonString) override;
        /** load a text file*/
        virtual void loadTextFile(const std::string& filename) override;

        virtual bool addConnectionVector(const std::vector<std::string> &v1);
      private:
        std::unordered_multimap<std::string_view, Connection> connections;  //!< the connections descriptors
        std::vector<Connection> matchers;
        std::unordered_set<std::string> tags;
        std::uint64_t matchCount{0};
        bool prematch{false};
        
    };
}  
