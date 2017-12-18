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

    class ValueCapture
    {
    public:
        helics::Time time;
        helics::subscription_id_t id;
        bool first = false;
        std::string value;
        ValueCapture() = default;
        ValueCapture(helics::Time t1, helics::subscription_id_t id1, const std::string &val)
            : time(t1), id(id1), value(val) {};
    };

    class ValueStats
    {
    public:
        helics::Time time = helics::Time::minVal();
        std::string lastVal;
        std::string key;
        int cnt = 0;
    };

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
        void run(helics::Time stopTime);
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
    private:
        int loadArguments(boost::program_options::variables_map &vm_map);
    protected:
        std::shared_ptr<CombinationFederate> fed;
        std::vector<ValueCapture> points;
        std::set<std::string> tags;
        std::vector<Subscription> subscriptions;
        std::vector<Endpoint> endpoints;
        std::map<helics::subscription_id_t, std::pair<std::string, std::string>> subids;
        std::map<std::string, int> eptids;
        std::string mapfile;
        std::string outFileName;
    };

} //namespace helics

#endif // HELICS_RECORDER_HPP