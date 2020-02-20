/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#ifndef BENCHMARKFEDERATE_H
#define BENCHMARKFEDERATE_H

#include "helics/application_api/CombinationFederate.hpp"
#include "helics/core/ActionMessage.hpp"
#include "helics/helics-config.h"
#include "helics/core/helicsCLI11.hpp"
#include "helics_benchmark_util.h"

#include <iostream>
#include <sstream>
#include <memory>
#include <vector>

using namespace helics;
/** class implementing common functionality for benchmarks */
class BenchmarkFederate {
  public:
    // getters and setters for parameters
    void setDeltaTime(helics::Time dt) { deltaTime = dt; }
    helics::Time getDeltaTime() { return deltaTime; }
    void setFinalTime(helics::Time t) { finalTime = t; }
    helics::Time getFinalTime() { return finalTime; }

    void setIndex(int i) { index = i; }
    int getIndex() { return index; }
    void setMaxIndex(int i) { maxIndex = i; }
    int getMaxIndex() { return maxIndex; }

    // functions for setting callbacks
    void setBeforeFinalizeCallback(std::function<void()> cb = {}) { callBeforeFinalize = cb; }
    void setAfterFinalizeCallback(std::function<void()> cb = {}) { callAfterFinalize = cb; }

    // protected to give derived classes more control
  protected:
    class Result {
        public:
        std::string name;
        std::string key;
        std::string value;
    };
    std::vector<Result> results;

    // parameters most benchmark federates need
    helics::Time deltaTime=helics::Time(10, time_units::ns); // sampling rate
    helics::Time finalTime=helics::Time(10000, time_units::ns); // final time
    int index=0; // the index for an instance of the benchmark federate
    int maxIndex=0; // the maximum index + 1 given to a benchmark federate in a run

    // CLI11 Options for derived classes to change them if needed (e.g. set required)
    CLI::Option *opt_delta_time;
    CLI::Option *opt_final_time;
    CLI::Option *opt_index;
    CLI::Option *opt_max_index;

    // callbacks for more control when timing code
    std::function<void()> callBeforeFinalize = nullptr;
    std::function<void()> callAfterFinalize = nullptr;

    std::unique_ptr<helics::CombinationFederate> fed;

    // Command line options
    std::unique_ptr<helics::helicsCLI11App> app;

    // variables to track current state, mainly for gbenchmark piecewise setup
    bool initialized{false};
    bool readyToRun{false};

    // functions to be overriden by derived benchmark classes
    virtual void setupArgumentParsing() {} // Set/override default base param values before arguments are parsed, add benchmark specific options
    virtual void doParamInit(helics::FederateInfo& fi) { (void) fi; } // Initialization after command line options setup parameters
    virtual void doFedInit() {} // Initialization after the federate object is created (create endpoints, inputs, etc)
    virtual void doMakeReady() {} // Initialization after the federation is set up, but before timing starts
    virtual void doMainLoop() {} // Contains the main loop for the benchmark
    virtual std::string getName() { return ""; } // Returns the federate name

  public:
    BenchmarkFederate() : BenchmarkFederate("") {}

    BenchmarkFederate(std::string name, helics::Time dt, helics::Time ft, int i, int max_i) : BenchmarkFederate(name)
    {
        deltaTime = dt;
        finalTime = ft;
        index = i;
        maxIndex = max_i;
    }
    
    // TODO add JSON output format option    
    BenchmarkFederate(std::string name)
    {
        // Setup up basic CLI11 with options all benchmark feds need to support
        app = std::make_unique<helics::helicsCLI11App>(name);
        app->allow_extras();

        // add common time options (optional)
        opt_delta_time = app->add_option("--delta_time", deltaTime, "the sampling rate/timestep size");
        opt_final_time =
            app->add_option("--final_time", finalTime, "final/stop time for the benchmark");
        opt_delta_time->ignore_underscore();
        opt_final_time->ignore_underscore();

        // add common index options (optional)
        opt_index = app->add_option("--index", index, "the index of this phold federate");
        opt_max_index =
            app->add_option("--max_index", maxIndex, "the maximum index given to a phold federate");
        opt_max_index->ignore_underscore();

        app->add_flag_function("--print_systeminfo", [](int count){ if (count) printHELICSsystemInfo(); }, "prints the HELICS system info");
    }

    void run(std::function<void()> callOnReady = {}, std::function<void()> callOnEnd = {})
    {
        if (!readyToRun) {
            makeReady();
        }
        if (callOnReady) {
            callOnReady();
        }
        execute();
        finalize();
        if (callOnEnd) {
            callOnEnd();
        }
    }


    int initialize(const std::string& coreName, int argc, char** argv)
    {
        helics::FederateInfo fi;
        fi.coreName = coreName;
        return initialize(fi, argc, argv);
    }

    int initialize(const helics::FederateInfo fi, int argc, char** argv)
    {
        setupArgumentParsing();
        return internalInitialize(fi, parseArgs(argc, argv));
    }

    int initialize(const std::string& coreName, std::string bmInit = "")
    {
        helics::FederateInfo fi;
        fi.coreName = coreName;
        return initialize(fi, bmInit);
    }

    int initialize(const helics::FederateInfo fi, std::string initstr = "")
    {
        setupArgumentParsing();
        return internalInitialize(fi, parseArgs(initstr));
    }

    void makeReady()
    {
        if (!initialized) {
            throw("must initialize first");
        }
        fed->enterExecutingMode();
        doMakeReady();
        readyToRun = true;
    }

    void printResults()
    {
        for (auto r : results) {
            std::cout << r.name << ": " << r.value << std::endl;
        }
    }

   
    template<class T>
    void addResult(std::string name, std::string key, T value)
    {
        std::ostringstream s;
        s << value;
        addResult(name, key, s.str());
    }

    void addResult(std::string name, std::string key, std::string value)
    {
        Result r;
        r.name = name;
        r.key = key;
        r.value = value;
        results.push_back(r);
    }

  private:
    void finalize()
    {
        if (callBeforeFinalize) {
            callBeforeFinalize();
        }
        fed->finalize();
        if (callAfterFinalize) {
            callAfterFinalize();
        }
    }

    void execute()
    {
        doMainLoop();
    }


    int internalInitialize(helics::FederateInfo fi, int parseResult)
    {
        if (parseResult != 0) {
            initialized = false;
            return parseResult;
        }
        fi.loadInfoFromArgs(app->remainArgs());

        doParamInit(fi);
        std::string name = getName();
        fed = std::make_unique<helics::CombinationFederate>(name, fi);
        doFedInit();
        initialized = true;
        return 0;
    }

    /*int parseArgs(int argc, char** argv)
    {
        auto res = app->helics_parse(argc, argv);
        return handleCLI11Result(res);
    }

    int parseArgs(std::string initstr)
    {
        auto res = app->helics_parse(initstr);
        return handleCLI11Result(res);
    }*/


    template<typename ... Args>
    int parseArgs(Args ... args)
    //int handleCLI11Result(helics::helicsCLI11App::parse_output res)
    {
        auto res = app->helics_parse(args ...);

        helics::FederateInfo fi;
        if (res != helics::helicsCLI11App::parse_output::ok) {
            switch (res) {
                case helics::helicsCLI11App::parse_output::help_call:
                case helics::helicsCLI11App::parse_output::help_all_call:
                    fi.loadInfoFromArgs("--help");
                    // FALLTHRU
                case helics::helicsCLI11App::parse_output::version_call:
                default:
                    // Nothing to do, send result higher up to decide what should happen
                    break;
            }
        }
        return static_cast<int>(res);
    }
};

#endif // BENCHMARKFEDERATE_H
