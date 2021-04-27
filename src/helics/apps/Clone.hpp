/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once
#include "../application_api/Endpoints.hpp"
#include "../application_api/Publications.hpp"
#include "../application_api/Subscriptions.hpp"
#include "helicsApp.hpp"

#include <map>
#include <memory>
#include <set>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace helics {
class CloningFilter;

namespace apps {
    /** class designed to capture data points from a set of subscriptions or endpoints*/
    class HELICS_CXX_EXPORT Clone: public App {
      public:
        /** construct from a FederateInfo structure
    @param appName the name of the Recorder, can be left empty for the default or to pull from the
    federateInfo object
    @param fi  a federate information structure
    */
        Clone(const std::string& appName, FederateInfo& fi);
        /** construct from command line arguments in a vector
   @param args the command line arguments to pass in a reverse vector
   */
        explicit Clone(std::vector<std::string> args);
        /** construct from command line arguments*/
        Clone(int argc, char* argv[]);

        /**constructor taking a federate information structure and using the given core
    @param appName the name of the Recorder, can be left empty for the default or to pull from the
    federateInfo object
    @param core a pointer to core object which the federate can join
    @param fi  a federate information structure
    */
        Clone(const std::string& appName,
              const std::shared_ptr<Core>& core,
              const FederateInfo& fi);

        /**constructor taking a federate information structure and using the given core
    @param appName the name of the federate (can be empty to use defaults from fi)
    @param core a coreApp object that can be joined
    @param fi  a federate information structure
    */
        Clone(const std::string& appName, CoreApp& core, const FederateInfo& fi);

        /**constructor taking a file with the required information
    @param appName the name of the app
    @param jsonString a file or json string defining the federate information in JSON or text
    */
        Clone(const std::string& appName, const std::string& jsonString);
        /** move construction*/
        Clone(Clone&& other_recorder) = default;
        /** move assignment*/
        Clone& operator=(Clone&& record) = default;
        /** destructor*/
        ~Clone();
        /** run the Cloner until the specified time*/
        virtual void runTo(Time runToTime) override;
        /** save the data to a file*/
        void saveFile(const std::string& filename = std::string{});
        /** get the number of captured points*/
        auto pointCount() const { return points.size(); }
        /** get the number of captured messages*/
        auto messageCount() const { return messages.size(); }
        /** get a string with the value of point index
    @param index the number of the point to retrieve
    @return a pair with the tag as the first element and the value as the second
    */
        std::tuple<Time, std::string, std::string> getValue(int index) const;
        /** get a message
    @details makes a copy of a message and returns it in a unique_ptr
    @param index the number of the message to retrieve
    */
        std::unique_ptr<Message> getMessage(int index) const;

        /** set the name of the federate to Clone
    @param federateName the name of the federate to clone
    */
        void setFederateToClone(const std::string& federateName);
        /** set the name of the output file
    @param fileName  the name of the file, can be "" if no file should be auto saved*/
        void setOutputFile(std::string fileName) { outFileName = std::move(fileName); }

      private:
        /** add a subscription to capture*/
        void addSubscription(const std::string& key);

        /** copy all messages that come from a specified endpoint*/
        void addSourceEndpointClone(const std::string& sourceEndpoint);

        virtual void initialize() override;
        void generateInterfaces();
        void captureForCurrentTime(Time currentTime, int iteration = 0);
        /** build the command line argument processing application*/
        std::shared_ptr<helicsCLI11App> buildArgParserApp();
        /** process remaining command line arguments*/
        void processArgs();

      protected:
        /** helper class for capturing data points*/
        class ValueCapture {
          public:
            helics::Time time;
            int index{-1};
            int16_t iteration{0};
            bool first{false};
            std::string value;
            ValueCapture() = default;
            ValueCapture(helics::Time t1, int id1, const std::string& val):
                time(t1), index(id1), value(val)
            {
            }
        };

        bool allow_iteration{false};  //!< trigger to allow Iteration
        bool verbose{false};  //!< print all captured values to the screen
        bool fileSaved{false};  //!< true if the file has been saved already
        Time nextPrintTimeStep{
            helics::timeZero};  //!< the time advancement period for printing markers
        std::unique_ptr<CloningFilter> cFilt;  //!< a pointer to a clone filter
        std::vector<ValueCapture> points;  //!< lists of points that were captured
        std::vector<Input> subscriptions;  //!< the actual subscription objects
        std::vector<std::string>
            cloneSubscriptionNames;  //!< string of the subscriptions of the cloned federate
        std::unique_ptr<Endpoint> cloneEndpoint;  //!< the endpoint for cloned message delivery
        std::vector<std::unique_ptr<Message>> messages;  //!< list of messages
        std::map<helics::interface_handle, int> subids;  //!< map of the subscription ids
        std::map<std::string, int> subkeys;  //!< translate subscription names to an index
        std::map<helics::interface_handle, int> eptids;  // translate subscription id to index
        std::map<std::string, int> eptNames;  //!< translate endpoint name to index
        std::string captureFederate;  //!< storage for the name of the federate to clone
        std::string fedConfig;  //!< storage for the federateConfiguration
        std::string outFileName{"clone.json"};  //!< the final output file
        std::vector<int> pubPointCount;  //!< a    vector containing the counts of each publication
    };

}  // namespace apps
}  // namespace helics
