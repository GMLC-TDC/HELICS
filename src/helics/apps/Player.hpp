/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once

#include "../application_api/CombinationFederate.hpp"
#include "../application_api/Endpoints.hpp"
#include "../application_api/Publications.hpp"

#include "../application_api/HelicsPrimaryTypes.hpp"
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
namespace apps
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

class MessageHolder
{
  public:
    Time sendTime;
    int index;
    Message mess;
};

/** class implementing a Player object, which is capable of reading a file and generating interfaces
and sending signals at the appropriate times
@details  the Player class is not thread-safe,  don't try to use it from multiple threads without external
protection, that will result in undefined behavior
*/
class Player
{
  public:
    /** default constructor*/
    Player () = default;
    /** construct from command line arguments
    @param argc the number of arguments
    @param argv the strings in the input
    */
    Player (int argc, char *argv[]);
    /** construct from a federate info object
    @param fi a pointer info object containing information on the desired federate configuration
    */
    explicit Player (const FederateInfo &fi);
    /**constructor taking a federate information structure and using the given core
    @param core a pointer to core object which the federate can join
    @param[in] fi  a federate information structure
    */
    Player (const std::shared_ptr<Core> &core, const FederateInfo &fi);
    /**constructor taking a file with the required information
    @param[in] jsonString file or JSON string defining the federate information and other configuration
    */
    explicit Player (const std::string &jsonString);

    /** move construction*/
    Player (Player &&other_player) = default;
    /** don't allow the copy constructor*/
    Player (const Player &other_player) = delete;
    /** move assignment*/
    Player &operator= (Player &&fed) = default;
    /** don't allow the copy assignment,  the default would fail anyway since federates are not copyable either*/
    Player &operator= (const Player &fed) = delete;
    ~Player ();

    /** load a file containing publication information
    @param filename the file containing the configuration and Player data  accepted format are JSON, xml, and a
    Player format which is tab delimited or comma delimited*/
    void loadFile (const std::string &filename);
    /** initialize the Player federate
    @details generate all the publications and organize the points, the final publication count will be available
    after this time and the Player will enter the initialization mode, which means it will not be possible to add
    more publications calling run will automatically do this if necessary
    */
    void initialize ();
    /*run the Player*/
    void run ();

    /** run the Player until the specified time
    @param stopTime_input the desired stop time
    */
    void run (Time stopTime_input);

    /** add a publication to a Player
    @param key the key of the publication to add
    @param type the type of the publication
    @param units the units associated with the publication
    */
    void addPublication (const std::string &key, helics_type_t type, const std::string &pubUnits = std::string ());

    /** add a publication to a Player
    @param key the key of the publication to add
    @param type the type of the publication
    @param units the units associated with the publication
    */
    template <class valType>
    typename std::enable_if_t<helicsType<valType> () != helics_type_t::helicsInvalid>
    addPublication (const std::string &key, const std::string &pubUnits = std::string ())
    {
        if (!useLocal)
        {
            publications.push_back (Publication (GLOBAL, fed.get (), key, helicsType<valType> (), pubUnits));
        }
        else
        {
            publications.push_back (Publication (fed.get (), key, helicsType<valType> (), pubUnits));
        }

        pubids[key] = static_cast<int> (publications.size ()) - 1;
    }

    /** add an endpoint to the Player
    @param endpointName the name of the endpoint
    @param endpointType the named type of the endpoint
    */
    void addEndpoint (const std::string &endpointName, const std::string &endpointType = std::string ());
    /** add a data point to publish through a Player
    @param pubTime the time of the publication
    @param key the key for the publication
    @param val the value to publish
    */
    template <class valType>
    void addPoint (Time pubTime, const std::string &key, const valType &val)
    {
        points.resize (points.size () + 1);
        points.back ().time = pubTime;
        points.back ().pubName = key;
        points.back ().value = val;
    }
    /** add a message to a Player queue
    @param pubTime  the time the message should be sent
    @param src the source endpoint of the message
    @param dest the destination endpoint of the message
    @param payload the payload of the message
    */
    void addMessage (Time sendTime, const std::string &src, const std::string &dest, const std::string &payload);

    /** add an event for a specific time to a Player queue
    @param sendTime  the time the message should be sent
    @param actionTime  the eventTime listed for the message
    @param src the source endpoint of the message
    @param dest the destination endpoint of the message
    @param payload the payload of the message
    */
    void addMessage (Time sendTime,
                     Time actionTime,
                     const std::string &src,
                     const std::string &dest,
                     const std::string &payload);

    /** get the number of points loaded*/
    auto pointCount () const { return points.size (); }
    /** get the number of messages loaded*/
    auto messageCount () const { return messages.size (); }
    /** get the number of publications */
    auto publicationCount () const { return publications.size (); }
    /** get the number of endpoints*/
    auto endpointCount () const { return endpoints.size (); }
    /** finalize the Player federate*/
    void finalize ();

    /** check if the Player is ready to run*/
    bool isActive () const { return !deactivated; }

  private:
    int loadArguments (boost::program_options::variables_map &vm_map);
    /** load from a jsonString
    @param either a JSON filename or a string containing JSON
    */
    void loadJsonFile (const std::string &jsonString);
    /** load a text file*/
    void loadTextFile (const std::string &textFile);
    /** helper function to sort through the tags*/
    void sortTags ();
    /** helper function to generate the publications*/
    void generatePublications ();
    /** helper function to generate the used Endpoints*/
    void generateEndpoints ();
    /** helper function to sort the points and link them to publications*/
    void cleanUpPointList ();
    /** function to decode data strings for messages*/
    std::string decode (std::string &&stringToDecode);
    /** send all points and messages up to the specified time*/
    void sendInformation (Time sendTime);

    /** extract a time from the string based on Player parameters
    @param str the string containing the time
    @param lineNumber the lineNumber of the file which is used in case of invalid specification
    */
    helics::Time extractTime (const std::string &str, int lineNumber = 0) const;

  private:
    std::shared_ptr<CombinationFederate> fed;  //!< the federate created for the Player
    std::vector<ValueSetter> points;  //!< the points to generate into the federation
    std::vector<MessageHolder> messages;  //!< list of message to hold
    std::map<std::string, std::string> tags;  //!< map of the key and type strings
    std::set<std::string> epts;  //!< set of the used endpoints
    std::vector<Publication> publications;  //!< the actual publication objects
    std::vector<Endpoint> endpoints;  //!< the actual endpoint objects
    std::map<std::string, int> pubids;  //!< publication id map
    std::map<std::string, int> eptids;  //!< endpoint id maps
    helics::helics_type_t defType =
      helics::helics_type_t::helicsString;  //!< the default data type unless otherwise specified
    Time stopTime = Time::maxVal ();  //!< the time the Player should stop
    std::string masterFileName;  //!< the name of the master file used to do the construction
    size_t pointIndex = 0;  //!< the current point index
    size_t messageIndex = 0;  //!< the current message index
    timeUnits units = timeUnits::sec;
    double timeMultiplier = 1.0;  //!< specify the time multiplier for different time specifications
    bool useLocal = false;
    bool fileLoaded = false;
    bool deactivated = false;
};
}  // namespace apps
} // namespace helics

