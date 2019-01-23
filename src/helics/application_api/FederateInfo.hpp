/*
Copyright © 2017-2019,
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
    int uniqueKey = 0;  //!< location for keying the info for application purposes
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
    /** construct from a type
    @param cType the type of core to use for the federate*/
    explicit FederateInfo (core_type cType) : coreType (cType){};
    /** load a federateInfo object from command line arguments
    @details calls /ref loadInfoFromArgs in the constructor
    @param argc the number of arguments
    @param argv an array of char * pointers to the arguments
    */
    FederateInfo (int argc, const char *const *argv);
    /** load a federateInfo object from command line arguments outside the constructor
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

/** get an integer property/flag from a string name of the property or flag
@param val a name of property to get an integer index code for used in /ref CoreFederateInfo::setProperty
@return the integer code for a given property
*/
int getPropertyIndex (std::string val);

/** get an integer option index for a binary flag option
@param val a name of flag option to get an integer index code for used in /ref CoreFederateInfo::setOptionFlag
@return the integer code for a given property
*/
int getOptionIndex (std::string val);

}  // namespace helics
