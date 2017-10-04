/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "MpiComms.h"
#include "helics/core/ActionMessage.h"
#include <memory>

namespace helics
{
MpiComms::MpiComms (const std::string &brokerTarget, const std::string &localTarget)
    : CommsInterface (brokerTarget, localTarget)
{
}
/** destructor*/
MpiComms::~MpiComms () = default;

void MpiComms::queue_rx_function () {}

void MpiComms::closeTransmitter ()
{
    ActionMessage rt (CMD_PROTOCOL);
    rt.index = DISCONNECT;
    transmit (-1, rt);
}

void MpiComms::closeReceiver ()
{
    if (tx_status == connection_status::connected)
    {
        ActionMessage cmd (CMD_PROTOCOL);
        cmd.index = CLOSE_RECEIVER;
        transmit (-1, cmd);
    }
    else
    {
        // TODO:: what to do here
    }
}
}  // namespace helics