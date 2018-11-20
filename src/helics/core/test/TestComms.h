/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once

#include "helics/helics-config.h"
#include "../CommsInterface.hpp"
#include <future>
#include <set>

namespace helics
{
namespace testcore {
/** implementation for the communication interface that uses ZMQ messages to communicate*/
class TestComms final : public CommsInterface
{
  public:
    /** default constructor*/
    TestComms ();
    /** destructor*/
    ~TestComms ();

    virtual void loadNetworkInfo(const NetworkBrokerData &netInfo) override;
  private:
    virtual void queue_rx_function () override;  //!< the functional loop for the receive queue
    virtual void queue_tx_function () override;  //!< the loop for transmitting data
    virtual void closeReceiver () override;  //!< function to instruct the receiver loop to close

  public:

	  /** return a dummy port number*/
    int getPort () const { return -1; };

    std::string getAddress () const;
};

} // namespace testcore
}  // namespace helics

