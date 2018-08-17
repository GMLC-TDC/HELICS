/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#include "TcpHelperClasses.h"
#include <iostream>
#include <thread>

namespace helics
{
namespace tcp
{
using boost::asio::ip::tcp;

void TcpRxConnection::start ()
{
    if (triggerhalt)
    {
        receiving = false;
        return;
    }
    if (state == connection_state_t::prestart)
    {
        receiving = true;
        state = connection_state_t::halted;
    }
    connection_state_t exp = connection_state_t::halted;
    if (state.compare_exchange_strong (exp, connection_state_t::receiving))
    {
        if (!receiving.load ())
        {
            receiving.store (true);
        }
        socket_.async_receive (boost::asio::buffer (data.data () + residBufferSize,
                                                    data.size () - residBufferSize),
                               [this](const boost::system::error_code &error, size_t bytes_transferred) {
                                   handle_read (error, bytes_transferred);
                               });
    }
    else if (exp != connection_state_t::receiving)
    {
        receiving = false;
    }
}

void TcpRxConnection::setDataCall (std::function<size_t (TcpRxConnection::pointer, const char *, size_t)> dataFunc)
{
    if (state == connection_state_t::prestart)
    {
        dataCall = std::move (dataFunc);
    }
    else
    {
        throw (std::runtime_error ("cannot set data callback after socket is started"));
    }
}
void TcpRxConnection::setErrorCall (
  std::function<bool(TcpRxConnection::pointer, const boost::system::error_code &)> errorFunc)
{
    if (state == connection_state_t::prestart)
    {
        errorCall = std::move (errorFunc);
    }
    else
    {
        throw (std::runtime_error ("cannot set error callback after socket is started"));
    }
}

void TcpRxConnection::handle_read (const boost::system::error_code &error, size_t bytes_transferred)
{
    if (triggerhalt)
    {
        state = connection_state_t::halted;
        receiving = false;
        return;
    }
    if (!error)
    {
        auto used = dataCall (shared_from_this (), data.data (), bytes_transferred + residBufferSize);
        if (used < (bytes_transferred + residBufferSize))
        {
            if (used > 0)
            {
                std::copy (data.data () + used, data.data () + bytes_transferred + residBufferSize, data.data ());
            }
            residBufferSize = bytes_transferred + residBufferSize - used;
        }
        else
        {
            residBufferSize = 0;
            data.assign (data.size (), 0);
        }
        state = connection_state_t::halted;
        start ();
    }
    else if (error == boost::asio::error::operation_aborted)
    {
        state = connection_state_t::halted;
        receiving = false;
        return;
    }
    else
    {
        if (bytes_transferred > 0)
        {
            auto used = dataCall (shared_from_this (), data.data (), bytes_transferred + residBufferSize);
            if (used < (bytes_transferred + residBufferSize))
            {
                if (used > 0)
                {
                    std::copy (data.data () + used, data.data () + bytes_transferred + residBufferSize,
                               data.data ());
                }
                residBufferSize = bytes_transferred + residBufferSize - used;
            }
            else
            {
                residBufferSize = 0;
            }
        }
        if (errorCall)
        {
            if (errorCall (shared_from_this (), error))
            {
                state = connection_state_t::halted;
                start ();
            }
            else
            {
                receiving = false;
            }
        }
        else if ((error != boost::asio::error::eof) && (error != boost::asio::error::operation_aborted))
        {
            if (error != boost::asio::error::connection_reset)
            {
                std::cerr << "receive error " << error.message () << std::endl;
            }
            state = connection_state_t::halted;
            receiving = false;
        }
        else
        {
            receiving = false;
        }
    }
}

void TcpRxConnection::send (const void *buffer, size_t dataLength)
{
    auto sz = socket_.send (boost::asio::buffer (buffer, dataLength));
    assert (sz == dataLength);
    (void)(sz);
}

void TcpRxConnection::send (const std::string &dataString)
{
    auto sz = socket_.send (boost::asio::buffer (dataString));
    assert (sz == dataString.size ());
    (void)(sz);
}

void TcpRxConnection::close ()
{
    triggerhalt = true;
    state = connection_state_t::closed;
    boost::system::error_code ec;
    socket_.shutdown (boost::asio::ip::tcp::socket::shutdown_send, ec);
    if (ec)
    {
        std::cerr << "error occurred sending shutdown::" << ec << std::endl;
    }
    socket_.close ();
    while (receiving)
    {
        std::this_thread::yield ();
    }
}

TcpConnection::pointer TcpConnection::create (boost::asio::io_service &io_service,
                                              const std::string &connection,
                                              const std::string &port,
                                              size_t bufferSize)
{
    return pointer (new TcpConnection (io_service, connection, port, bufferSize));
}

TcpConnection::TcpConnection (boost::asio::io_service &io_service,
                              const std::string &connection,
                              const std::string &port,
                              size_t bufferSize)
    : socket_ (io_service), data (bufferSize)
{
    tcp::resolver resolver (io_service);
    tcp::resolver::query query (tcp::v4 (), connection, port);
    tcp::resolver::iterator endpoint_iterator = resolver.resolve (query);
    socket_.async_connect (*endpoint_iterator,
                           [this](const boost::system::error_code &error) { connect_handler (error); });
}

void TcpConnection::connect_handler (const boost::system::error_code &error)
{
    if (!error)
    {
        connected.store (true);
    }
    else
    {
        std::cerr << "connection error " << error.message () << ": code =" << error.value () << '\n';
    }
}
void TcpConnection::send (const void *buffer, size_t dataLength)
{
    if (!isConnected ())
    {
        waitUntilConnected (200);
    }
    auto sz = socket_.send (boost::asio::buffer (buffer, dataLength));
    assert (sz == dataLength);
    ((void)(sz));
}

void TcpConnection::send (const std::string &dataString)
{
    if (!isConnected ())
    {
        waitUntilConnected (200);
    }
    auto sz = socket_.send (boost::asio::buffer (dataString));
    assert (sz == dataString.size ());
    ((void)(sz));
}

size_t TcpConnection::receive (void *buffer, size_t maxDataLength)
{
    return socket_.receive (boost::asio::buffer (buffer, maxDataLength));
}

int TcpConnection::waitUntilConnected (int timeOut)
{
    int cnt = 0;
    while (!isConnected ())
    {
        std::this_thread::sleep_for (std::chrono::milliseconds (50));
        cnt += 50;
        if (cnt > timeOut)
        {
            return (-1);
        }
    }
    return 0;
}

void TcpConnection::close ()
{
    cancel ();
    boost::system::error_code ec;
    socket_.shutdown (boost::asio::ip::tcp::socket::shutdown_send, ec);
    if (ec)
    {
        // I don't know what to do with this, in practice this message is mostly spurious
        // but is seems I should do something with it, I just don't know what
        // std::cerr << "error occurred sending shutdown" << std::endl;
        ((void)(ec));
    }
    socket_.close ();
}

TcpAcceptor::TcpAcceptor (boost::asio::io_service &io_service, boost::asio::ip::tcp::endpoint &ep)
    : acceptor_ (io_service), endpoint_ (ep)
{
    acceptor_.open (ep.protocol ());
}

TcpAcceptor::TcpAcceptor (boost::asio::io_service &io_service, int port)
    : acceptor_ (io_service, boost::asio::ip::tcp::endpoint (boost::asio::ip::tcp::v4 (), port)),
      endpoint_ (boost::asio::ip::tcp::v4 (), port), state (accepting_state_t::connected)
{

}

bool TcpAcceptor::connect ()
{
    accepting_state_t exp = accepting_state_t::opened;
    if (state.compare_exchange_strong (exp, accepting_state_t::connecting))
    {
    boost::system::error_code ec;
    acceptor_.bind (endpoint_, ec);
    if (ec)
    {
        state = accepting_state_t::opened;
        return false;
    }
    state = accepting_state_t::connected;
    return true;
	}
    return (state == accepting_state_t::connected);
}

bool TcpAcceptor::connect (int timeout)
{
	if (state == accepting_state_t::halted)
	{
        state = accepting_state_t::opened;
	}
    accepting_state_t exp = accepting_state_t::opened;
    if (state.compare_exchange_strong (exp, accepting_state_t::connecting))
    {
   
    bool bindsuccess = false;
    int tcount = 0;
    while (!bindsuccess)
    {
        boost::system::error_code ec;
        acceptor_.bind (endpoint_, ec);
        if (ec)
        {
            if (tcount > timeout)
            {
                state = accepting_state_t::opened;
                break;
            }
            std::this_thread::sleep_for (std::chrono::milliseconds (200));
            tcount += 200;
        }
        else
        {
            state = accepting_state_t::connected;
            bindsuccess = true;
        }
    }
    return bindsuccess;
	}
    return (state == accepting_state_t::connected);
}

/** start the acceptor*/
bool TcpAcceptor::start (TcpRxConnection::pointer conn)
{
    if (state!=accepting_state_t::connected)
    {
        return false;
    }
    bool exp = false;
    if (accepting.compare_exchange_strong (exp, true))
    {
        auto &socket = conn->socket ();
        acceptor_.listen ();
        acceptor_.async_accept (socket,
                                [this, connection = std::move (conn)](const boost::system::error_code &error) {
                                    handle_accept (connection, error);
                                });
    }
    return true;
}

/** close the acceptor*/
void TcpAcceptor::close ()
{
    acceptor_.close ();
    state = accepting_state_t::halted;
	while (accepting)
	{
        std::this_thread::yield ();
	}
}

std::string TcpAcceptor::to_string () const
{
    auto str = endpoint_.address ().to_string ();
    str += ':';
    str += std::to_string (endpoint_.port ());
    return str;
}
void TcpAcceptor::handle_accept (TcpRxConnection::pointer new_connection, const boost::system::error_code &error)
{
    auto ptr = shared_from_this ();
    accepting = false;
    if (!error)
    {
        if (acceptCall)
        {
            acceptCall (std::move(ptr), std::move (new_connection));
        }
    }
    else if (error != boost::asio::error::operation_aborted)
    {
        if (errorCall)
        {
            errorCall (std::move (ptr), error);
        }
        else
        {
            std::cerr << " error in accept::" << error.message () << std::endl;
        }
    }
}

TcpServer::TcpServer (boost::asio::io_service &io_service,
                      const std::string &address,
                      int portNum,
                      int nominalBufferSize)
    : ioserv (io_service), bufferSize (nominalBufferSize)
{
    if ((address == "*")||(address=="tcp://*"))
    {
        acceptors.push_back (TcpAcceptor::create (ioserv, portNum));
    }
    else if (address == "localhost")
    {
        endpoints.push_back (boost::asio::ip::tcp::endpoint (boost::asio::ip::tcp::v4 (), portNum));
    }
    else
    {
        tcp::resolver resolver (io_service);
        tcp::resolver::query query (tcp::v4 (), address, std::to_string (portNum),
                                    tcp::resolver::query::canonical_name);
        tcp::resolver::iterator endpoint_iterator = resolver.resolve (query);
        tcp::resolver::iterator end;
        if (endpoint_iterator != end)
        {
            while (endpoint_iterator != end)
            {
                endpoints.push_back (*endpoint_iterator);
                ++endpoint_iterator;
            }
        }
        else
        {
            halted = true;
            return;
        }
    }
    initialConnect ();
}

TcpServer::TcpServer (boost::asio::io_service &io_service,
                      const std::string &address,
                      const std::string &port,
                      int nominalBufferSize)
    : ioserv (io_service), bufferSize (nominalBufferSize)
{
    tcp::resolver resolver (io_service);
    tcp::resolver::query query (tcp::v4 (), address, port, tcp::resolver::query::canonical_name);
    tcp::resolver::iterator endpoint_iterator = resolver.resolve (query);
    tcp::resolver::iterator end;
    if (endpoint_iterator != end)
    {
        while (endpoint_iterator != end)
        {
            endpoints.push_back (*endpoint_iterator);
            ++endpoint_iterator;
        }
    }
    else
    {
        halted = true;
        return;
    }
    initialConnect ();
}

TcpServer::TcpServer (boost::asio::io_service &io_service, int portNum, int nominalBufferSize)
    : ioserv (io_service), bufferSize (nominalBufferSize)
{
    endpoints.push_back (boost::asio::ip::tcp::endpoint (boost::asio::ip::tcp::v4 (), portNum));
    initialConnect ();
}

void TcpServer::initialConnect ()
{
    if (halted == true)
    {
        return;
    }
    for (auto &ep : endpoints)
    {
        auto acc = TcpAcceptor::create (ioserv, ep);

        acc->set_option (tcp::acceptor::reuse_address (false));
        acc->setAcceptCall (
          [this](TcpAcceptor::pointer accPtr, TcpRxConnection::pointer conn) { handle_accept (accPtr, conn); });
        acceptors.push_back (std::move (acc));
    }
	for (auto &acc : acceptors)
	{
        if (!acc->connect ())
        {
            halted = true;
            break;
        }
	}
}

bool TcpServer::reConnect (int timeout)
{
    halted = false;
    for (auto &acc : acceptors)
    {
        if (!acc->isConnected ())
        {
            if (!acc->connect (timeout))
            {
                std::cerr << "unable to connect on " << acc->to_string () << '\n';
                halted = true;
                break;
            }
        }
    }
    return !halted;
}

TcpServer::pointer TcpServer::create (boost::asio::io_service &io_service,
                                      const std::string &address,
                                      int PortNum,
                                      int nominalBufferSize)
{
    return pointer (new TcpServer (io_service, address, PortNum, nominalBufferSize));
}

TcpServer::pointer TcpServer::create (boost::asio::io_service &io_service,
                                      const std::string &address,
                                      const std::string &port,
                                      int nominalBufferSize)
{
    return pointer (new TcpServer (io_service, address, port, nominalBufferSize));
}

TcpServer::pointer TcpServer::create (boost::asio::io_service &io_service, int PortNum, int nominalBufferSize)
{
    return pointer (new TcpServer (io_service, PortNum, nominalBufferSize));
}

void TcpServer::start ()
{
    if (halted)
    {
        return;
    }
    bool exp = false;
    if (accepting.compare_exchange_strong (exp, true))
    {
        auto connects = connections.lock ();
        if (!connects->empty ())
        {
            for (auto &conn : *connects)
            {
                if (!conn->isReceiving ())
                {
                    conn->start ();
                }
            }
        }
        for (auto &acc : acceptors)
        {
            acc->start (TcpRxConnection::create (ioserv, bufferSize));
        }
    }
}

void TcpServer::handle_accept (TcpAcceptor::pointer acc, TcpRxConnection::pointer new_connection)
{
    // Set options here
    boost::asio::socket_base::linger optionLinger (true, 0);
    new_connection->socket ().set_option (optionLinger);

    new_connection->setDataCall (dataCall);
    new_connection->setErrorCall (errorCall);
    {  // scope for the connection lock
        auto connects = connections.lock ();
        //  new_connection->index = static_cast<int> (connects->size());
        // the previous 3 calls have to be made before this call since they could be used immediately
        new_connection->start ();
        connects->push_back (std::move (new_connection));
    }
    if (!halted)
    {
        acc->start (TcpRxConnection::create (ioserv, bufferSize));
    }
}

void TcpServer::close ()
{
    halted = true;
    accepting = false;
    //cancel first to give the threads some time to process
    for (auto &acc : acceptors)
    {
        acc->cancel();
    }
    //yeild the thread to allow some time for handles to process the cancellation
    std::this_thread::yield();
    for (auto &acc : acceptors)
    {
        acc->close ();
    }
    acceptors.clear ();
    auto connects = connections.lock ();
    for (auto &conn : *connects)
    {
        conn->close ();
    }
    connects->clear ();
}

}  // namespace tcp
}  // namespace helics
