/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#pragma once
#include "../application_api/Endpoints.hpp"
#include "../application_api/Subscriptions.hpp"
#include "helicsApp.hpp"
#include <map>
#include <memory>
#include <set>

namespace helics
{
class CloningFilter;

namespace apps
{
/** helper class for capturing data points*/
class ValueCapture
{
  public:
    helics::Time time;
    int index = -1;
    int16_t iteration = 0;
    bool first = false;
    std::string value;
    ValueCapture () = default;
    ValueCapture (helics::Time t1, int id1, const std::string &val) : time (t1), index (id1), value (val){};
};

/** helper class for displaying statistics*/
class ValueStats
{
  public:
    helics::Time time = helics::Time::minVal ();
    std::string lastVal;
    std::string key;
    int cnt = 0;
};

/** class designed to capture data points from a set of subscriptions or endpoints*/
class Recorder : public App
{
  public:
    /** construct from a FederateInfo structure*/
    explicit Recorder (const std::string &name, FederateInfo &fi);
    /** construct from command line arguments*/
    Recorder (int argc, char *argv[]);

    /**constructor taking a federate information structure and using the given core
    @param core a pointer to core object which the federate can join
    @param[in] fi  a federate information structure
    */
    Recorder (const std::string &name, const std::shared_ptr<Core> &core, const FederateInfo &fi);
    /**constructor taking a file with the required information
    @param[in] name the name of the app
    @param[in] file a file defining the federate information
    */
    Recorder (const std::string &name, const std::string &jsonString);
    /** move construction*/
    Recorder (Recorder &&other_recorder) = default;
    /** move assignment*/
    Recorder &operator= (Recorder &&record) = default;
    /** destructor*/
    ~Recorder ();
    /** run the Player until the specified time*/
    virtual void runTo (Time runToTime) override;
    /** add a subscription to capture*/
    void addSubscription (const std::string &key);
    /** add an endpoint*/
    void addEndpoint (const std::string &endpoint);
    /** copy all messages that come from a specified endpoint*/
    void addSourceEndpointClone (const std::string &sourceEndpoint);
    /** copy all messages that are going to a specific endpoint*/
    void addDestEndpointClone (const std::string &destEndpoint);
    /** add a capture interface
    @param captureDesc describes a federate to capture all the interfaces for
    */
    void addCapture (const std::string &captureDesc);
    /** save the data to a file*/
    void saveFile (const std::string &filename);
    /** get the number of captured points*/
    auto pointCount () const { return points.size (); }
    /** get the number of captured messages*/
    auto messageCount () const { return messages.size (); }
    /** get a string with the value of point index
    @param index the number of the point to retrieve
    @return a pair with the tag as the first element and the value as the second
    */
    std::pair<std::string, std::string> getValue (int index) const;
    /** get a message
    @details makes a copy of a message and returns it in a unique_ptr
    @param index the number of the message to retrieve
    */
    std::unique_ptr<Message> getMessage (int index) const;

  private:
    /** load arguments through a variable map created through command line arguments
     */
    int loadArguments (boost::program_options::variables_map &vm_map);
    /** load from a jsonString
    @param jsonString either a JSON filename or a string containing JSON
    */
    virtual void loadJsonFile (const std::string &jsonString) override;
    /** load a text file*/
    virtual void loadTextFile (const std::string &textFile) override;
    /** helper function to write the date to a JSON file*/
    void writeJsonFile (const std::string &filename);
    /** helper function to write the date to a text file*/
    void writeTextFile (const std::string &filename);

    virtual void initialize () override;
    void generateInterfaces ();
    void captureForCurrentTime (Time currentTime, int iteration = 0);
    void loadCaptureInterfaces ();
    /** encode the string in base64 if needed otherwise just return the string*/
    std::string encode (const std::string &str2encode);

  protected:
    bool allow_iteration = false;  //!< trigger to allow Iteration
    bool verbose = false;  //!< print all captured values to the screen
    Time nextPrintTimeStep = helics::timeZero;  //!< the time advancement period for printing markers
    std::unique_ptr<CloningFilter> cFilt;  //!< a pointer to a clone filter
    std::vector<ValueCapture> points;  //!< lists of points that were captured
    std::vector<Input> subscriptions;  //!< the actual subscription objects
    std::vector<Endpoint> endpoints;  //!< the actual endpoint objects
    std::unique_ptr<Endpoint> cloneEndpoint;  //!< the endpoint for cloned message delivery
    std::vector<std::unique_ptr<Message>> messages;  //!< list of messages
    std::map<helics::interface_handle, int> subids;  //!< map of the subscription ids
    std::map<std::string, int> subkeys;  //!< translate subscription names to an index
    std::map<helics::interface_handle, int> eptids;  // translate subscription id to index
    std::map<std::string, int> eptNames;  //!< translate endpoint name to index
    std::vector<ValueStats> vStat;  //!< storage for statistics capture
    std::vector<std::string> captureInterfaces;  //!< storage for the interfaces to capture
    std::string mapfile;  //!< file name for the on-line file updater
    std::string outFileName;  //!< the final output file
};

}  // namespace apps
}  // namespace helics
