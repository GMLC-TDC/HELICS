/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include "../../common/zmqContextManager.h"
#include "../../common/zmqHelper.h"
#include "../../common/zmqSocketDescriptor.h"
#include "../ActionMessage.hpp"
#include "../NetworkBrokerData.hpp"
#include "ZmqCommsTest.h"
#include "ZmqRequestSets.h"
//#include <boost/asio.hpp>
//#include <csignal>
#include <memory>

static const int DEFAULT_BROKER_PORT_NUMBER = 23404; //Todo define a different port number

using namespace std::chrono;
/** bind a zmq socket, with a timeout and timeout period*/
static bool
bindzmqSocket (zmq::socket_t &socket, const std::string &address, int port, milliseconds timeout, milliseconds period = milliseconds(200))
{
    bool bindsuccess = false;
    milliseconds tcount{ 0 };
    while (!bindsuccess)
    {
        try
        {
            socket.bind (helics::makePortAddress (address, port));
            bindsuccess = true;
        }
        catch (const zmq::error_t &)
        {
            if (tcount == milliseconds(0))
            {
                std::cerr << "zmq binding error on socket sleeping then will try again \n";
            }
            if (tcount > timeout)
            {
                break;
            }
            std::this_thread::sleep_for (period);
            tcount += period;
        }
    }
    return bindsuccess;
}

