/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "helics/application_api/CombinationFederate.hpp"
#include "helics/core/ActionMessage.hpp"
#include "helics/core/helicsCLI11.hpp"
#include "helics/helics-config.h"
#include "helics_benchmark_util.h"

#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

/** class implementing common functionality for benchmarks */
class BenchmarkFederate {
  public:
    enum class OutputFormat {
        PLAIN_TEXT,
        //          JSON
    };

    // getters and setters for parameters
    /** sets the delta time parameter
     * @param dt the delta time to set
     */
    void setDeltaTime(helics::Time dt) { deltaTime = dt; }
    /** gets the delta time parameter*/
    helics::Time getDeltaTime() { return deltaTime; }
    /** sets the final time parameter
     * @param t the final/stop time*/
    void setFinalTime(helics::Time t) { finalTime = t; }
    /** gets the final time parameter*/
    helics::Time getFinalTime() { return finalTime; }

    /** sets the index parameter
     * @param i the index
     */
    void setIndex(int i) { index = i; }
    /** gets the index parameter*/
    int getIndex() const { return index; }
    /** sets the max index parameter
     * @param i the max index
     */
    void setMaxIndex(int i) { maxIndex = i; }
    /** gets the max index parameter*/
    int getMaxIndex() const { return maxIndex; }

    // functions for setting callbacks
    /** sets a callback function to call immediately after doMainLoop() returns, but before
     * the helics finalize() call
     * @param cb a function that takes no arguments and returns void
     */
    void setBeforeFinalizeCallback(std::function<void()> cb = {})
    {
        callBeforeFinalize = std::move(cb);
    }
    /** sets a callback function to all after the helics finalize() call completes
     * @param cb a function that takes no arguments and returns void
     */
    void setAfterFinalizeCallback(std::function<void()> cb = {})
    {
        callAfterFinalize = std::move(cb);
    }

    /** sets the output format to use when printing results
     * @param f format to print results in
     */
    void setOutputFormat(OutputFormat f) { result_format = f; }

    // protected to give derived classes more control
  protected:
    class Result {
      public:
        std::string name;
        std::string key;
        std::string value;
    };
    std::vector<Result> results;  //!< a vector of output results to print

    OutputFormat result_format{OutputFormat::PLAIN_TEXT};  //!< output format for printing results

    // variables to track current state, mainly for gbenchmark piecewise setup
    bool initialized{false};
    bool readyToRun{false};

    // parameters most benchmark federates need
    std::string benchmarkName;  //<! the name of the benchmark federate
    int index{0};  //<! the index for an instance of the benchmark federate
    int maxIndex{0};  //<! the maximum index + 1 given to a benchmark federate in a run
    helics::Time deltaTime{helics::Time(10, time_units::ns)};  //<! sampling rate
    helics::Time finalTime{helics::Time(10000, time_units::ns)};  //<! final time

    std::unique_ptr<helics::CombinationFederate>
        fed;  //<! the federate object to use in derived classes

    // callbacks for more control when timing code
    std::function<void()> callBeforeFinalize{
        nullptr};  //<! callback function immediately before helics finalize()
    std::function<void()> callAfterFinalize{
        nullptr};  //<! callback function immediately after helics finalize()

    // CLI11 Options for derived classes to change them if needed (e.g. set required)
    CLI::Option* opt_delta_time{nullptr};  //<! the CLI11 option for --delta_time
    CLI::Option* opt_final_time{nullptr};  //<! the CLI11 option for --final_time
    CLI::Option* opt_index{nullptr};  //<! the CLI11 option for --index
    CLI::Option* opt_max_index{nullptr};  //<! the CLI11 option for --max_index

    // Command line options
    std::unique_ptr<helics::helicsCLI11App>
        app;  //<! the CLI11 app object to use in derived classes

    // functions to be overridden by derived benchmark classes
    /** set/override default base parameter values before arguments are parsed, and modify CLI11
     * options for default arguments*/
    virtual void setupArgumentParsing() {}
    /** initialization steps after command line options have been parsed, but before the federate
     * object is created
     * @param fi a reference to a helics::FederateInfo object that can be used to change how the
     * federate object is made
     */
    virtual void doParamInit(helics::FederateInfo& fi) { (void)fi; }
    /** initialization that needs to happen after the federate object is created, such as creating
     * endpoints and inputs*/
    virtual void doFedInit() {}
    /** initialization that requires the federation to be set up, but before timing starts, like
     * creating initial events*/
    virtual void doMakeReady() {}
    /** the main loop for the benchmark*/
    virtual void doMainLoop() {}
    /** returns the federate name (should be unique within the federation*/
    virtual std::string getName() { return ""; }
    /** allows a federate to add custom results to get printed when running in standalone mode*/
    virtual void doAddBenchmarkResults() {}

