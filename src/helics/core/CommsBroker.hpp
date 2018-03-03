/*
Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.
*/

#pragma once
#include <atomic>
#include <memory>
#include <mutex>
#include <string>

namespace helics
{
class ActionMessage;
class CommsInterface;

/** helper class defining some common functionality for brokers and cores that use different
communication methods*/
template <class COMMS, class BrokerT>
class CommsBroker : public BrokerT
{
  protected:
    std::atomic<int> disconnectionStage{0};  //!< the stage of disconnection
    mutable std::mutex dataMutex;  //!< mutex protecting comms data
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

  public:
    virtual void transmit (int route_id, const ActionMessage &cmd) override;

    virtual void addRoute (int route_id, const std::string &routeInfo) override;
};
}  // namespace helics
