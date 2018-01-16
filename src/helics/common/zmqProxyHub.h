/*
Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.
*/
/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil;  eval: (c-set-offset 'innamespace 0); -*- */
/*
* LLNS Copyright Start
* Copyright (c) 2017, Lawrence Livermore National Security
* This work was performed under the auspices of the U.S. Department
* of Energy by Lawrence Livermore National Laboratory in part under
* Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
* Produced at the Lawrence Livermore National Laboratory.
* All rights reserved.
* For details, see the LICENSE file.
* LLNS Copyright End
*/

#ifndef ZMQPROXYHUB_H_
#define ZMQPROXYHUB_H_


#include <memory>
#include "zmqSocketDescriptor.h"
#include "zmqContextManager.h"
#include <functional>
#include <thread>
#include <atomic>

/** class building and managing a zmq proxy
@details the proxy runs in its own thread managed by the proxy class
*/
class zmqProxyHub
{
public:
	static std::shared_ptr<zmqProxyHub> getProxy(const std::string &proxyName, const std::string &pairType="pubsub", const std::string &contextName = "");

	~zmqProxyHub();

	void startProxy();
	void stopProxy();
	void modifyIncomingConnection(socket_ops op,const std::string &connection);
	void modifyOutgoingConnection(socket_ops op, const std::string &connection);

	const std::string &getName() const
	{
		return name;
	}
	const std::string &getIncomingConnection() const
	{
		return incomingPrimaryConnection;
	}
	const std::string &getOutgoingConnection() const
	{
		return outgoingPrimaryConnection;
	}
	bool isRunning() const
	{
		return proxyRunning;
	}
private:
	static std::vector<std::shared_ptr<zmqProxyHub>> proxies; //!< container for pointers to all the available contexts
	
	std::string name;  //!< the name of the proxy
	std::string outgoingPrimaryConnection;  //!< the primary outgoing connection
	std::string incomingPrimaryConnection; //!< the primary incoming connection

	std::shared_ptr<zmqContextManager> contextManager;  //!< pointer the context the reactor is using
	std::unique_ptr<zmq::socket_t> controllerSocket;  //!< socket used for control of the proxy
	zmqSocketDescriptor incoming;  //!< socketDescriptor for the incoming connection
	zmqSocketDescriptor outgoing;	//!< socketDescriptor for the outgoing connection
	std::thread proxyThread;	//!< the thread id for the proxy loop
	std::atomic<bool> proxyRunning{ false };	//!< flag indicating the proxy has been started
	/** private constructor*/
	zmqProxyHub(const std::string &proxyName, const std::string &pairtype, const std::string &context);
	/** loop for the proxy thread*/
	void proxyLoop();
};
#endif


