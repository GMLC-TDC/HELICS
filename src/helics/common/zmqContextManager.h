/*
Copyright (C) 2017, Battelle Memorial Institute
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

#ifndef ZMQ_CONTEXT_MANAGER_HEADER_
#define ZMQ_CONTEXT_MANAGER_HEADER_

#include <vector>
#include <memory>
#include <string>
#include <map>

namespace zmq
{
	class context_t;
}

/** class defining a singleton context manager for all zmq usage in gridDyn*/
class zmqContextManager
{
private:
	static std::map<std::string, std::shared_ptr<zmqContextManager>> contexts; //!< container for pointers to all the available contexts
	std::string name;  //!< context name
	std::unique_ptr<zmq::context_t> zcontext; //!< pointer to the actual context
	zmqContextManager(const std::string &contextName);

public:
	static std::shared_ptr<zmqContextManager> getContextPointer(const std::string &contextName="");

	static zmq::context_t &getContext(const std::string &contextName="");

	static void closeContext(const std::string &contextName="");

	virtual ~zmqContextManager();

	const std::string &getName() const
	{
		return name;
	}

	zmq::context_t &getBaseContext() const
	{
		return *zcontext;
	}



};

#endif
