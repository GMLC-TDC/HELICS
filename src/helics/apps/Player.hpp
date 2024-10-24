/*
Copyright (c) 2017-2024,
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
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

namespace helics {
namespace apps {
    struct ValueSetter {
        Time time{Time::minVal()};
        int iteration = 0;
        int index{-1};
        std::string type;
        std::string pubName;
        defV value;
    };

    struct MessageHolder {
        Time sendTime{Time::minVal()};
        int index{-1};
        Message mess;
    };

    /** class implementing a Player object, which is capable of reading a file and generating
interfaces and sending signals at the appropriate times
@details  the Player class is not thread-safe,  don't try to use it from multiple threads without
external protection, that will result in undefined behavior
*/
    class HELICS_CXX_EXPORT Player: public App {
      public:
        /** default constructor*/
        Player() = default;
        /** construct from command line arguments in a vector
   @param args the command line arguments to pass in a reverse vector
   */
        explicit Player(std::vector<std::string> args);
        /** construct from command line arguments
    @param argc the number of arguments
    @param argv the strings in the input
    */
        Player(int argc, char* argv[]);
        /** construct from a federate info object
    @param name the name of the federate (can be empty to use defaults from fedInfo)
    @param fedInfo a pointer info object containing information on the desired federate
    configuration
    */
        explicit Player(std::string_view name, const FederateInfo& fedInfo);
        /**constructor taking a federate information structure and using the given core
    @param name the name of the federate (can be empty to use defaults from fedInfo)
    @param core a pointer to core object which the federate can join
    @param fedInfo  a federate information structure
    */
        Player(std::string_view name,
               const std::shared_ptr<Core>& core,
               const FederateInfo& fedInfo);
        /**constructor taking a federate information structure and using the given core
    @param name the name of the federate (can be empty to use defaults from fedInfo)
    @param core a coreApp object that can be joined
    @param fedInfo  a federate information structure
    */
        Player(std::string_view name, CoreApp& core, const FederateInfo& fedInfo);
        /**constructor taking a file with the required information
    @param appName the name of the app
    @param configString JSON, TOML or text file or JSON string defining the federate information and
    other configuration
    */
        Player(std::string_view appName, const std::string& configString);

        /** move construction*/
        Player(Player&& other_player) = default;
        /** move assignment*/
        Player& operator=(Player&& fed) = default;

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

        /** add a publication to a Player
    @param name the identifier of the publication to add
    @param type the type of the publication
    @param pubUnits the units associated with the publication
    */
        void addPublication(std::string_view name,
                            DataType type,
                            std::string_view pubUnits = std::string_view());

        /** add a publication to a Player
    @param key the key of the publication to add
    @param pubUnits the units associated with the publication
    */
        template<class valType>
        typename std::enable_if_t<helicsType<valType>() != DataType::HELICS_CUSTOM>
            addPublication(std::string_view key, std::string_view pubUnits = std::string_view())
        {
            if (!useLocal) {
                publications.emplace_back(
                    InterfaceVisibility::GLOBAL, fed.get(), key, helicsType<valType>(), pubUnits);
            } else {
                publications.emplace_back(fed.get(), key, helicsType<valType>(), pubUnits);
            }

            pubids[key] = static_cast<int>(publications.size()) - 1;
        }

        /** add an endpoint to the Player
    @param endpointName the name of the endpoint
    @param endpointType the named type of the endpoint
    */
        void addEndpoint(std::string_view endpointName,
                         std::string_view endpointType = std::string_view());
        /** add a data point to publish through a Player
    @param pubTime the time of the publication
    @param key the key for the publication
    @param val the value to publish
    */
        template<class valType>
        void addPoint(Time pubTime, std::string_view key, const valType& val)
        {
            points.resize(points.size() + 1);
            points.back().time = pubTime;
            points.back().pubName = key;
            points.back().value = val;
        }

        /** add a data point to publish through a Player
    @param pubTime the time of the publication
    @param iteration the iteration count on which the value should be published
    @param key the key for the publication
    @param val the value to publish
    */
        template<class valType>
        void addPoint(Time pubTime, int iteration, std::string_view key, const valType& val)
        {
            points.resize(points.size() + 1);
            points.back().time = pubTime;
            points.back().iteration = iteration;
            points.back().pubName = key;
            points.back().value = val;
        }
        /** add a message to a Player queue
    @param sendTime  the time the message should be sent
    @param src the source endpoint of the message
    @param dest the destination endpoint of the message
    @param payload the payload of the message
    */
        void addMessage(Time sendTime,
                        std::string_view src,
                        std::string_view dest,
                        std::string_view payload);

        /** add an event for a specific time to a Player queue
    @param sendTime  the time the message should be sent
    @param actionTime  the eventTime listed for the message
    @param src the source endpoint of the message
    @param dest the destination endpoint of the message
    @param payload the payload of the message
    */
        void addMessage(Time sendTime,
                        Time actionTime,
                        std::string_view src,
                        std::string_view dest,
                        std::string_view payload);

        /** get the number of points loaded*/
        auto pointCount() const { return points.size(); }
        /** get the number of messages loaded*/
        auto messageCount() const { return messages.size(); }
        /** get the number of publications */
        auto publicationCount() const { return publications.size(); }
        /** get the number of endpoints*/
        auto endpointCount() const { return endpoints.size(); }
        /** get the point from an index*/
        const auto& getPoint(int index) const { return points[index]; }
        /** get the messages from an index*/
        const auto& getMessage(int index) const { return messages[index]; }

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
        /** helper function to sort through the tags*/
        void sortTags();
        /** helper function to generate the publications*/
        void generatePublications();
        /** helper function to generate the used Endpoints*/
        void generateEndpoints();
        /** helper function to sort the points and link them to publications*/
        void cleanUpPointList();

        /** send all points and messages up to the specified time*/
        void sendInformation(Time sendTime, int iteration = 0);

        /** extract a time from the string based on Player parameters
    @param str the string containing the time
    @param lineNumber the lineNumber of the file which is used in case of invalid specification
    */
        helics::Time extractTime(std::string_view str, int lineNumber = 0) const;

      private:
        std::vector<ValueSetter> points;  //!< the points to generate into the federation
        std::vector<MessageHolder> messages;  //!< list of message to hold
        std::unordered_map<std::string, std::string> tags;  //!< map of the key and type strings
        std::set<std::string> epts;  //!< set of the used endpoints
        std::deque<Publication> publications;  //!< the actual publication objects
        std::deque<Endpoint> endpoints;  //!< the actual endpoint objects
        std::unordered_map<std::string_view, int> pubids;  //!< publication id map
        std::unordered_map<std::string_view, int> eptids;  //!< endpoint id maps
        helics::DataType defType =
            helics::DataType::HELICS_STRING;  //!< the default data type unless otherwise specified
        size_t pointIndex = 0;  //!< the current point index
        size_t messageIndex = 0;  //!< the current message index
        time_units units = time_units::sec;
        double timeMultiplier =
            1.0;  //!< specify the time multiplier for different time specifications
        Time nextPrintTimeStep =
            helics::timeZero;  //!< the time advancement period for printing markers
    };
}  // namespace apps
}  // namespace helics
