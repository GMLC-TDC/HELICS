/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once

#include "../application_api/CombinationFederate.hpp"
#include "json/json-forwards.h"
namespace boost
{
namespace program_options
{
class variables_map;
} //namespace program_options
} //namespace boost

namespace helics
{
namespace apps
{

/** class defining a basic helics App
@details  the App class is not thread-safe in non-const methods,  don't try to use it from multiple threads without external
protection, that will result in undefined behavior
*/
class App
{
  public:
    /** default constructor*/
    App () = default;
    /** construct from command line arguments
    @param argc the number of arguments
    @param argv the strings in the input
    */
    App (const std::string &appName, int argc, char *argv[]);
    /** construct from a federate info object
    @param fi a pointer info object containing information on the desired federate configuration
    */
    explicit App (const FederateInfo &fi);
    /**constructor taking a federate information structure and using the given core
    @param core a pointer to core object which the federate can join
    @param[in] fi  a federate information structure
    */
    App(const std::shared_ptr<Core> &core, const FederateInfo &fi);
    /**constructor taking a file with the required information
    @param[in] jsonString file or JSON string defining the federate information and other configuration
    */
    App (const std::string &appName, const std::string &jsonString);

    /** move construction*/
    App (App &&other_app) = default;
    /** don't allow the copy constructor*/
    App (const App &other_app) = delete;
    /** move assignment*/
    App &operator= (App &&fed) = default;
    /** don't allow the copy assignment,  the default would fail anyway since federates are not copyable either*/
    App &operator= (const App &fed) = delete;
    virtual ~App ();

    /** load a file containing publication information
    @param filename the file containing the configuration and Player data  accepted format are JSON, xml, and a
    Player format which is tab delimited or comma delimited*/
    void loadFile (const std::string &filename);
    /** initialize the Player federate
    @details generate all the publications and organize the points, the final publication count will be available
    after this time and the Player will enter the initialization mode, which means it will not be possible to add
    more publications calling run will automatically do this if necessary
    */
    virtual void initialize ();
    /*run the Player*/
    virtual void run ();

    /** run the Player until the specified time
    @param stopTime_input the desired stop time
    */
    virtual void runTo (Time stopTime_input)=0;

    /** finalize the Player federate*/
    virtual void finalize ();

    /** check if the Player is ready to run*/
    bool isActive () const { return !deactivated; }

  protected:
    int loadArguments (boost::program_options::variables_map &vm_map);
    /** load from a jsonString
    @param jsonString either a JSON filename or a string containing JSON
    */
    virtual void loadJsonFile (const std::string &jsonString);
    /** load from a jsonString and check a field named appName for configuration options
    @param jsonString either a JSON filename or a string containing JSON
    */
    void loadJsonFileConfiguration(const std::string &appName, const std::string &jsonString);
    /** load a text file*/
    virtual void loadTextFile (const std::string &textFile);
private:
    void loadConfigOptions(const Json_helics::Value &element);
  protected:
    std::shared_ptr<CombinationFederate> fed;  //!< the federate created for the Player
    Time stopTime = Time::maxVal ();  //!< the time the Player should stop
    std::string masterFileName;  //!< the name of the master file used to do the construction
    bool useLocal = false;
    bool fileLoaded = false;
    bool deactivated = false;
    bool quietMode = false;
};
}  // namespace apps
} // namespace helics

