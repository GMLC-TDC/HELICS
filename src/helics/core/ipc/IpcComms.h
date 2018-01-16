/*

Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef _HELICS_IPC_COMMS_
#define _HELICS_IPC_COMMS_
#pragma once

#include "../CommsInterface.h"
#include <atomic>

namespace helics {

/** implementation for the core that uses boost interprocess messages to communicate*/
class IpcComms final:public CommsInterface {

public:
	/** default constructor*/
	IpcComms() = default;
	IpcComms(const std::string &brokerTarget, const std::string &localTarget);
	/** destructor*/
	~IpcComms();

private:
    std::atomic<int> ipcbackchannel{0}; //!< a back channel message system if the primary is not working

	virtual void queue_rx_function() override;	//!< the functional loop for the receive queue
	virtual void queue_tx_function() override;  //!< the loop for transmitting data
	virtual void closeReceiver() override;  //!< function to instruct the receiver loop to close
	
private:
	
};

#define IPC_BACKCHANNEL_TRY_RESET 2
#define IPC_BACKCHANNEL_DISCONNECT 4

} // namespace helics

#endif /* _HELICS_IPC_COMMS_ */

