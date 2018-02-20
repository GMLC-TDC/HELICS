/*
Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.
*/

#pragma once
#include "../application_api/Subscriptions.hpp"
#include "../application_api/CombinationFederate.hpp"
#include "../application_api/Endpoints.hpp"
#include <map>
#include <memory>
#include <set>

#include "PrecHelper.hpp"

namespace boost
{
    namespace program_options
    {
        class variables_map;
    }
}

namespace helics
{
    /** helper class for capturing data points*/
    class ValueCapture
    {
    public:
        helics::Time time;
        int index=-1;
        bool first = false;
        std::string value;
        ValueCapture() = default;
        ValueCapture(helics::Time t1, int id1, const std::string &val)
            : time(t1), index(id1), value(val) {};
    };

    /** helper class for displaying statistics*/
    class ValueStats
    {
    public:
        helics::Time time = helics::Time::minVal();
        std::string lastVal;
        std::string key;
        int cnt = 0;
    };

    class CloningFilter;

    /** class designed to capture data points from a set of subscriptions or endpoints*/
    class Recorder
    {
    public:
        /** construct from a FederateInfo structure*/
        explicit Recorder(FederateInfo &fi);
        /** construct from command line arguments*/
        Recorder(int argc, char *argv[]);

        /**constructor taking a federate information structure and using the given core
        @param core a pointer to core object which the federate can join
        @param[in] fi  a federate information structure
        */
        Recorder(const std::shared_ptr<Core> &core, const FederateInfo &fi);
        /**constructor taking a file with the required information
        @param[in] file a file defining the federate information
        */
        explicit Recorder(const std::string &jsonString);
        /** move construction*/
        Recorder(Recorder &&other_recorder) = default;
        /** don't allow the copy constructor*/
        Recorder(const Recorder &other_recorder) = delete;
        /** move assignment*/
        Recorder &operator= (Recorder &&record) = default;
        /** don't allow the copy assignment,  the default would fail anyway since federates are not copyable either*/
        Recorder &operator= (const Recorder &record) = delete;
        /** destructor*/
        ~Recorder();

        /** load a file containing subscription information
        @param filename the name of the file to load (.txt, .json, or .xml
        @return 0 on success <0 on failure
        */
        int loadFile(const std::string &filename);

        /*run the Player*/
        void run();
        /** run the Player until the specified time*/
        void run(Time stopTime);
        /** add a subscription to capture*/
        void addSubscription(const std::string &key);
        /** add an endpoint*/
        void addEndpoint(const std::string &endpoint);
        /** copy all messages that come from a specified endpoint*/
        void addSourceEndpointClone(const std::string &sourceEndpoint);
        /** copy all messages that are going to a specific endpoint*/
        void addDestEndpointClone(const std::string &destEndpoint);
        /** add a capture interface
        @param captureDesc describes a federate to capture all the interfaces for
        */
        void addCapture(const std::string &captureDesc);
        /** save the data to a file*/
        void saveFile(const std::string &filename);
        /** get the number of captured points*/
        auto pointCount() const
        {
            return points.size();
        }
        /** get the number of captured messages*/
        auto messageCount() const
        {
            return messages.size();
        }
        /** get a string with the value of point index
        @param index the number of the point to retrieve
        @return a pair with the tag as the first element and the value as the second
        */
        std::pair<std::string,std::string> getValue(int index) const;
        /** get a message 
        @details makes a copy of a message and returns it in a unique_ptr
        @param index the number of the message to retrieve
        */
        std::unique_ptr<Message> getMessage(int index) const;

        /** finalize the federate*/
        void finalize();

        /** check if the Recorder is ready to run*/
        bool isActive() const
        {
            return !deactivated;
        }
    private:
        /** load arguments through a variable map created through command line arguments
        */
        int loadArguments(boost::program_options::variables_map &vm_map);
        /** load from a jsonString
        @param either a JSON filename or a string containing JSON
        */
        int loadJsonFile(const std::string &jsonString);
        /** load a text file*/
        int loadTextFile(const std::string &textFile);
        /** helper function to write the date to a JSON file*/
        void writeJsonFile(const std::string &filename);
        /** helper function to write the date to a text file*/
        void writeTextFile(const std::string &filename);

        void initialize();
        void generateInterfaces();
        void captureForCurrentTime(Time currentTime);
        void loadCaptureInterfaces();
        /** encode the string in base64 if needed otherwise just return the string*/
        std::string encode(const std::string &str2encode);
    protected:
        std::shared_ptr<CombinationFederate> fed; //!< the federate 
        std::unique_ptr<CloningFilter> cFilt; //!< a pointer to a clone filter
        std::vector<ValueCapture> points;   //!< lists of points that were captured
        std::vector<Subscription> subscriptions;    //!< the actual subscription objects
        std::vector<Endpoint> endpoints;    //!< the actual endpoint objects
        std::unique_ptr<Endpoint> cloneEndpoint; //!< the endpoint for cloned message delivery
        std::vector<std::unique_ptr<Message>> messages; //!< list of messages
        std::map<helics::subscription_id_t, int> subids; //!< map of the subscription ids
        std::map<std::string, int> subkeys;  //!< translate subscription names to an index
        std::map<helics::endpoint_id_t, int> eptids; //translate subscription id to index
        std::map<std::string, int> eptNames;    //!< translate endpoint name to index
        std::vector<ValueStats> vStat; //!< storage for statistics capture
        std::vector<std::string> captureInterfaces;  //!< storage for the interfaces to capture
        std::string mapfile;    //!< file name for the on-line file updater
        std::string outFileName;    //!< the final output file
        Time autoStopTime = Time::maxVal(); //!< the stop time
        bool deactivated = false;
    };

} //namespace helics
