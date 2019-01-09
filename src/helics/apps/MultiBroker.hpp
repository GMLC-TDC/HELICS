/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#pragma once
#include "../core/CoreBroker.hpp"

namespace helics
{
class ActionMessage;
class CommsInterface;

/** helper class defining some common functionality for brokers and cores that use different
communication methods*/
class MultiBroker : public CoreBroker
{
  protected:
    std::atomic<int> disconnectionStage{0};  //!< the stage of disconnection
    std::vector<std::unique_ptr<CommsInterface>> comms;  //!< the actual comms object
    std::unique_ptr<CommsInterface> masterComm;
    std::atomic<bool> brokerInitialized{false};  //!< atomic protecting local initialization
  public:
    /** default constructor*/
    MultiBroker () noexcept;
    /** construct from command line arguments
    @param argc the number of arguments
    @param argv the strings in the input
    */
    MultiBroker (int argc, char *argv[]);
    /** construct from command line arguments parsed as a single string
    @param argString a merged string with all the arguments
    */
    explicit MultiBroker (const std::string &argString);
    /** destructor*/
    ~MultiBroker ();

  private:
    virtual void brokerDisconnect () override;
    virtual bool tryReconnect () override;
    /** disconnect the comm object*/
    void commDisconnect ();
    void loadComms ();

  public:
    virtual void transmit (route_id rid, const ActionMessage &cmd) override;
    virtual void transmit (route_id rid, ActionMessage &&cmd) override;

    virtual void addRoute (route_id rid, const std::string &routeInfo) override;
};
}  // namespace helics