namespace helics
{
namespace zeromq
{
void ZmqCommsTest::loadNetworkInfo (const NetworkBrokerData &netInfo)
{
    NetworkCommsInterface::loadNetworkInfo (netInfo);
    if (!propertyLock ())
    {
        return;
    }
    if (!brokerTarget_.empty())
    {
        insertProtocol(brokerTarget_, interface_type::tcp);
    }
    if (!localTarget_.empty())
    {
        insertProtocol(localTarget_, interface_type::tcp);
    }
    if (localTarget_ == "tcp://localhost")
    {
        localTarget_ = "tcp://127.0.0.1";
    }
    else if (localTarget_ == "udp://localhost")
    {
        localTarget_ = "udp://127.0.0.1";
    }
    if (brokerTarget_ == "tcp://localhost")
    {
        brokerTarget_ = "tcp://127.0.0.1";
    }
    else if (brokerTarget_ == "udp://localhost")
    {
        brokerTarget_ = "udp://127.0.0.1";
    }
    propertyUnLock ();
}

ZmqCommsTest::ZmqCommsTest () noexcept : NetworkCommsInterface (interface_type::ip) {}

/** destructor*/
ZmqCommsTest::~ZmqCommsTest () { disconnect (); }

int ZmqCommsTest::getDefaultBrokerPort () const { return DEFAULT_BROKER_PORT_NUMBER; }

int ZmqCommsTest::processIncomingMessage (zmq::message_t &msg,
		std::map<std::string, std::string> &connection_info)
{
	int status = 0;
    if (msg.size () == 5)
    {
        std::string str (static_cast<char *> (msg.data ()), msg.size ());
        if (str == "close")
        {
            return (-1);
        }
    }
    ActionMessage M (static_cast<char *> (msg.data ()), msg.size ());
	//std::cout << "****NAME: " << name << " Frontend rx message" <<
	//		" Protocol command action: " << M.action() << " message ID: " << M.messageID << std::endl;

    if (!isValidCommand (M))
    {
        std::cerr << "invalid command received" << std::endl;
        return 0;
    }
    if (isProtocolCommand (M))
    {
        switch (M.messageID)
        {
        case PORT_DEFINITIONS:
        	loadPortDefinitions (M);
        	break;
        case NAME_NOT_FOUND:
			//std::cout << "broker name " << brokerName_ << " does not match broker connection\n";
			disconnecting = true;
			setRxStatus (connection_status::error);
			status = -1;
			break;
        case DISCONNECT:
        	disconnecting = true;
            setRxStatus (connection_status::terminated);
            status = -1;
        case DISCONNECT_ERROR:
        	disconnecting = true;
			setRxStatus (connection_status::error);
			status = -1;
			break;
        case CLOSE_RECEIVER:
            status = -1;
            break;
        case RECONNECT_RECEIVER:
            setRxStatus (connection_status::connected);
            break;
        case CONNECTION_INFORMATION:
        		if(serverMode) {
        			//std::cout << name << "Adding connection info: " << M.payload << std::endl;
        			connection_info.emplace(M.name, M.payload);
//        			for(auto &mc : connection_info) {
//        				//std::cout << name << "conn info: " << mc.first << "second: " << mc.second << std::endl;
//        			}
        		}
        		break;
        default:
            break;
        }
    }
    ActionCallback (std::move (M));
    return status;
}

int ZmqCommsTest::replyToIncomingMessage (zmq::message_t &msg, zmq::socket_t &sock)
{
    ActionMessage M (static_cast<char *> (msg.data ()), msg.size ());
    if (isProtocolCommand (M))
    {
        if (M.messageID == CLOSE_RECEIVER)
        {
            return (-1);
        }
        auto reply = generateReplyToIncomingMessage (M);
        auto str = reply.to_string ();
        sock.send (str.data (), str.size ());
        return 0;
    }
    else
    {
        ActionCallback (std::move (M));
        ActionMessage resp (CMD_PRIORITY_ACK);
        auto str = resp.to_string ();
        sock.send (str.data (), str.size ());
        return 0;
    }
}

void ZmqCommsTest::queue_rx_function ()
{
	std::vector<char> buffer;
    auto ctx = zmqContextManager::getContextPointer ();
    // contains mapping between route id and core name
    std::map<route_id_t, std::string> routes;
	// control socket to receive control commands from tx
	// contains mapping between core name and address
	std::map<std::string, std::string> connection_info;

    // backend socket is to get messages from transmitter.
    // The messages will be processed based control route id or data route id
    // NOTE: ZMQ sockets are not thread-safe. So using inproc sockets
    zmq::socket_t backendSocket (ctx->getContext (), ZMQ_PAIR);
    std::string backendsocketstring = std::string ("inproc://") + name + "_backend";
    try
    {
        backendSocket.bind (backendsocketstring.c_str ());
    }
    catch (const zmq::error_t &e)
    {
        logError (std::string ("binding error on internal comms socket:") + e.what ());
        setRxStatus (connection_status::error);
        return;
    }
    backendSocket.setsockopt (ZMQ_LINGER, 200);

    if (brokerPort < 0)
    {
        brokerPort = DEFAULT_BROKER_PORT_NUMBER;
    }
    int sock_type = ZMQ_DEALER;
    if (serverMode) {
    	sock_type = ZMQ_ROUTER;
    }
    zmq::socket_t brokerConnection(ctx->getContext (), sock_type);
	if (serverMode)
	{
		brokerConnection.setsockopt (ZMQ_LINGER, 500);
		auto bindsuccess = bindzmqSocket (brokerConnection, localTarget_, brokerPort, connectionTimeout);
		if (!bindsuccess)
		{
			brokerConnection.close ();
			disconnecting = true;
			logError (std::string ("Unable to bind zmq router socket giving up ") +
					  makePortAddress (localTarget_, brokerPort));
			setRxStatus (connection_status::error);
			return;
		}
	} else {
		brokerConnection.setsockopt(ZMQ_IDENTITY, name.c_str(), name.size());
		brokerConnection.setsockopt (ZMQ_LINGER, 500);
		brokerConnection.connect(makePortAddress (brokerTarget_, brokerPort));

		// generate a local protocol connection string
		ActionMessage cmessage (CMD_PROTOCOL);
		cmessage.messageID = CONNECTION_INFORMATION;
		cmessage.name = name;
		cmessage.payload = getAddress ();
		cmessage.to_vector(buffer);
		//std::cout << "***NAME: " << name << " action: " << cmessage.action() << '\n';
		brokerConnection.send(buffer.data(), buffer.size());
	}

    std::vector<zmq::pollitem_t> poller (2);
    poller[0].socket = static_cast<void *> (backendSocket);
    poller[0].events = ZMQ_POLLIN;
    poller[1].socket = static_cast<void *> (brokerConnection);
    poller[1].events = ZMQ_POLLIN;

    setRxStatus (connection_status::connected);
    while (true)
    {
        auto rc = zmq::poll (poller, std::chrono::milliseconds (1000));
        if (rc > 0)
        {
            zmq::message_t msg;

            if ((poller[0].revents & ZMQ_POLLIN) != 0)
            {
            	backendSocket.recv(&msg);
            	ActionMessage wrapper (static_cast<char *> (msg.data ()), msg.size ());
            	route_id_t route_id(wrapper.messageID);
            	ActionMessage cmd (wrapper.payload);
            	//std::cout << name << "route_id: " << route_id << " parent route id: " << parent_route_id << '\n';
            	//std::cout << "**** NAME: " << name << " BACKEND RX message" << ": protocol command: " << cmd.action() << "message ID:" << cmd.messageID << std::endl;
            	if (isProtocolCommand (cmd))
				{
					if (route_id == control_route)
					{
						//std::cout << "Enter control route" << std::endl;
						bool close_tx = processTxControlCmd(cmd, routes, connection_info);

						if(close_tx) {
							routes.clear();
							if (getRxStatus () == connection_status::connected)
							{
								setRxStatus (connection_status::terminated);
								brokerConnection.close();
							}
							backendSocket.close ();
							setTxStatus (connection_status::terminated);
							break;
						}
					}
					continue;
				}
            	buffer.clear();
				cmd.to_vector(buffer);
				if (route_id == parent_route_id)
				{
					if(hasBroker) {
						//std::cout << "**** NAME: " << name << " Sending message to broker: " << cmd.action()
//								<< "message ID: " << cmd.messageID << std::endl;
						brokerConnection.send (buffer.data (), buffer.size ());
					}
					else
		            {
		                logWarning ("no route to broker for message");
		            }
				}
				else if (route_id == control_route)
				{
					auto status = processIncomingMessage (msg, connection_info);//----------> ToCheck
					if (status < 0)
					{
						break;
					}
				} else {
					// send out through the front end socket connection
					auto rt_find = routes.find (route_id);
					if (rt_find != routes.end ())
					{
						std::string name = rt_find->second;
						//std::cout << "Sending message to core: " << name << std::endl;
						//Need to first send identity and empty string
						brokerConnection.send (name.c_str(), name.size (), ZMQ_SNDMORE);
						brokerConnection.send ("", name.size (), ZMQ_SNDMORE);
						//Send the actual data
						brokerConnection.send (buffer.data (), buffer.size (), ZMQ_NOBLOCK);
					}
					else
					{
						if (hasBroker)
						{
							brokerConnection.send (buffer.data (), buffer.size ());
						}
						else
						{
							logWarning ("unknown route and no broker, dropping message");
						}
					}
				}
            }
            if ((poller[1].revents & ZMQ_POLLIN) != 0)
            {
            	brokerConnection.recv(&msg);
            	std::string str (static_cast<char *> (msg.data ()), msg.size ());

            	//std::cout << " FRONTEND RX MESSAGE: Identity" << name << " *****DATA: " << str << '\n';
            	brokerConnection.recv(&msg);
            	std::string str2 (static_cast<char *> (msg.data ()), msg.size ());
            	//std::cout << " FRONTEND RX MESSAGE: message data" << name << " *****DATA: " << str2 << '\n';
                auto status = processIncomingMessage (msg, connection_info);
//				for(auto &mc : connection_info) {
//					//std::cout << name << " mc.second: " << mc.second << std::endl;
//				}
                if (status < 0)
                {
                    break;
                }
            }
		}
        if (requestDisconnect.load (std::memory_order::memory_order_acquire))
        {
            break;
        }
    }
    disconnecting = true;
    setRxStatus (connection_status::terminated);
}

bool ZmqCommsTest::processTxControlCmd(ActionMessage cmd,
		std::map<route_id_t, std::string> &routes,
		std::map<std::string, std::string> &connection_info)
{
	bool close_tx = false;

	switch (cmd.messageID)
	{
	case RECONNECT:
		setTxStatus (connection_status::connected);
		break;
	case CONNECTION_INFORMATION:
		// Shouldn't reach here ideally
		if(serverMode) {
			connection_info.emplace(cmd.name, cmd.payload);
		}
		break;
	case NEW_ROUTE:
		////std::cout << name << " update connection info" << std::endl;
		try
		{
			for(auto &mc : connection_info) {
				//std::cout << name << " mc.second: " << mc.second << " cmd.payload: " << cmd.payload << std::endl;
				if(mc.second == cmd.payload) {

					routes.emplace(route_id_t(cmd.getExtraData()), mc.first);
					//std::cout << "Route id: " << cmd.getExtraData() << std::endl;
					break;
				}
			}
		}
		catch (const zmq::error_t &e)
		{
			// TODO:: do something???
			logError (std::string ("unable to connect route") + cmd.payload + "::" + e.what ());
		}
		break;
	case DISCONNECT:
		close_tx = true;
		break;
	}
	return close_tx;
}

void ZmqCommsTest::queue_tx_function ()
{
    std::vector<char> buffer;
    if (!brokerTarget_.empty())
	{
		hasBroker = true;
	}
    auto ctx = zmqContextManager::getContextPointer ();

    // Setup the backend socket for comms with the receiver
    zmq::socket_t backendSocket (ctx->getContext (), ZMQ_PAIR);
    backendSocket.setsockopt (ZMQ_LINGER, 200);
    std::string backendsockString = std::string ("inproc://") + name + "_backend";
    backendSocket.connect (backendsockString);

    std::map<std::string, std::string> connection_info;  // contains mapping between core name and address
    std::map<route_id_t, std::string> routes;  // contains mapping between route id and core name

    setTxStatus (connection_status::connected);
    zmq::message_t msg;

    while (true)
    {
        route_id_t route_id;
        ActionMessage cmd;
		//std::cout << "***NAME: " << name << " BACKEND TX message: " << " action: " << cmd.action() << " route id: " <<
		//		route_id << " message ID: " << cmd.messageID << '\n';
        ActionMessage backendMessage = ActionMessage();

        std::tie (route_id, cmd) = txQueue.pop ();
        backendMessage.messageID = route_id.baseValue();
        backendMessage.payload = cmd.packetize();
        backendMessage.to_vector (buffer);
        backendSocket.send (buffer.data (), buffer.size ());
        // Todo check for close and exit from loop
        if (isProtocolCommand (cmd) && (route_id == control_route))
        {
        	if(cmd.messageID == DISCONNECT) {
        		backendSocket.close();
        		return;
        	}
        }
    }
}

void ZmqCommsTest::closeReceiver ()
{
    switch (getTxStatus ())
    {
    case connection_status::startup:
    case connection_status::connected:
    {
        ActionMessage cmd (CMD_PROTOCOL);
        cmd.messageID = CLOSE_RECEIVER;
        transmit (control_route, cmd);
    }

    break;
    default:
        if (!disconnecting)
        {
            // try connecting with the receivers push socket
        	ActionMessage cmd (CMD_PROTOCOL);
        	cmd.messageID = CLOSE_RECEIVER;
        	transmit (control_route, cmd);
        }
        break;
    }
}

}  // namespace zeromq
}  // namespace helics
