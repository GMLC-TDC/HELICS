/*
Copyright © 2017-2018, Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once

#include "../application_api/CombinationFederate.hpp"
#include "../application_api/Endpoints.hpp"
#include "../application_api/Subscriptions.hpp"
#include <map>
#include <memory>
#include <functional>

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

namespace apps
{
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
    /** add an endpoint*/
    void addEndpoint(const std::string &endpoint);
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
    /** set the callback for a message received through cloned interfaces
    @details the function signature will take the time in the Tracer a unique ptr to the message
    */
    void setClonedMessageCallback(std::function<void(Time, std::unique_ptr<Message>)> callback)
    {
        clonedMessageCallback = std::move(callback);
    }
    /** set the callback for a message received through endpoints
    @details the function signature will take the time in the Tracer, the endpoint name as a string, and a unique ptr to the message
    */
    void setEndpointMessageCallback(std::function<void(Time, const std::string &, std::unique_ptr<Message>)> callback)
    {
        endpointMessageCallback = std::move(callback);
    }
    /** set the callback for a value published
    @details the function signature will take the time in the Tracer, the publication key as a string, and the value as a string
    */
    void setValueCallback(std::function<void(Time, const std::string &, const std::string &)> callback)
    {
        valueCallback = std::move(callback);
    }
    /** turn the screen display on for values and messages*/
    void enableTextOutput() {
        printMessage = true;
    }
    void disableTextOutput()
    {
        printMessage = false;
    }
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



  protected:
    std::shared_ptr<CombinationFederate> fed;  //!< the federate
    std::unique_ptr<CloningFilter> cFilt;  //!< a pointer to a clone filter

    std::vector<Subscription> subscriptions;  //!< the actual subscription objects
    std::map<std::string, int> subkeys;  //!< translate subscription names to an index

    std::vector<Endpoint> endpoints;  //!< the actual endpoint objects
    std::map<std::string, int> eptNames;    //!< translate endpoint name to index
    std::unique_ptr<Endpoint> cloneEndpoint;  //!< the endpoint for cloned message delivery
    std::vector<std::string> captureInterfaces;  //!< storage for the interfaces to capture

    Time autoStopTime = Time::maxVal ();  //!< the stop time
    bool deactivated = false;
    bool printMessage = false;
    std::function<void(Time, std::unique_ptr<Message>)> clonedMessageCallback;
    std::function<void(Time, const std::string &, std::unique_ptr<Message>)> endpointMessageCallback;
    std::function<void(Time, const std::string &, const std::string &)> valueCallback;
};

}  // namespace apps
} // namespace helics

