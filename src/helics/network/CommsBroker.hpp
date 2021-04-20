/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once
#include "helics/core/ActionMessage.hpp"

#include <atomic>
#include <memory>
#include <string>

namespace helics {
class CommsInterface;

/** helper class defining some common functionality for brokers and cores that use different
communication methods*/
template<class COMMS, class BrokerT>
class CommsBroker: public BrokerT {
  protected:
    std::atomic<int> disconnectionStage{0};  //!< the stage of disconnection
    std::unique_ptr<COMMS> comms;  //!< the actual comms object
    std::atomic<bool> brokerInitialized{false};  //!< atomic protecting local initialization
  public:
    /** default constructor*/
    CommsBroker() noexcept;
    /** create from a single argument*/
    explicit CommsBroker(bool arg) noexcept;
    /** create from an object name*/
    explicit CommsBroker(const std::string& obj_name);
    /** destructor*/
    ~CommsBroker();

  private:
    virtual void brokerDisconnect() override;
    virtual bool tryReconnect() override;
    /** disconnect the comm object*/
    void commDisconnect();
    /** load the comms object directly*/
    void loadComms();

  public:
    virtual void transmit(route_id rid, const ActionMessage& cmd) override;
    virtual void transmit(route_id rid, ActionMessage&& cmd) override;

    virtual void addRoute(route_id rid, int interfaceId, const std::string& routeInfo) override;

    virtual void removeRoute(route_id rid) override;
    /** get a pointer to the comms object*/
    COMMS* getCommsObjectPointer();
};
}  // namespace helics
