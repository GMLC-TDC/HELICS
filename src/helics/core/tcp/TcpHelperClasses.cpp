/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/

#include "TcpHelperClasses.h"
#include <iostream>
#include <thread>

using boost::asio::ip::tcp;

void tcp_rx_connection::start ()
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

void tcp_rx_connection::handle_read (const boost::system::error_code &error, size_t bytes_transferred)
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

void tcp_rx_connection::send (const void *buffer, size_t dataLength)
{
    auto sz = socket_.send (boost::asio::buffer (buffer, dataLength));
    assert (sz == dataLength);
}

void tcp_rx_connection::send (const std::string &dataString)
{
    auto sz = socket_.send (boost::asio::buffer (dataString));
    assert (sz == dataString.size ());
}

void tcp_rx_connection::close ()
{
    stop ();
    disconnected = true;
    boost::system::error_code ec;
    socket_.shutdown (boost::asio::ip::tcp::socket::shutdown_send, ec);
    if (ec)
    {
        std::cerr << "error occurred sending shutdown" << std::endl;
    }
    socket_.close ();
}

tcp_connection::pointer tcp_connection::create (boost::asio::io_service &io_service,
                                                const std::string &connection,
                                                const std::string &port,
                                                size_t bufferSize)
{
    return pointer (new tcp_connection (io_service, connection, port, bufferSize));
}

tcp_connection::tcp_connection (boost::asio::io_service &io_service,
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

void tcp_connection::connect_handler (const boost::system::error_code &error)
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
void tcp_connection::send (const void *buffer, size_t dataLength)
{
    if (!isConnected())
    {
        waitUntilConnected(200);
    }
    auto sz = socket_.send (boost::asio::buffer (buffer, dataLength));
    assert (sz == dataLength);
}

void tcp_connection::send (const std::string &dataString)
{
    if (!isConnected())
    {
        waitUntilConnected(200);
    }
    auto sz = socket_.send (boost::asio::buffer (dataString));
    assert (sz == dataString.size ());
}

size_t tcp_connection::receive (void *buffer, size_t maxDataLength)
{
    return socket_.receive (boost::asio::buffer (buffer, maxDataLength));
}

int tcp_connection::waitUntilConnected (int timeOut)
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

void tcp_connection::close ()
{
    cancel ();
    boost::system::error_code ec;
    socket_.shutdown (boost::asio::ip::tcp::socket::shutdown_send, ec);
    if (ec)
    {
        std::cerr << "error occurred sending shutdown" << std::endl;
    }
    socket_.close ();
}

void tcp_server::start ()
{
    if (!connections.empty ())
    {
        for (auto &conn : connections)
        {
            conn->start ();
        }
    }
    tcp_rx_connection::pointer new_connection =
      tcp_rx_connection::create (acceptor_.get_io_service (), bufferSize);
    acceptor_.async_accept (new_connection->socket (),
                            [this, new_connection](const boost::system::error_code &error) {
                                handle_accept (new_connection, error);
                            });
}

void tcp_server::handle_accept (tcp_rx_connection::pointer new_connection, const boost::system::error_code &error)
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

void tcp_server::stop ()
{
    acceptor_.cancel ();
    for (auto &conn : connections)
    {
        conn->stop ();
    }
}

void tcp_server::close ()
{
    acceptor_.cancel();
    for (auto &conn : connections)
    {
        conn->close();
    }
    connections.clear ();
}