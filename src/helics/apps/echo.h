/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/

#ifndef HELICS_ECHO_HPP
#define HELICS_ECHO_HPP

#include "../application_api/MessageFederate.h"
#include "../application_api/Endpoints.hpp"

#include <map>
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
    
    /** class implementing a echo object, which will generate endpoint interfaces and send a data message back to the source at the same time as it was received
    @details  the player class is not threadsafe,  don't try to use it from multiple threads without external protection, that will result in undefined behavior
    */
    class echo
    {
    public:
        /** default constructor*/
        echo() = default;
        /** construct from command line arguments
        @param argc the number of arguments
        @param argv the strings in the input
        */
        echo(int argc, char *argv[]);
        /** construct from a federate info object
        @param fi a pointer info object containing information on the desired federate configuration
        */
      echo(const FederateInfo &fi);
        /**constructor taking a federate information structure and using the given core
        @param core a pointer to core object which the federate can join
        @param[in] fi  a federate information structure
        */
        echo(std::shared_ptr<Core> core, const FederateInfo &fi);
        /**constructor taking a file with the required information
        @param[in] jsonString file or json string defining the federate information and other configuration
        */
        echo(const std::string &jsonString);

        /** move construction*/
        echo(echo &&other_echo) = default;
        /** don't allow the copy constructor*/
        echo(const echo &other_echo) = delete;
        /** move assignment*/
        echo &operator= (echo &&fed) = default;
        /** don't allow the copy assignment,  the default would fail anyway since federates are not copyable either*/
        echo &operator= (const echo &fed) = delete;
        ~echo();

        /** load a file containing publication information
        @param filename the file containing the configuration and player data  accepted format are json, xml, and a player format which is tab delimited or comma delimited*/
        void loadFile(const std::string &filename);
        /** initialize the player federate
        @details generate all the publications and organize the points, the final publication count will be available after this time
        and the player will enter the initialization mode, which means it will not be possible to add more publications
        calling run will automatically do this if necessary
        */
        void initialize();
        /*run the echo federate*/
        void run();

        /** run the echo federate until the specified time
        @param stopTime_input the desired stop time
        */
        void run(Time stopTime_input);

        
        /** add an endpoint to the player
        @param endpointName the name of the endpoint
        @param endpointType the named type of the endpoint
        */
        void addEndpoint(const std::string &endpointName, const std::string &endpointType = "");

        /** get the number of points loaded*/
       auto echoCount() const
        {
            return echoCounter;
        }
     
        /** get the number of endpoints*/
        auto endpointCount() const
        {
            return endpoints.size();
        }
    private:
        int loadArguments(boost::program_options::variables_map &vm_map);
       

    private:
        std::shared_ptr<MessageFederate> fed; //!< the federate created for the player
        std::vector<Endpoint> endpoints;    //!< the actual endpoint objects
        Time stopTime = Time::maxVal(); //!< the time the player should stop
        size_t echoCounter = 0; //!< the current message index
    };
}

#endif // HELICS_PLAYER_HPP