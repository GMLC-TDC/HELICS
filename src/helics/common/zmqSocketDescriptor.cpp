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

#include "zmqSocketDescriptor.h"

zmq::socket_t zmqSocketDescriptor::makeSocket(zmq::context_t &ctx) const
{
	zmq::socket_t sock(ctx, type);
	modifySocket(sock);
	return sock;
}


std::unique_ptr<zmq::socket_t> zmqSocketDescriptor::makeSocketPtr(zmq::context_t &ctx) const
{
	auto sock=std::make_unique<zmq::socket_t>(ctx, type);
	modifySocket(*sock);
	return sock;
}

void zmqSocketDescriptor::modifySocket(zmq::socket_t &sock) const
{
	for (auto &op : ops)
	{
		switch (op.first)
		{
		case socket_ops::bind:
			sock.bind(op.second);
			break;
		case socket_ops::unbind:
			sock.unbind(op.second);
			break;
		case socket_ops::connect:
			sock.connect(op.second);
			break;
		case socket_ops::disconnect:
			sock.disconnect(op.second);
			break;
		case socket_ops::subscribe:
			if ((type == zmq::socket_type::pub) || (type == zmq::socket_type::sub))
			{
				sock.setsockopt(ZMQ_SUBSCRIBE, op.second);
			}
			break;
		case socket_ops::unsubscribe:
			if ((type == zmq::socket_type::pub) || (type == zmq::socket_type::sub))
			{
				sock.setsockopt(ZMQ_UNSUBSCRIBE, op.second);
			}
			break;
		default:
			break;
		}
	}
}
