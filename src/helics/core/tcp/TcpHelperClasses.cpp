/*
Copyright © 2017-2018, Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
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
    bool exp = false;
    if (receiving.compare_exchange_strong (exp, true))
    {
        socket_.async_receive (boost::asio::buffer (data.data () + residBufferSize,
                                                    data.size () - residBufferSize),
                               [connection = shared_from_this ()](const boost::system::error_code &error,
                                                                  size_t bytes_transferred) {
                                   connection->handle_read (error, bytes_transferred);
                               });
    }
}

void TcpRxConnection::handle_read (const boost::system::error_code &error, size_t bytes_transferred)
{
    if (disconnected)
    {
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
        receiving = false;
        start ();
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
        receiving = false;
        if (errorCall)
        {
            if (errorCall (shared_from_this (), error))
            {
                start ();
            }
        }
        else if ((error != boost::asio::error::eof) && (error != boost::asio::error::operation_aborted))
        {
            if (error != boost::asio::error::connection_reset)
            {
                std::cerr << "receive error " << error.message () << std::endl;
            }
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
    stop ();
    disconnected = true;
    boost::system::error_code ec;
    socket_.shutdown (boost::asio::ip::tcp::socket::shutdown_send, ec);
    if (ec!=nullptr)
    {
        std::cerr << "error occurred sending shutdown" << std::endl;
    }
    socket_.close ();
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
    if (ec!=nullptr)
    {
        // I don't know what to do with this, in practice this message is mostly spurious
        // but is seems I should do something with it, I just don't know what
        // std::cerr << "error occurred sending shutdown" << std::endl;
        ((void)(ec));
    }
    socket_.close ();
}

void TcpServer::start ()
{
    if (!connections.empty ())
    {
        for (auto &conn : connections)
        {
            conn->start ();
        }
    }
    TcpRxConnection::pointer new_connection = TcpRxConnection::create (acceptor_.get_io_service (), bufferSize);
    acceptor_.async_accept (new_connection->socket (),
                            [this, new_connection](const boost::system::error_code &error) {
                                handle_accept (new_connection, error);
                            });
}

void TcpServer::handle_accept (TcpRxConnection::pointer new_connection, const boost::system::error_code &error)
{
    if (!error)
    {
        new_connection->setDataCall (dataCall);
        new_connection->setErrorCall (errorCall);
        new_connection->index = static_cast<int> (connections.size ());
        // the previous 3 calls have to be made before this call since they could be used immediately
        new_connection->start ();
        connections.push_back (std::move (new_connection));
        start ();
    }
    else if (error != boost::asio::error::operation_aborted)
    {
        std::cerr << " error in accept::" << error.message () << std::endl;
    }
}

void TcpServer::stop ()
{
    acceptor_.cancel ();
    for (auto &conn : connections)
    {
        conn->stop ();
    }
}

void TcpServer::close ()
{
    acceptor_.cancel ();
    for (auto &conn : connections)
    {
        conn->close ();
    }
    connections.clear ();
}

} //namespace tcp
} // namespace helics

