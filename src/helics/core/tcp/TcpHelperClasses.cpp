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
using namespace std::literals::chrono_literals;
void TcpConnection::start ()
{
    if (triggerhalt)
    {
        receivingHalt.trigger ();
        return;
    }
    if (state == connection_state_t::prestart)
    {
        receivingHalt.activate ();
        connected.activate ();
        state = connection_state_t::halted;
    }
    connection_state_t exp = connection_state_t::halted;
    if (state.compare_exchange_strong (exp, connection_state_t::operating))
    {
        if (!receivingHalt.isActive ())
        {
            receivingHalt.activate ();
        }
        if (!triggerhalt)
        {
            socket_.async_receive (boost::asio::buffer (data.data () + residBufferSize,
                                                        data.size () - residBufferSize),
                                   [this](const boost::system::error_code &error, size_t bytes_transferred) {
                                       handle_read (error, bytes_transferred);
                                   });
        }
        else
        {
            receivingHalt.trigger ();
        }
    }
    else if (exp != connection_state_t::operating)
    {
        receivingHalt.trigger ();
    }
}

void TcpConnection::setDataCall (std::function<size_t (TcpConnection::pointer, const char *, size_t)> dataFunc)
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
void TcpConnection::setErrorCall (
  std::function<bool(TcpConnection::pointer, const boost::system::error_code &)> errorFunc)
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

void TcpConnection::handle_read (const boost::system::error_code &error, size_t bytes_transferred)
{
    if (triggerhalt)
    {
        state = connection_state_t::halted;
        receivingHalt.trigger ();
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
        receivingHalt.trigger ();
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
                state = connection_state_t::halted;
                receivingHalt.trigger ();
            }
        }
        else if ((error != boost::asio::error::eof) && (error != boost::asio::error::operation_aborted))
        {
            if (error != boost::asio::error::connection_reset)
            {
                std::cerr << "receive error " << error.message () << std::endl;
            }
            state = connection_state_t::halted;
            receivingHalt.trigger ();
        }
        else
        {
            state = connection_state_t::halted;
            receivingHalt.trigger ();
        }
    }
}

// boost::asio::socket_base::linger optionLinger(true, 2);
// socket_.set_option(optionLinger, ec);
void TcpConnection::close ()
{
    triggerhalt = true;
    state = connection_state_t::closed;
    boost::system::error_code ec;
    if (socket_.is_open ())
    {
        socket_.shutdown (boost::asio::ip::tcp::socket::shutdown_both, ec);
        if (ec)
        {
            std::cerr << "error occurred sending shutdown::" << ec << std::endl;
            ec.clear ();
        }
        socket_.close (ec);
        receivingHalt.wait ();
    }
    else
    {
        if (receivingHalt.isActive ())
        {
            socket_.close (ec);
            receivingHalt.wait ();
        }
    }
}

void TcpConnection::closeNoWait ()
{
    triggerhalt = true;
    state = connection_state_t::closed;
    boost::system::error_code ec;
    if (socket_.is_open ())
    {
        socket_.shutdown (boost::asio::ip::tcp::socket::shutdown_both, ec);
        if (ec)
        {
            std::cerr << "error occurred sending shutdown::" << ec << std::endl;
            ec.clear ();
        }
        socket_.close (ec);
    }
    else
    {
        if (receivingHalt.isActive ())
        {
            socket_.close (ec);
        }
    }
}

/** wait on the closing actions*/
void TcpConnection::waitOnClose ()
{
    if (triggerhalt)
    {
        receivingHalt.wait ();
    }
    else
    {
        close ();
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
        connected.activate ();
    }
    else
    {
        std::cerr << "connection error " << error.message () << ": code =" << error.value () << '\n';
        connectionError = true;
        connected.activate ();
    }
}
size_t TcpConnection::send (const void *buffer, size_t dataLength)
{
    if (!isConnected ())
    {
        if (!waitUntilConnected (300ms))
        {
            std::cerr << "connection timeout waiting again" << std::endl;
        }
        if (!waitUntilConnected (200ms))
        {
            std::cerr << "connection timeout twice, now returning" << std::endl;
            return 0;
        }
    }
    auto sz = socket_.send (boost::asio::buffer (buffer, dataLength));
    assert (sz == dataLength);
    return sz;
}

