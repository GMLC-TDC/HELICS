/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once
#include "../core/CoreFederateInfo.hpp"

namespace helics
{
/** data class defining federate properties and information
 */
class FederateInfo : public CoreFederateInfo
{
  public:
    char separator = '/';  //!< separator for global name of localFederates
    bool autobroker = false;  //!< specify that the core should generate a broker if not found otherwise
    core_type coreType = core_type::ZMQ;  //!< the type of the core
    int brokerPort = -1;  //!< broker port information

    std::string defName;  //!< a default name to use for a federate
    std::string coreName;  //!< the name of the core
    std::string coreInitString;  //!< an initialization string for the core API object
    std::string broker;  //!< connection information for the broker
    std::string
      localport;  //!< string for defining the local port to use usually a number but other strings are possible

    /** default constructor*/
    FederateInfo () = default;
    /** construct from the name and type*/
    FederateInfo (core_type cType) : coreType (cType){};
    /** load a federateInfo object from command line arguments
    @param argc the number of arguments
    @param argv an array of char * pointers to the arguments
    */
    FederateInfo (int argc, const char *const *argv);
    /** load a federateInfo object from command line arguments
    @param argc the number of arguments
    @param argv an array of char * pointers to the arguments
    */
    void loadInfoFromArgs (int argc, const char *const *argv);
};

/** generate a FederateInfo object from a config file (JSON, TOML)
 */
FederateInfo loadFederateInfo (const std::string &configString);

/** generate string for passing arguments to the core*/
std::string generateFullCoreInitString (const FederateInfo &fi);

/** get an integer property/flag from a string name of the property or flag*/
int getPropertyIndex (std::string val);

}  // namespace helics
