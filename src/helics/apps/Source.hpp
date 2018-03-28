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
    Time period;
    Time nextTime;
    int generatorIndex=-1;
    std::string generatorName;
    // source type
};

class SignalGenerator
{
protected:
    Time lastTime = timeZero;
    Time keyTime = timeZero;
public:
    SignalGenerator() = default;
    virtual ~SignalGenerator() = default;
    /** set a numerical parameter*/
    virtual void set(const std::string &parameter, double val);
    /** set a string parameter*/
    virtual void setString(const std::string &parameter, const std::string &val);
    /** generate a new value at time signalTime
    @return a value and a defV object*/
    virtual defV generate(Time signalTime) = 0;
    /** set the key time*/
    void setTime(Time indexTime) { keyTime = indexTime; }
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
    @details connect all sources with a generator
    */
    void initialize();
    /*run the source*/
    void run ();

    /** run the source until the specified time
    @param stopTime_input the desired stop time
    */
    void run (Time stopTime_input);

    /** add a publication to a source
    @param key the key of the publication to add
    @param generator the name of the generator to link with
    @param type the type of the publication
    @param period the period of the publication
    @param units the units associated with the publication
    */
    void addPublication (const std::string &key,const std::string &generator, helics_type_t type, Time period, const std::string &units = std::string());

    /** add a publication to a source
    @param key the key of the publication to add
    @param type the type of the publication
    @param period the period of the publication
    @param units the units associated with the publication
    */
    void addPublication(const std::string &key, helics_type_t type, Time period, const std::string &units = std::string())
    {
        addPublication(key, std::string(), type, period, units);
    }
    /** add a signal generator to the source object
    @return an index for later reference of the signal generator
    */
    int addSignalGenerator(const std::string &name, const std::string &type);
    /** set the start time for a publication */
    void setStartTime(const std::string &key, Time startTime);
    /** set the start time for a publication */
    void setPeriod(const std::string &key, Time period);
    /** tie a publication to a signal generator*/
    void linkPublicationToGenerator(const std::string &key, const std::string &generator);
    /** tie a publication to a signal generator*/
    void linkPublicationToGenerator(const std::string &key, int genIndex);
    /** get a pointer to the signal generator*/
    std::shared_ptr<SignalGenerator> getGenerator(int index);

    /** finalize the Source federate*/
    void finalize();
  private:
    int loadArguments (boost::program_options::variables_map &vm_map);
    /** load from a jsonString
    @param either a json filename or a string containing json
    */
    void loadJsonFile (const std::string &jsonString);
    /** execute a source object and update its time return the next execution time*/
    Time runSource(SourceObject &obj, Time currentTime);
    /** execute all the sources*/
    Time runSourceLoop(Time currentTime);
  private:
    std::shared_ptr<CombinationFederate> fed;  //!< the federate created for the source
    std::vector<SourceObject> sources;  //!< the actual publication objects
    std::vector<std::shared_ptr<SignalGenerator>> generators; //!< the signal generators
    std::map<std::string, int> generatorLookup; //!< map of generator names to indices
    std::vector<Endpoint> endpoints;  //!< the actual endpoint objects
    std::map<std::string, int> pubids;  //!< publication id map
    Time stopTime = Time::maxVal ();  //!< the time the source should stop
    Time defaultPeriod = 1.0; //!< the default period of publication
    bool deactivated = false; //!< indicator that the app is disabled
    bool useLocal = false;
    bool fileLoaded = false;
};
}  // namespace apps
} // namespace helics

