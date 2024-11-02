/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "../application_api/CombinationFederate.hpp"

#include <fstream>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace CLI {
class App;
}  // namespace CLI

namespace helics::fileops {
class JsonBuffer;
}
namespace helics::apps {

class AppTextParser;

/** class defining a basic helics App
@details  the App class is not thread-safe in non-const methods,  don't try to use it from multiple
threads without external protection, that will result in undefined behavior
*/
class HELICS_CXX_EXPORT App {
  public:
    /** default constructor*/
    App() = default;
    /** construct from command line arguments in a vector
@param defaultAppName the name to use if not specified in one of the arguments
@param args the command line arguments to pass in a reverse vector
*/
    App(std::string_view defaultAppName, std::vector<std::string> args);
    /** construct from command line arguments
@param defaultAppName the name to use if not specified in one of the arguments
@param argc the number of arguments
@param argv the strings in the input
*/
    App(std::string_view defaultAppName, int argc, char* argv[]);
    /** construct from a federate info object
@param appName the name of the application, can be left empty to use a name specified in fedInfo
@param fedInfo a pointer info object containing information on the desired federate
configuration
*/
    App(std::string_view appName, const FederateInfo& fedInfo);
    /**constructor taking a federate information structure and using the given core
@param appName the name of the application, can be left empty to use a name specified in fedInfo
@param core a pointer to core object which the federate can join
@param fedInfo  a federate information structure
*/
    App(std::string_view appName, const std::shared_ptr<Core>& core, const FederateInfo& fedInfo);
    /**constructor taking a federate information structure and using the given coreApp
@param appName the name of the application, can be left empty to use a name specified in fedInfo
@param core a pointer to core object which the federate can join
@param fedInfo  a federate information structure
*/
    App(std::string_view appName, CoreApp& core, const FederateInfo& fedInfo);
    /**constructor taking a file with the required information
@param appName the name of the application, can be left empty to use a name specified in
configString
@param configString file or JSON string defining the federate information and other
configuration
*/
    App(std::string_view appName, const std::string& configString);

    /** move construction*/
    App(App&& other_app) = default;
    /** don't allow the copy constructor*/
    App(const App& other_app) = delete;
    /** move assignment*/
    App& operator=(App&& app) = default;
    /** don't allow the copy assignment,  the default would fail anyway since federates are not
     * copyable either*/
    App& operator=(const App& app) = delete;
    virtual ~App();

    /** load a file containing interface information
@param filename the file containing the configuration and App data  accepted format are JSON,
and a App format which is tab delimited or comma delimited toml file can be used load interfaces
but not app configuration
@param enableFederateInterfaceRegistration default true, if set to false will not load federate
information*/
    void loadFile(const std::string& filename, bool enableFederateInterfaceRegistration = true);
    /** initialize the App federate
@details generate all the publications and organize the points, the final publication count will
be available after this time and the App will enter the initialization mode, which means it
will not be possible to add more publications calling run will automatically do this if
necessary
*/
    virtual void initialize();
    /*run the Player*/
    virtual void run();

    /** run the Player until the specified time
@param stopTime_input the desired stop time
*/
    virtual void runTo(Time stopTime_input) = 0;

    /** finalize the Player federate*/
    virtual void finalize();

    /** check if the Player is ready to run*/
    bool isActive() const { return !deactivated; }
    /** get a const reference to the federate*/
    const CombinationFederate& accessUnderlyingFederate() const { return *fed; }

    /** get a copy of the federate pointer (this can be dangerous if misused) */
    std::shared_ptr<CombinationFederate> getUnderlyingFederatePointer() { return fed; }

  protected:
    /** load from a jsonString
@param jsonString either a JSON filename or a string containing JSON
*/
    virtual void loadJsonFile(const std::string& jsonString,
                              bool enableFederateInterfaceRegistration);
    /** load from a jsonString and check a field named appName for configuration options
@param appName the name of the app which may be used in section of the JSON for some local
configuration
@param jsonString either a JSON filename or a string containing JSON
*/
    void loadJsonFileConfiguration(const std::string& appName,
                                   const std::string& jsonString,
                                   bool enableFederateInterfaceRegistration);
    /** load a text file*/
    virtual void loadTextFile(const std::string& textFile);
    /** actively load the specified files from the configuration*/
    void loadInputFiles();
    /** load the config options from a text parser*/
    void loadConfigOptions(AppTextParser& aparser);

  private:
    void loadConfigOptions(const fileops::JsonBuffer& element);
    /** generate the command line parser*/
    std::unique_ptr<helicsCLI11App> generateParser();
    /** process the command line arguments */
    void processArgs(std::unique_ptr<helicsCLI11App>& app,
                     FederateInfo& fedInfo,
                     std::string_view defaultAppName);

  protected:
    std::shared_ptr<CombinationFederate> fed;  //!< the federate created for the App
    Time stopTime = Time::maxVal();  //!< the time the App should stop
    std::string configFileName;  //!< name of the config file used for constructing the federate
    std::string inputFileName;  //!< the name of the app input file
    bool useLocal{false};
    bool fileLoaded{false};
    bool deactivated{false};
    bool quietMode{false};
    bool helpMode{false};
    std::vector<std::string> remArgs;
};

class AppTextParser {
  public:
    explicit AppTextParser(const std::string& filename);
    /** run a preparse on the counting lines and extracting command strings*/
    std::vector<int> preParseFile(const std::vector<char>& klines);

    const std::string& configString() const { return configStr; }
    /** load the next meaningful line and its linenumber*/
    bool loadNextLine(std::string& line, int& lineNumber);
    void reset();

  private:
    bool mLineComment{false};
    std::ifstream filePtr;
    std::string configStr;
    std::string mFileName;
    int currentLineNumber{0};
};
}  // namespace helics::apps
