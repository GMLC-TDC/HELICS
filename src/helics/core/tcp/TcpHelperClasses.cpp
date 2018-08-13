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

TcpServer::TcpServer (boost::asio::io_service &io_service,
                      const std::string &address,
                      int portNum,
                      int nominalBufferSize)
    : acceptor_ (io_service), bufferSize (nominalBufferSize)
{
    if (address == "localhost")
    {
        ep = boost::asio::ip::tcp::endpoint (boost::asio::ip::tcp::v4 (), portNum);
    }
    else
    {
        tcp::resolver resolver (io_service);
        tcp::resolver::query query (tcp::v4 (), address, std::to_string (portNum),
                                    tcp::resolver::query::canonical_name);
        ep = *resolver.resolve (query).begin ();
    }
    initialConnect ();
}

TcpServer::TcpServer (boost::asio::io_service &io_service,
                      const std::string &address,
                      const std::string &port,
                      int nominalBufferSize)
    : acceptor_ (io_service), bufferSize (nominalBufferSize)
{
        tcp::resolver resolver (io_service);
        tcp::resolver::query query (tcp::v4 (), address, port,
                                    tcp::resolver::query::canonical_name);
        ep = *resolver.resolve (query).begin ();
    initialConnect ();
}

TcpServer::TcpServer (boost::asio::io_service &io_service,
                      int portNum,
                      int nominalBufferSize)
    : acceptor_ (io_service), bufferSize (nominalBufferSize)
{
    ep = boost::asio::ip::tcp::endpoint (boost::asio::ip::tcp::v4 (), portNum);
    initialConnect ();
}


void TcpServer::initialConnect ()
{
    acceptor_.open (ep.protocol ());
    acceptor_.set_option (tcp::acceptor::reuse_address (false));
    boost::system::error_code ec;
    acceptor_.bind (ep, ec);
    if (ec)
    {
        halted = true;
    }
}

bool TcpServer::reConnect(int timeout)
{
    if (!halted)
    {
        close ();
    }

    boost::system::error_code ec;
    bool bindsuccess = false;
    int tcount = 0;
    while (!bindsuccess)
    {
        acceptor_.bind (ep, ec);
        if (ec)
        {
            if (tcount > timeout)
            {
                break;
            }
            std::this_thread::sleep_for (std::chrono::milliseconds (200));
            tcount += 200;
        }
        else
        {
            bindsuccess = true;
        }
    }

    halted = !bindsuccess;
    return bindsuccess;
}

bool TcpServer::reConnect (const std::string &address, const std::string &port, int timeout)
{
    if (!address.empty ())
    {
        tcp::resolver resolver (acceptor_.get_io_service ());
        tcp::resolver::query query (tcp::v4 (), address, port, tcp::resolver::query::canonical_name);
        ep = *resolver.resolve (query).begin ();
    }
    return reConnect (timeout);
}

TcpServer::pointer TcpServer::create (boost::asio::io_service &io_service,
                                      const std::string &address,
                                      int PortNum,
                                      int nominalBufferSize)
{
    return pointer (new TcpServer (io_service, address, PortNum, nominalBufferSize));
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
        TcpRxConnection::pointer new_connection =
          TcpRxConnection::create (acceptor_.get_io_service (), bufferSize);
        auto &socket = new_connection->socket ();
        acceptor_.listen ();
        acceptor_.async_accept (socket,
                                [this, connection = std::move (new_connection)](
                                  const boost::system::error_code &error) { handle_accept (connection, error); });
    }
}

void TcpServer::handle_accept (TcpRxConnection::pointer new_connection, const boost::system::error_code &error)
{
    if (!error)
    {
        new_connection->setDataCall (dataCall);
        new_connection->setErrorCall (errorCall);
        {  // scope for the connection lock
            auto connects = connections.lock ();
            //  new_connection->index = static_cast<int> (connects->size());
            // the previous 3 calls have to be made before this call since they could be used immediately
            new_connection->start ();
            connects->push_back (std::move (new_connection));
        }
        accepting = false;
        if (!halted)
        {
            start ();
        }
    }
    else if (error != boost::asio::error::operation_aborted)
    {
        std::cerr << " error in accept::" << error.message () << std::endl;
        accepting = false;
    }
    else
    {
        accepting = false;
    }
}

void TcpServer::close ()
{
    halted = true;
    acceptor_.close ();
    auto connects = connections.lock ();
    for (auto &conn : *connects)
    {
        conn->close ();
    }
    while (accepting)
    {
        std::this_thread::yield ();
    }
    connects->clear ();
}

}  // namespace tcp
}  // namespace helics
