/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once
#include "../application_api/Endpoints.hpp"
#include "../application_api/HelicsPrimaryTypes.hpp"
#include "../application_api/Publications.hpp"
#include "helicsApp.hpp"

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

namespace helics {
namespace apps {
    /** helper class for containing a signal generator source*/
    struct SourceObject {
        Publication pub;
        Time period;
        Time nextTime{timeZero};
        int generatorIndex{-1};
        std::string generatorName;
        SourceObject() = default;
        SourceObject(const Publication& p, Time per): pub(p), period(per) {}
        // source type
    };

    /** parent class for a signal generator which generates values to feed into a helics
     * federation*/
    class HELICS_CXX_EXPORT SignalGenerator {
      protected:
        Time lastTime{timeZero};
        Time keyTime{timeZero};

      public:
        SignalGenerator() = default;
        virtual ~SignalGenerator() = default;
        /** set a numerical parameter*/
        virtual void set(const std::string& parameter, double val);
        /** set a string parameter*/
        virtual void setString(const std::string& parameter, const std::string& val);
        /** generate a new value at time signalTime
    @return a value and a defV object*/
        virtual defV generate(Time signalTime) = 0;
        /** set the key time*/
        void setTime(Time indexTime) { keyTime = indexTime; }
    };

    /** class implementing a source federate, which is capable of generating signals of various
kinds and sending signals at the appropriate times
@details  the source class is NOT threadsafe,  don't try to use it from multiple threads without
external protection, that will result in undefined behavior
*/
    class HELICS_CXX_EXPORT Source: public App {
      public:
        /** default constructor*/
        Source() = default;
        /** construct from command line arguments in a vector
    @param args The vector of string, the remaining arguments are returned in the args
    */
        explicit Source(std::vector<std::string> args);
        /** construct from command line arguments
    @param argc the number of arguments
    @param argv the strings in the input
    */
        Source(int argc, char* argv[]);
        /** construct from a federate info object
    @param name the name of the source object (can be empty to use defaults from fi)
    @param fi a pointer info object containing information on the desired federate configuration
    */
        Source(const std::string& name, const FederateInfo& fi);
        /**constructor taking a federate information structure and using the given core
    @param name the name of the source object (can be empty to use defaults from fi)
    @param core a pointer to core object which the federate can join
    @param fi  a federate information structure
    */
        Source(const std::string& name, const std::shared_ptr<Core>& core, const FederateInfo& fi);

        /**constructor taking a federate information structure and using the given core
    @param name the name of the federate (can be empty to use defaults from fi)
    @param core a coreApp object that can be joined
    @param fi  a federate information structure
    */
        Source(const std::string& name, CoreApp& core, const FederateInfo& fi);
        /**constructor taking a file with the required information
    @param name the name of the source object (can be empty to use defaults from fi)
    @param configString file a JSON or TOML file or string containing configuration informatino
    */
        Source(const std::string& name, const std::string& configString);

        /** move construction*/
        Source(Source&& other_source) = default;

        /** move assignment*/
        Source& operator=(Source&& fed) = default;

        /** initialize the source federate
    @details connect all sources with a generator
    */
        virtual void initialize() override;

        /** run the source until the specified time
    @param stopTime_input the desired stop time
    */
        virtual void runTo(Time stopTime_input) override;

        /** add a publication to a source
    @param key the key of the publication to add
    @param generator the name of the generator to link with
    @param type the type of the publication
    @param period the period of the publication
    @param units the units associated with the publication
    */
        void addPublication(const std::string& key,
                            const std::string& generator,
                            data_type type,
                            Time period,
                            const std::string& units = std::string());

        /** add a publication to a source
    @param key the key of the publication to add
    @param type the type of the publication
    @param period the period of the publication
    @param units the units associated with the publication
    */
        void addPublication(const std::string& key,
                            data_type type,
                            Time period,
                            const std::string& units = std::string())
        {
            addPublication(key, std::string(), type, period, units);
        }
        /** add a signal generator to the source object
    @return an index for later reference of the signal generator
    */
        int addSignalGenerator(const std::string& name, const std::string& type);
        /** set the start time for a publication */
        void setStartTime(const std::string& key, Time startTime);
        /** set the start time for a publication */
        void setPeriod(const std::string& key, Time period);
        /** tie a publication to a signal generator*/
        void linkPublicationToGenerator(const std::string& key, const std::string& generator);
        /** tie a publication to a signal generator*/
        void linkPublicationToGenerator(const std::string& key, int genIndex);
        /** get a pointer to the signal generator*/
        std::shared_ptr<SignalGenerator> getGenerator(int index);

      private:
        /** process remaining command line arguments*/
        void processArgs();
        /** load from a jsonString
    @param jsonString either a JSON filename or a string containing JSON
    */
        virtual void loadJsonFile(const std::string& jsonString) override;
        /** execute a source object and update its time return the next execution time*/
        Time runSource(SourceObject& obj, Time currentTime);
        /** execute all the sources*/
        Time runSourceLoop(Time currentTime);

      private:
        std::vector<SourceObject> sources;  //!< the actual publication objects
        std::vector<std::shared_ptr<SignalGenerator>> generators;  //!< the signal generators
        std::map<std::string, int> generatorLookup;  //!< map of generator names to indices
        std::vector<Endpoint> endpoints;  //!< the actual endpoint objects
        std::map<std::string, int> pubids;  //!< publication id map
        Time defaultPeriod = 1.0;  //!< the default period of publication
    };
}  // namespace apps
}  // namespace helics