  public:
    BenchmarkFederate(): BenchmarkFederate("") {}

    /** constructor taking a name for the benchmark app
     * @param name the name of the benchmark federate, shown by CLI11 --help option
     */
    explicit BenchmarkFederate(const std::string& name):
        benchmarkName(name),
        app(std::make_unique<helics::helicsCLI11App>(name + " Benchmark Federate"))
    {
        addResult("BENCHMARK FEDERATE TYPE", "benchmark_federate_type", name);
        app->allow_extras();

        // add common time options (optional)
        opt_delta_time =
            app->add_option("--delta_time", deltaTime, "the sampling rate/timestep size");
        opt_final_time =
            app->add_option("--final_time", finalTime, "final/stop time for the benchmark");
        opt_delta_time->ignore_underscore();
        opt_final_time->ignore_underscore();

        // add common index options (optional)
        opt_index = app->add_option("--index", index, "the index of this phold federate");
        opt_max_index =
            app->add_option("--max_index", maxIndex, "the maximum index given to a phold federate");
        opt_max_index->ignore_underscore();

        // add a flag for printing system info
        app->add_flag_callback(
            "--print_systeminfo",
            []() { printHELICSsystemInfo(); },
            "prints the HELICS system info");
    }

    /** starts execution of the federation
     * @param callOnReady a no argument, void return type function called after doMakeReady is run
     * @param callOnEnd a no argument, void return type function called after helics finalize()
     */
    void run(const std::function<void()>& callOnReady = {},
             const std::function<void()>& callOnEnd = {})
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

    /** initialize function parses options and sets up parameters
     * @param coreName the name of the core to connect to
     * @param args command line argument format supported by CLI11 (argc/argv, string, or vector of
     * strings)
     * @return 0 on success, non-zero indicates failure
     */
    template<typename... Args>
    int initialize(const std::string& coreName, Args... args)
    {
        helics::FederateInfo fi;
        fi.coreName = coreName;
        return initialize(fi, args...);
    }

    /** initialize function parses options and sets up parameters
     * @param fi a helics::FederateInfo object
     * @param args command line argument format supported by CLI11 (argc/argv, string, or vector of
     * strings)
     * @return 0 on success, non-zero indicates failure
     */
    template<typename... Args>
    int initialize(const helics::FederateInfo& fi, Args... args)
    {
        setupArgumentParsing();
        return internalInitialize(fi, parseArgs(args...));
    }

    /** make the federate ready to run; enter execution mode and setup initial state*/
    void makeReady()
    {
        if (!initialized) {
            throw("must initialize first");
        }
        fed->enterExecutingMode();
        doMakeReady();
        readyToRun = true;
    }

    /** print formatted results from the simulation*/
    void printResults()
    {
        doAddBenchmarkResults();
        for (const auto& r : results) {
            if (result_format == OutputFormat::PLAIN_TEXT) {
                std::cout << r.name << ": " << r.value << std::endl;
            }
        }
    }

    /** add a named result to the list of results to output
     * @param name the name of the result
     * @param key the short key name of the result
     * @param value the value of the result (must support printing with iostream)
     */
    template<class T>
    void addResult(const std::string& name, const std::string& key, T value)
    {
        std::ostringstream s;
        s << value;
        addResult(name, key, s.str());
    }

    /** add a named string result to the list of results to show
     * @param name the name of the result
     * @param key the short key name of the result
     * @param value a string representation of the result
     */
    void addResult(const std::string& name, const std::string& key, const std::string& value)
    {
        Result r;
        r.name = name;
        r.key = key;
        r.value = value;
        results.push_back(r);
    }

  private:
    /** call federate finalize() and handle before/after callbacks*/
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

    /** do the main loop for the benchmark -- callbacks for extra timing info could be added here*/
    void execute() { doMainLoop(); }

    /** internal initialization function that handles federate info arguments and calling derived
     * class virtual functions*/
    int internalInitialize(helics::FederateInfo fi, int parseArgsResult)
    {
        if (parseArgsResult != 0) {
            initialized = false;
            return parseArgsResult;
        }
        fi.loadInfoFromArgs(app->remainArgs());

        doParamInit(fi);
        std::string name = getName();
        fed = std::make_unique<helics::CombinationFederate>(name, fi);
        doFedInit();
        initialized = true;
        return 0;
    }

    /** parse arguments with CLI11 and handle standard help and version arguments*/
    template<typename... Args>
    int parseArgs(Args... args)
    {
        auto res = app->helics_parse(args...);

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
