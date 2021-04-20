/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "../application_api/Endpoints.hpp"
#include "../application_api/Subscriptions.hpp"
#include "helicsApp.hpp"

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace helics {
class CloningFilter;

namespace apps {
    /** class designed to capture data points from a set of subscriptions or endpoints*/
    class HELICS_CXX_EXPORT Tracer: public App {
      public:
        /** construct from a FederateInfo structure*/
        explicit Tracer(const std::string& name, FederateInfo& fi);
        /** construct from command line arguments in a vector
   @param args the command line arguments to pass in a reverse vector
   */
        explicit Tracer(std::vector<std::string> args);

        /** construct from command line arguments*/
        Tracer(int argc, char* argv[]);

        /**constructor taking a federate information structure and using the given core
    @param name the name of the tracer object, if empty it tries to figure it out from fi
    @param core a pointer to core object which the federate can join
    @param fi  a federate information structure
    */
        Tracer(const std::string& name, const std::shared_ptr<Core>& core, const FederateInfo& fi);

        /**constructor taking a federate information structure and using the given core
    @param name the name of the federate (can be empty to use defaults from fi)
    @param core a coreApp object that can be joined
    @param fi  a federate information structure
    */
        Tracer(const std::string& name, CoreApp& core, const FederateInfo& fi);

        /**constructor taking a file with the required information
    @param name the name of the app may be empty to pull name from the file
    @param file a file defining the federate information
    */
        Tracer(const std::string& name, const std::string& file);
        /** move construction*/
        Tracer(Tracer&& other_tracer) = default;
        /** move assignment*/
        Tracer& operator=(Tracer&& tracer) = default;
        /**destructor*/
        ~Tracer();
        virtual void runTo(Time runToTime) override;
        /** add a subscription to capture*/
        void addSubscription(const std::string& key);
        /** add an endpoint*/
        void addEndpoint(const std::string& endpoint);
        /** copy all messages that come from a specified endpoint*/
        void addSourceEndpointClone(const std::string& sourceEndpoint);
        /** copy all messages that are going to a specific endpoint*/
        void addDestEndpointClone(const std::string& destEndpoint);
        /** add a capture interface
    @param captureDesc describes a federate to capture all the interfaces for
    */
        void addCapture(const std::string& captureDesc);

        /** set the callback for a message received through cloned interfaces
    @details the function signature will take the time in the Tracer a unique_ptr to the message
    */
        void setClonedMessageCallback(std::function<void(Time, std::unique_ptr<Message>)> callback)
        {
            clonedMessageCallback = std::move(callback);
        }
        /** set the callback for a message received through endpoints
    @details the function signature will take the time in the Tracer, the endpoint name as a string,
    and a unique_ptr to the message
    */
        void setEndpointMessageCallback(
            std::function<void(Time, const std::string&, std::unique_ptr<Message>)> callback)
        {
            endpointMessageCallback = std::move(callback);
        }
        /** set the callback for a value published
    @details the function signature will take the time in the Tracer, the publication key as a
    string, and the value as a string
    */
        void setValueCallback(
            std::function<void(Time, const std::string&, const std::string&)> callback)
        {
            valueCallback = std::move(callback);
        }
        /** turn the screen display on for values and messages*/
        void enableTextOutput() { printMessage = true; }
        /** turn the screen display off for values and messages*/
        void disableTextOutput() { printMessage = false; }

      private:
        /** load from a jsonString
    @param jsonString either a JSON filename or a string containing JSON
    */
        virtual void loadJsonFile(const std::string& jsonString) override;
        /** load a text file*/
        virtual void loadTextFile(const std::string& textFile) override;

        virtual void initialize() override;
        void generateInterfaces();
        void captureForCurrentTime(Time currentTime, int iteration = 0);
        void loadCaptureInterfaces();

        /** build the command line argument processing application*/
        std::shared_ptr<helicsCLI11App> buildArgParserApp();
        /** process remaining command line arguments*/
        void processArgs();

      protected:
        bool printMessage = false;
        bool allow_iteration =
            false;  //!< flag to allow iteration of the federate for time requests
        bool skiplog = false;  //!< skip the log function and print directly to cout
        std::unique_ptr<CloningFilter> cFilt;  //!< a pointer to a clone filter

        std::vector<Input> subscriptions;  //!< the actual subscription objects
        std::map<std::string, int> subkeys;  //!< translate subscription names to an index

        std::vector<Endpoint> endpoints;  //!< the actual endpoint objects
        std::map<std::string, int> eptNames;  //!< translate endpoint name to index
        std::unique_ptr<Endpoint> cloneEndpoint;  //!< the endpoint for cloned message delivery
        std::vector<std::string> captureInterfaces;  //!< storage for the interfaces to capture

        std::function<void(Time, std::unique_ptr<Message>)> clonedMessageCallback;
        std::function<void(Time, const std::string&, std::unique_ptr<Message>)>
            endpointMessageCallback;
        std::function<void(Time, const std::string&, const std::string&)> valueCallback;
    };

}  // namespace apps
}  // namespace helics
