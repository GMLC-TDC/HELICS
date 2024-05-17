/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once
#include "../core/CoreFederateInfo.hpp"
#include "../core/CoreTypes.hpp"
#include "helics_cxx_export.h"

#include <memory>
#include <string>
#include <vector>

namespace CLI {
class App;
}

namespace helics {
class helicsCLI11App;

namespace fileops {
    class JsonBuffer;
}
/** data class defining federate properties and information
 */
class HELICS_CXX_EXPORT FederateInfo: public CoreFederateInfo {
  public:
    int uniqueKey{0};  //!< location for keying the info for application purposes
    char separator{'/'};  //!< separator for global name of localFederates
    bool autobroker{
        false};  //!< specify that the core should generate a broker if not found otherwise
    /** specify that the core/federate should operate in a user debugging
    mode which will turn off some timeouts*/
    bool debugging{false};
    bool observer{false};
    CoreType coreType{CoreType::DEFAULT};  //!< the type of the core
    int brokerPort{-1};  //!< broker port information

    bool forceNewCore{false};  //!< indicator that the federate should not use an existing core
    /** indicate that the federate should use json serialization for all data transfers*/
    bool useJsonSerialization{false};
    /** specify that the core should use encryption*/
    bool encrypted{false};
    /** specify that the federate and associated core should enable profiling to the specified
     * file*/
    std::string profilerFileName;
    std::string encryptionConfig;  //!< encryption configuration string or file
    std::string defName;  //!< a default name to use for a federate
    std::string coreName;  //!< the name of the core
    std::string coreInitString;  //!< an initialization string for the core API object
    std::string brokerInitString;  //!< an initialization string for the broker if auto generated
    std::string broker;  //!< connection information for the broker
    std::string key;  //!< key for the broker
    std::string localport;  //!< string for defining the local port to use usually a number but
                            //!< other strings are possible

    std::string configString;  //!< storage for config file name or string
    bool fileInUse{false};
    /** default constructor*/
    FederateInfo();
    /** construct from a type
    @param cType the type of core to use for the federate*/
    explicit FederateInfo(CoreType cType);
    /** load a federateInfo object from command line arguments in a string
    @details calls /ref loadInfoFromArgs in the constructor
    @param args a string containing the command line arguments
    */
    explicit FederateInfo(const std::string& args);
    /** load a federateInfo object from command line arguments
    @details calls /ref loadInfoFromArgs in the constructor
    @param argc the number of arguments
    @param argv an array of char * pointers to the arguments
    */
    FederateInfo(int argc, char* argv[]);
    /** load a federateInfo object from arguments stored in a vector
    @details calls /ref loadInfoFromArgs in the constructor
    @param[in,out] args a vector of arguments to load.  The unused arguments will be returned in the
    vector
    */
    explicit FederateInfo(std::vector<std::string>& args);
    /** load a federateInfo object from command line arguments outside the constructor
   @param args a string containing the command line arguments
   @return a vector of strings containing the unused arguments
   */
    std::vector<std::string> loadInfoFromArgs(const std::string& args);
    /** load a federateInfo object from command line arguments outside the constructor
    @param argc the number of arguments
    @param argv an array of char * pointers to the arguments
    @return a vector of strings containing the unused arguments
    */
    std::vector<std::string> loadInfoFromArgs(int argc, char* argv[]);
    /** load a federateInfo object from command line arguments contained in a vector
    @param[in,out] args a vector of arguments to load.  The unused arguments will be returned in the
    vector
    */
    void loadInfoFromArgs(std::vector<std::string>& args);
    /** load a federateInfo object from command line arguments outside the constructor
 @param args a string containing the command line arguments
 */
    void loadInfoFromArgsIgnoreOutput(const std::string& args);
    /** load a federateInfo object from command line arguments outside the constructor
    @param argc the number of arguments
    @param argv an array of char * pointers to the arguments
    */
    void loadInfoFromArgsIgnoreOutput(int argc, char* argv[]);

    /** load a federateInfo object from a toml string either a file or toml string
    @param toml a string containing the name of the toml file or toml contents
    */
    void loadInfoFromToml(const std::string& toml, bool runArgParser = true);

    /** load a federateInfo object from a JSON string either a file or JSON string
  @param json a string containing the name of the JSON file or JSON contents
  */
    void loadInfoFromJson(const std::string& json, bool runArgParser = true);

    /** check if a property has been set and return its value*/
    Time checkTimeProperty(int propId, Time defVal) const;
    bool checkFlagProperty(int propId, bool defVal) const;
    int checkIntProperty(int propId, int defVal) const;

    /** inject the federateInfo parser into another CLI11 App*/
    void injectParser(CLI::App* app);

  private:
    std::unique_ptr<helicsCLI11App> makeCLIApp();
    /** do some additional configuration from config files */
    void config_additional(CLI::App* app);
    /** method for loading info from json*/
    void loadJsonConfig(const fileops::JsonBuffer& json);
};

/** generate a FederateInfo object from a config file or string (JSON, TOML)
 */
HELICS_CXX_EXPORT FederateInfo loadFederateInfo(const std::string& configString);

/** generate string for passing arguments to the core*/
HELICS_CXX_EXPORT std::string generateFullCoreInitString(const FederateInfo& fedInfo);

/** get an integer/time property/flag from a string name of the property or flag
@param val a name of property to get an integer index code for used in /ref
CoreFederateInfo::setProperty
@return the integer code for a given property
*/
HELICS_CXX_EXPORT int getPropertyIndex(std::string val);

/** get a flag index from a string name of the flag
@param val a name of a flag option to get an integer index code for use in /ref
CoreFederateInfo::setFlagOption
@return the integer code for a given flag
*/
HELICS_CXX_EXPORT int getFlagIndex(std::string val);

/** get an integer option index for a binary flag option
@param val a name of flag option to get an integer index code for used in /ref
CoreFederateInfo::setOptionFlag
@return the integer code for a given property (-1) if not found
*/
HELICS_CXX_EXPORT int getOptionIndex(std::string val);

/** get a numerical value for a string option value
@param val a value from an enumeration or flag used as part of a value
@return the integer code of a given option value (-1) if not found
*/
HELICS_CXX_EXPORT int getOptionValue(std::string val);

}  // namespace helics
