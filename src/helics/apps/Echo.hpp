/*
Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.
*/
#pragma once

#include "../application_api/Endpoints.hpp"
#include "../application_api/MessageFederate.hpp"

#include <mutex>
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
/** class implementing a Echo object, which will generate endpoint interfaces and send a data message back to the
source at the with a specified delay
@details  the Echo class is not threadsafe in general,  don't try to use it from multiple threads without external protection,
that will result in undefined behavior.  setEchoDelay is threadsafe
*/
class Echo
{
  public:
    /** default constructor*/
    Echo () = default;
    /** construct from command line arguments
    @param argc the number of arguments
    @param argv the strings in the input
    */
    Echo (int argc, char *argv[]);
    /** construct from a federate info object
    @param fi a pointer info object containing information on the desired federate configuration
    */
    explicit Echo (const FederateInfo &fi);
    /**constructor taking a federate information structure and using the given core
    @param core a pointer to core object which the federate can join
    @param[in] fi  a federate information structure
    */
    Echo (const std::shared_ptr<Core> &core, const FederateInfo &fi);
    /**constructor taking a file with the required information
    @param[in] jsonString file or json string defining the federate information and other configuration
    */
    explicit Echo (const std::string &jsonString);

    /** move construction*/
    Echo (Echo &&other_echo) = default;
    /** don't allow the copy constructor*/
    Echo (const Echo &other_echo) = delete;
    /** move assignment*/
    Echo &operator= (Echo &&fed) = default;
    /** don't allow the copy assignment,  the default would fail anyway since federates are not copyable either*/
    Echo &operator= (const Echo &fed) = delete;
    ~Echo ();

    /** load a file containing publication information
    @param filename the file containing the configuration and Player data  accepted format are json, xml, and a
    Player format which is tab delimited or comma delimited*/
    void loadFile (const std::string &filename);

    /** initialize the echo federate
    @details generate all the publications and organize the points, the final publication count will be available
    after this time and the Player will enter the initialization mode, which means it will not be possible to add
    more publications calling run will automatically do this if necessary
    */
    void initialize ();
    /*run the Echo federate*/
    void run ();

    /** run the Echo federate until the specified time
    @param stopTime_input the desired stop time
    */
    void run (Time stopTime_input);

    /** add an endpoint to the Player
    @param endpointName the name of the endpoint
    @param endpointType the named type of the endpoint
    */
    void addEndpoint (const std::string &endpointName, const std::string &endpointType = "");

    /** get the number of points loaded*/
    auto echoCount () const { return echoCounter; }
    /** set the delay time
     */
    void setEchoDelay (Time delay);
    /** finalize the Player federate*/
    void finalize ();
    /** get the number of endpoints*/
    auto endpointCount () const { return endpoints.size (); }
    /** check if the Player is ready to run*/
    bool isActive () const { return !deactivated; }

  private:
    /** load information from a program options variable map*/
    int loadArguments (boost::program_options::variables_map &vm_map);
    /** load information from a JSON file*/
    void loadJsonFile (const std::string &filename);
    /** echo an actual message from an endpoint*/
    void echoMessage(const Endpoint *ept, Time currentTime);
  private:
    std::shared_ptr<MessageFederate> fed;  //!< the federate created for the Player
    std::vector<Endpoint> endpoints;  //!< the actual endpoint objects
    Time delayTime = timeZero;  //!< respond to each message with the specified delay
    Time stopTime = Time::maxVal ();  //!< the time the Player should stop
    size_t echoCounter = 0;  //!< the current message index
    bool deactivated = false; //!< set to true if the Echo app has been deactivated by arguments (help, version)
    bool fileLoaded = false; //!< indicator that a file has been loaded already
    std::mutex delayTimeLock; //mutex protecting delayTime
};
}  // namespace apps
} // namespace helics

