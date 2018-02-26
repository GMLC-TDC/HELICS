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
#include "../application_api/Subscriptions.hpp"
#include <map>
#include <memory>
#include <set>

#include "PrecHelper.hpp"

namespace boost
{
namespace program_options
{
class variables_map;
}
}

namespace helics
{

class CloningFilter;

/** class designed to capture data points from a set of subscriptions or endpoints*/
class Tracer
{
  public:
    /** construct from a FederateInfo structure*/
    explicit Tracer (FederateInfo &fi);
    /** construct from command line arguments*/
    Tracer (int argc, char *argv[]);

    /**constructor taking a federate information structure and using the given core
    @param core a pointer to core object which the federate can join
    @param[in] fi  a federate information structure
    */
    Tracer (const std::shared_ptr<Core> &core, const FederateInfo &fi);
    /**constructor taking a file with the required information
    @param[in] file a file defining the federate information
    */
    explicit Tracer (const std::string &jsonString);
    /** move construction*/
    Tracer (Tracer &&other_tracer) = default;
    /** don't allow the copy constructor*/
    Tracer (const Tracer &other_tracer) = delete;
    /** move assignment*/
    Tracer &operator= (Tracer &&tracer) = default;
    /** don't allow the copy assignment,  the default would fail anyway since federates are not copyable either*/
    Tracer &operator= (const Tracer &tracer) = delete;
    /** destructor*/
    ~Tracer ();

    /** load a file containing subscription information
    @param filename the name of the file to load (.txt, .json, or .xml
    @return 0 on success <0 on failure
    */
    int loadFile (const std::string &filename);

    /*run the Player*/
    void run ();
    /** run the Player until the specified time*/
    void run (Time stopTime);
    /** add a subscription to capture*/
    void addSubscription (const std::string &key);
    /** copy all messages that come from a specified endpoint*/
    void addSourceEndpointClone (const std::string &sourceEndpoint);
    /** copy all messages that are going to a specific endpoint*/
    void addDestEndpointClone (const std::string &destEndpoint);
    /** add a capture interface
    @param captureDesc describes a federate to capture all the interfaces for
    */
    void addCapture (const std::string &captureDesc);
    
    /** finalize the federate*/
    void finalize ();

    /** check if the Tracer is ready to run*/
    bool isActive () const { return !deactivated; }

  private:
    /** load arguments through a variable map created through command line arguments
     */
    int loadArguments (boost::program_options::variables_map &vm_map);
    /** load from a jsonString
    @param either a JSON filename or a string containing JSON
    */
    int loadJsonFile (const std::string &jsonString);
    /** load a text file*/
    int loadTextFile (const std::string &textFile);
    
    void initialize ();
    void generateInterfaces ();
    void captureForCurrentTime (Time currentTime);
    void loadCaptureInterfaces ();
    /** encode the string in base64 if needed otherwise just return the string*/
    std::string encode (const std::string &str2encode);

  protected:
    std::shared_ptr<CombinationFederate> fed;  //!< the federate
    std::unique_ptr<CloningFilter> cFilt;  //!< a pointer to a clone filter
  
    std::vector<Subscription> subscriptions;  //!< the actual subscription objects
    std::vector<Endpoint> endpoints;  //!< the actual endpoint objects
    std::unique_ptr<Endpoint> cloneEndpoint;  //!< the endpoint for cloned message delivery

    std::map<helics::subscription_id_t, int> subids;  //!< map of the subscription ids
    std::map<std::string, int> subkeys;  //!< translate subscription names to an index
    std::vector<std::string> captureInterfaces;  //!< storage for the interfaces to capture

    Time autoStopTime = Time::maxVal ();  //!< the stop time
    bool deactivated = false;
};

}  // namespace helics