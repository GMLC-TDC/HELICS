/*
Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.
*/

#pragma once
#include "../application_api/CombinationFederate.hpp"
#include "../application_api/Endpoints.hpp"
#include "../application_api/Publications.hpp"

#include "../application_api/HelicsPrimaryTypes.hpp"
#include <map>

#include "PrecHelper.hpp"
#include <set>

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
class SourceObject
{
  public:
    Publication pub;
    // source type
};

/** class implementing a source federate, which is capable of generating signals of various kinds
and sending signals at the appropriate times
@details  the source class is not threadsafe,  don't try to use it from multiple threads without external
protection, that will result in undefined behavior
*/
class Source
{
  public:
    /** default constructor*/
    Source () = default;
    /** construct from command line arguments
    @param argc the number of arguments
    @param argv the strings in the input
    */
    Source (int argc, char *argv[]);
    /** construct from a federate info object
    @param fi a pointer info object containing information on the desired federate configuration
    */
    explicit Source (const FederateInfo &fi);
    /**constructor taking a federate information structure and using the given core
    @param core a pointer to core object which the federate can join
    @param[in] fi  a federate information structure
    */
    Source (const std::shared_ptr<Core> &core, const FederateInfo &fi);
    /**constructor taking a file with the required information
    @param[in] jsonString file or JSON string defining the federate information and other configuration
    */
    explicit Source (const std::string &jsonString);

    /** move construction*/
    Source (Source &&other_source) = default;
    /** don't allow the copy constructor*/
    Source (const Source &other_source) = delete;
    /** move assignment*/
    Source &operator= (Source &&fed) = default;
    /** don't allow the copy assignment,  the default would fail anyway since federates are not copyable either*/
    Source &operator= (const Source &fed) = delete;
    ~Source ();

    /** load a file containing publication information
    @param filename the file containing the configuration and source data  accepted format are json, xml, and a
    source format which is tab delimited or comma delimited*/
    void loadFile (const std::string &filename);
    /** initialize the source federate
    @details generate all the publications and organize the points, the final publication count will be available
    after this time and the source will enter the initialization mode, which means it will not be possible to add
    more publications calling run will automatically do this if necessary
    */
    void initialize ();
    /*run the source*/
    void run ();

    /** run the source until the specified time
    @param stopTime_input the desired stop time
    */
    void run (Time stopTime_input);

    /** add a publication to a source
    @param key the key of the publication to add
    @param type the type of the publication
    @param units the units associated with the publication
    */
    void addSource (const std::string &key, helics_type_t type, const std::string &units = "");

  private:
    int loadArguments (boost::program_options::variables_map &vm_map);
    /** load from a jsonString
    @param either a json filename or a string containing json
    */
    void loadJsonFile (const std::string &jsonString);

  private:
    std::shared_ptr<CombinationFederate> fed;  //!< the federate created for the source
    std::vector<SourceObject> sources;  //!< the actual publication objects
    Time stopTime = Time::maxVal ();  //!< the time the source should stop
    bool deactivated = false;
};
}  // namespace apps
} // namespace helics