size_t TcpConnection::send (const std::string &dataString)
{
    if (!isConnected ())
    {
        if (!waitUntilConnected (300ms))
        {
            std::cerr << "connection timeout waiting again" << std::endl;
        }
        if (!waitUntilConnected (200ms))
        {
            std::cerr << "connection timeout twice, now returning" << std::endl;
            return 0;
        }
    }
    auto sz = socket_.send (boost::asio::buffer (dataString));
    assert (sz == dataString.size ());
    return sz;
}

size_t TcpConnection::receive (void *buffer, size_t maxDataLength)
{
    return socket_.receive (boost::asio::buffer (buffer, maxDataLength));
}

bool TcpConnection::waitUntilConnected (std::chrono::milliseconds timeOut)
{
    if (isConnected ())
    {
        return true;
    }
    if (timeOut < 0ms)
    {
        connected.waitActivation ();
        return isConnected();
    }
    else
    {
        connected.wait_forActivation (timeOut);
        return isConnected ();
    }
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
bool TcpAcceptor::start (TcpConnection::pointer conn)
{
    if (!conn)
    {
        if (accepting.isActive ())
        {
            accepting.trigger ();
        }
        return false;
    }
    if (state != accepting_state_t::connected)
    {
        conn->close ();
        if (accepting.isActive ())
        {
            accepting.trigger ();
        }
        return false;
    }
    if (accepting.activate ())
    {
        auto &socket = conn->socket ();
        acceptor_.listen ();
        auto ptr = shared_from_this ();
        acceptor_.async_accept (socket, [this, apointer = std::move (ptr),
                                         connection = std::move (conn)](const boost::system::error_code &error) {
            handle_accept (apointer, connection, error);
        });
        return true;
    }
    else
    {
        conn->close ();
        return false;
    }
}

/** close the acceptor*/
void TcpAcceptor::close ()
{
    state = accepting_state_t::halted;
    acceptor_.close ();
    accepting.wait ();
}

std::string TcpAcceptor::to_string () const
{
    auto str = endpoint_.address ().to_string ();
    str += ':';
    str += std::to_string (endpoint_.port ());
    return str;
}
void TcpAcceptor::handle_accept (TcpAcceptor::pointer ptr,
                                 TcpConnection::pointer new_connection,
                                 const boost::system::error_code &error)
{
    if (state != accepting_state_t::connected)
    {
        boost::asio::socket_base::linger optionLinger (true, 0);
        try
        {
            new_connection->socket ().set_option (optionLinger);
        }
        catch (...)
        {
        }
        new_connection->close ();
        accepting.reset ();
        return;
    }
    if (!error)
    {
        if (acceptCall)
        {
            accepting.reset ();
            acceptCall (std::move (ptr), std::move (new_connection));
            if (!accepting.isActive ())
            {
                accepting.trigger ();
            }
        }
        else
        {
            boost::asio::socket_base::linger optionLinger (true, 0);
            try
            {
                new_connection->socket ().set_option (optionLinger);
            }
            catch (...)
            {
            }
            new_connection->close ();
            accepting.reset ();
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
        boost::asio::socket_base::linger optionLinger (true, 0);
        try
        {
            new_connection->socket ().set_option (optionLinger);
        }
        catch (...)
        {
        }
        new_connection->close ();
        accepting.reset ();
    }
    else
    {
        new_connection->close ();
        accepting.reset ();
    }
}

TcpServer::TcpServer (boost::asio::io_service &io_service,
                      const std::string &address,
                      int portNum,
                      bool port_reuse,
                      int nominalBufferSize)
    : ioserv (io_service), bufferSize (nominalBufferSize), reuse_address (port_reuse)
{
    if ((address == "*") || (address == "tcp://*"))
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
                      bool port_reuse,
                      int nominalBufferSize)
    : ioserv (io_service), bufferSize (nominalBufferSize), reuse_address (port_reuse)
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

TcpServer::~TcpServer () { close (); }

void TcpServer::initialConnect ()
{
    if (halted == true)
    {
        return;
    }
    for (auto &ep : endpoints)
    {
        auto acc = TcpAcceptor::create (ioserv, ep);
        if (reuse_address)
        {
            acc->set_option (tcp::acceptor::reuse_address (true));
        }
        else
        {
            acc->set_option (tcp::acceptor::reuse_address (false));
        }
        acc->setAcceptCall (
          [this](TcpAcceptor::pointer accPtr, TcpConnection::pointer conn) { handle_accept (accPtr, conn); });
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
    bool partialConnect = false;
    for (auto &acc : acceptors)
    {
        if (!acc->isConnected ())
        {
            if (!acc->connect (timeout))
            {
                if (partialConnect)
                {
                    std::cerr << "unable to connect all acceptors on " << acc->to_string () << '\n';
                }
                else
                {
                    std::cerr << "unable to connect on " << acc->to_string () << '\n';
                }

                halted = true;
                break;
            }
        }
        partialConnect = true;
    }
    return !halted;
}

TcpServer::pointer TcpServer::create (boost::asio::io_service &io_service,
                                      const std::string &address,
                                      int PortNum,
                                      bool port_reuse,
                                      int nominalBufferSize)
{
    return pointer (new TcpServer (io_service, address, PortNum, port_reuse, nominalBufferSize));
}

TcpServer::pointer TcpServer::create (boost::asio::io_service &io_service,
                                      const std::string &address,
                                      const std::string &port,
                                      bool port_reuse,
                                      int nominalBufferSize)
{
    return pointer (new TcpServer (io_service, address, port, port_reuse, nominalBufferSize));
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

    if (!halted)
    {
        {  // scope for the lock_guard
            std::lock_guard<std::mutex> lock (accepting);
            if (!connections.empty ())
            {
                for (auto &conn : connections)
                {
                    if (!conn->isReceiving ())
                    {
                        conn->start ();
                    }
                }
            }
        }

        for (auto &acc : acceptors)
        {
            acc->start (TcpConnection::create (ioserv, bufferSize));
        }
    }
}

void TcpServer::handle_accept (TcpAcceptor::pointer acc, TcpConnection::pointer new_connection)
{
    /*setting linger to 1 second*/
    boost::asio::socket_base::linger optionLinger (true, 0);
    new_connection->socket ().set_option (optionLinger);
    // Set options here
    if (halted)
    {
        new_connection->close ();
        return;
    }

    if (!halted)
    {
        new_connection->setDataCall (dataCall);
        new_connection->setErrorCall (errorCall);
        new_connection->start ();
        {  // scope for the lock_guard

            std::unique_lock<std::mutex> lock (accepting);
            if (!halted)
            {
                connections.push_back (std::move (new_connection));
            }
            else
            {
                lock.unlock ();
                new_connection->close ();
                return;
            }
        }
        acc->start (TcpConnection::create (ioserv, bufferSize));
    }
    else
    {
        new_connection->close ();
    }
}

void TcpServer::close ()
{
    halted = true;
    if (acceptors.size () == 1)
    {
        acceptors[0]->close ();
    }
    else if (!acceptors.empty ())
    {
        // cancel first to give the threads some time to process
        for (auto &acc : acceptors)
        {
            acc->cancel ();
        }
        for (auto &acc : acceptors)
        {
            acc->close ();
        }
    }

    acceptors.clear ();
    std::unique_lock<std::mutex> lock (accepting);
    auto sz = connections.size ();
    lock.unlock ();
    for (decltype (sz) ii = 0; ii < sz; ++ii)
    {
        connections[ii]->closeNoWait ();
    }
    for (decltype (sz) ii = 0; ii < sz; ++ii)
    {
        connections[ii]->waitOnClose ();
    }
    connections.clear ();
}

}  // namespace tcp
}  // namespace helics
