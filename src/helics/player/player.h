/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/

#ifndef HELICS_PLAYER_HPP
#define HELICS_PLAYER_HPP

#include "../application_api/Publications.hpp"
#include "../application_api/CombinationFederate.h"
#include "../application_api/Endpoints.hpp"

#include "../application_api/HelicsPrimaryTypes.h"
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
    class ValueSetter
    {
    public:
        Time time;
        int index;
        std::string type;
        std::string pubName;
        defV value;
    };


    /** class implementing a player object, which is capable of reading a file and generating interfaces
    and sending signals at the appropriate times
    @details  the player class is not threadsafe,  don't try to use it from multiple threads that will result in undefined behavior
    */
    class player
    {
    public:
        player() = default;
        player(int argc, char *argv[]);

       player(const FederateInfo &fi);
        /**constructor taking a federate information structure and using the given core
        @param core a pointer to core object which the federate can join
        @param[in] fi  a federate information structure
        */
        player(std::shared_ptr<Core> core, const FederateInfo &fi);
        /**constructor taking a file with the required information
        @param[in] file a file defining the federate information
        */
        player(const std::string &jsonString);

        /** move construction*/
        player(player &&other_player) = default;
        /** don't allow the copy constructor*/
        player(const player &other_player) = delete;
        /** move assignment*/
        player &operator= (player &&fed) = default;
        /** don't allow the copy assignment,  the default would fail anyway since federates are not copyable either*/
        player &operator= (const player &fed) = delete;
        ~player();

        /** load a file containing publication information*/
        void loadFile(const std::string &filename);
        /*run the player*/
        void run();

        void run(Time stopTime_input);

        /** add a publication to a player
        @param key the key of the publication to add
        @param type the type of the publication
        @param units the units associated with the publication
        */
        void addPublication( const std::string &key, helicsType_t type, const std::string &units="");
        
        /** add a data point to publish through a player
        @param pubTime the time of the publication
        @param key the key for the publication
        @param val the value to publish
        */
        template<class valType>
        void addPoint(Time pubTime, const std::string &key, const valType &val)
        {
            points.resize(points.size() + 1);
            points.back().time = pubTime;
            points.back().pubName = key;
            points.back().value = val;
        }
    private:
        int loadArguments(boost::program_options::variables_map &vm_map);
    private:
        std::shared_ptr<CombinationFederate> fed; //!< the federate created for the player
        std::vector<ValueSetter> points;    //!< the points to generate into the federation
        std::set<std::pair<std::string, std::string>> tags;    //!< sets of tags 
        std::vector<Publication> publications;  //!< the actual publication objects
        std::vector<Endpoint> endpoints;    //!< the actual endpoint objects
        std::map<std::string, int> pubids;  //!< publication id map
        std::map<std::string, int> eptids;  //!< endpoint id maps
        helics::helicsType_t defType = helics::helicsType_t::helicsString; //!< the default data type unless otherwise specified
        Time stopTime = Time::maxVal(); //!< the time the player should stop
    };
}

#endif // HELICS_PLAYER_HPP