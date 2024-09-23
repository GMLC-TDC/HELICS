/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "CoreApp.hpp"
#include "helicsApp.hpp"

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace helics::apps {
enum class InterfaceDirection { TO_FROM = -1, BIDIRECTIONAL = 0, FROM_TO = 1 };

struct Connection {
    std::string_view interface1;
    std::string_view interface2;
    InterfaceDirection direction;
    std::vector<std::size_t> tags;
    std::shared_ptr<std::string> stringBuffer{nullptr};
};

struct ConnectionsList;
struct PotentialConnections;
class RegexMatcher;
class TemplateMatcher;

/** class implementing a Connector object, which is capable of automatically connecting interfaces
in HELICS
@details  the Connector class is not thread-safe,  don't try to use it from multiple threads
without external protection, that will result in undefined behavior
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
@param name the name of the federate (can be empty to use defaults from fedInfo)
@param fedInfo a federate info object containing information on the desired federate configuration
*/
    explicit Connector(std::string_view name, const FederateInfo& fedInfo);
    /**constructor taking a federate information structure and using the given core
@param name the name of the federate (can be empty to use defaults from fedInfo)
@param coreObj a pointer to core object which the federate can join
@param fedInfo  a federate information structure
*/
    Connector(std::string_view name,
              const std::shared_ptr<Core>& coreObj,
              const FederateInfo& fedInfo);
    /**constructor taking a federate information structure and using the given core
@param name the name of the federate (can be empty to use defaults from fedInfo)
@param coreObj a coreApp object that can be joined
@param fedInfo  a federate information structure
*/
    Connector(std::string_view name, CoreApp& coreObj, const FederateInfo& fedInfo);
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
will not be possible to add more publications; calling run will automatically do this if
necessary
*/
    virtual void initialize() override;

    /** run the Player until the specified time
@param stopTime_input the desired stop time
*/
    virtual void runTo(Time stopTime_input) override;

    /** add a connection to a connector
@param interface1 the identifier of the first interface
@param interface2 the identifier of the second interface
@param direction the directionality of the interface
@param tags any string tags associated with the connection
*/
    void addConnection(std::string_view interface1,
                       std::string_view interface2,
                       InterfaceDirection direction = InterfaceDirection::BIDIRECTIONAL,
                       const std::vector<std::string>& tags = {});

    /** add a tag for later reference, return a string_view reference for the tag*/
    std::size_t addTag(std::string_view tagName);

    /** add an interface name for later reference, return a string_view reference for the interface
     * name*/
    std::string_view addInterface(std::string_view interfaceName);

    /** get the number of connections*/
    auto connectionCount() const { return connections.size(); }
    /** get the number of made connections*/
    auto madeConnections() const { return matchCount; }
    void allowMultipleConnections(bool value = true) { matchMultiple = value; }
    void matchEndpointTargets(bool value = true) { matchTargetEndpoints = value; }

  private:
    std::unique_ptr<helicsCLI11App> generateParser();
    /** process remaining command line arguments*/
    void processArgs();

    /** run any initial setup operations including file loading*/
    void initialSetup();
    /** load from a jsonString
@param jsonString either a JSON filename or a string containing JSON
*/
    virtual void loadJsonFile(const std::string& jsonString,
                              bool enableFederateInterfaceRegistration) override;
    /** load a text file*/
    virtual void loadTextFile(const std::string& filename) override;

    bool addConnectionVector(const std::vector<std::string>& connection);
    /** go through and make potential connections between interfaces*/
    void establishPotentialInterfaces(ConnectionsList& possibleConnections);

    /** scan the defined Potential Interfaces For Connections*/
    void scanPotentialInterfaces(ConnectionsList& possibleConnections);

    /** scan the potential interface Templates for connections*/
    void scanPotentialInterfaceTemplates(ConnectionsList& possibleConnections);

    /** scan unconnected interfaces for possible connections to potential interfaces*/
    void scanUnconnectedInterfaces(ConnectionsList& possibleConnections);

    /** actually go through and make connections*/
    void makeConnections(ConnectionsList& possibleConnections);
    /** try to make a connection for an input*/
    int makeTargetConnection(
        std::string_view origin,
        const std::vector<std::size_t>& tagList,
        std::unordered_set<std::string_view>& possibleConnections,
        const std::unordered_multimap<std::string_view, std::string_view>& aliases,
        const std::function<void(std::string_view origin, std::string_view target)>& callback);
    bool makePotentialConnection(
        std::string_view interfaceName,
        const std::vector<std::size_t>& tagList,
        std::unordered_map<std::string_view, PotentialConnections>& potentials,
        const std::unordered_multimap<std::string_view, std::string_view>& aliases);

    bool makePotentialTemplateConnection(
        std::string_view interfaceName,
        const std::vector<std::size_t>& tagList,
        std::vector<TemplateMatcher>& potentialTemplates,
        const std::unordered_multimap<std::string_view, std::string_view>& aliases);

    bool checkPotentialConnection(
        std::string_view interface,
        const std::vector<std::size_t>& tagList,
        std::unordered_set<std::string_view>& possibleConnections,
        std::unordered_map<std::string_view, PotentialConnections>& potentials,
        std::vector<TemplateMatcher>& potentialTemplates,
        const std::unordered_multimap<std::string_view, std::string_view>& aliases);
    /** get a list of the possible connections to based on the connections map*/
    std::vector<Connection>
        buildPossibleConnectionList(std::string_view startingInterface,
                                    const std::vector<std::size_t>& tagList) const;
    /** load the regex matchers */
    void generateRegexMatchers();

  private:
    CoreApp core;
    /// the connections descriptors
    std::unordered_multimap<std::string_view, Connection> connections;
    std::vector<Connection> matchers;
    std::vector<std::shared_ptr<RegexMatcher>> regexMatchers;
    std::unordered_map<std::size_t, std::string> tags;
    std::unordered_set<std::string> interfaces;
    std::uint64_t matchCount{0};
    std::uint64_t interfacesRequested{0};
    /// indicator to match unconnected target endpoints default{false}
    bool matchTargetEndpoints{false};
    /// indicator to do multiple matches [default is to stop at first match]
    bool matchMultiple{false};
    /// indicator that regex matches should always be checked
    bool alwaysCheckRegex{false};
};
}  // namespace helics::apps
