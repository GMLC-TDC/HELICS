/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/

#ifndef HELICS_RECORDER_HPP
#define HELICS_RECORDER_HPP

#include "../application_api/Subscriptions.hpp"
#include "../application_api/CombinationFederate.h"
#include "../application_api/Endpoints.hpp"
#include <map>
#include <memory>
#include <set>

#include "PrecHelper.h"

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

    /** class designed to capture data points from a set of subscriptions or endpoints*/
    class recorder
    {
    public:
        /** construct from a FederateInfo structure*/
        recorder(FederateInfo &fi);
        /** construct from command line arguments*/
        recorder(int argc, char *argv[]);

        /**constructor taking a federate information structure and using the given core
        @param core a pointer to core object which the federate can join
        @param[in] fi  a federate information structure
        */
        recorder(std::shared_ptr<Core> core, const FederateInfo &fi);
        /**constructor taking a file with the required information
        @param[in] file a file defining the federate information
        */
        recorder(const std::string &jsonString);
        /** move construction*/
        recorder(recorder &&other_player) = default;
        /** don't allow the copy constructor*/
        recorder(const recorder &other_player) = delete;
        /** move assignment*/
        recorder &operator= (recorder &&fed) = default;
        /** don't allow the copy assignment,  the default would fail anyway since federates are not copyable either*/
        recorder &operator= (const recorder &fed) = delete;
        /** destructor*/
        ~recorder();

        /** load a file containing subscription information
        @param filename the name of the file to load (.txt, .json, or .xml
        @return 0 on success <0 on failure
        */
        int loadFile(const std::string &filename);

        /*run the player*/
        void run();
        /** run the player until the specified time*/
        void run(Time stopTime);
        /** add a subscription to capture*/
        void addSubscription(const std::string &key);
        /** add an endpoint*/
        void addEndpoint(const std::string &endpoint);
        /** copy all messages that come from a specified endpoint*/
        void addSourceEndpointClone(const std::string &sourceEndpoint);
        /** copy all messages that are going to a specific endpoint*/
        void addDestEndpointClone(const std::string &destEndpoint);

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
        @details makes a copy of a message and returns it in a unique ptr
        @param index the number of the message to retrieve
        */
        std::unique_ptr<Message> getMessage(int index) const;

        /** finalize the federate*/
        void finalize();

    private:
        /** load arguments through a variable map created through command line arguments
        */
        int loadArguments(boost::program_options::variables_map &vm_map);
        /** load from a jsonString
        @param either a json filename or a string containing json
        */
        int loadJsonFile(const std::string &jsonString);
        /** load a text file*/
        int loadTextFile(const std::string &textFile);
        /** helper function to write the date to a json file*/
        void writeJsonFile(const std::string &filename);
        /** helper function to write the date to a text file*/
        void writeTextFile(const std::string &filename);
    protected:
        std::shared_ptr<CombinationFederate> fed; //!< the federate 
        std::vector<ValueCapture> points;   //!< lists of points that were captured
        std::set<std::string> tags; //!< a set of tags to generate points from
        std::vector<Subscription> subscriptions;    //!< the actual subscription objects
        std::vector<Endpoint> endpoints;    //!< the actual endpoint objects
        std::vector<std::unique_ptr<Message>> messages; //!< list of messages
        std::map<helics::subscription_id_t, int> subids; //!< map of the subscription ids
        std::map<std::string, int> subkeys;
        std::map<helics::endpoint_id_t, int> eptids;
        std::map<std::string, int> eptNames;
        std::string mapfile;    //!< file name for the online file updator
        std::string outFileName;    //!< the final output file
        Time autoStopTime = Time::maxVal(); //!< the stop time
    };

} //namespace helics

#endif // HELICS_RECORDER_HPP