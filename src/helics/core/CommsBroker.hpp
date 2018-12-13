/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#pragma once
#include "ActionMessage.hpp"
#include <atomic>
#include <memory>
#include <string>

namespace helics
{
class CommsInterface;

/** helper class defining some common functionality for brokers and cores that use different
communication methods*/
template <class COMMS, class BrokerT>
class CommsBroker : public BrokerT
{
  protected:
    std::atomic<int> disconnectionStage{0};  //!< the stage of disconnection
    std::unique_ptr<COMMS> comms;  //!< the actual comms object
    std::atomic<bool> initialized_{false};  //!< atomic protecting local initialization
  public:
    /** default constructor*/
    CommsBroker () noexcept;
    /** create from a single argument*/
    explicit CommsBroker (bool arg) noexcept;
    /** create from an object name*/
    explicit CommsBroker (const std::string &obj_name);
    /** destructor*/
    ~CommsBroker ();

  private:
    virtual void brokerDisconnect () override;
    virtual bool tryReconnect () override;
    /** disconnect the comm object*/
    void commDisconnect ();
    void loadComms ();

  public:
    virtual void transmit (route_id_t route_id, const ActionMessage &cmd) override;
    virtual void transmit (route_id_t route_id, ActionMessage &&cmd) override;

    virtual void addRoute (route_id_t route_id, const std::string &routeInfo) override;

    virtual void removeRoute (route_id_t route_id) override;
};
}  // namespace helics
