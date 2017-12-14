/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/

#include "TcpHelperClasses.h"
#include <iostream>

using boost::asio::ip::tcp;

void tcp_rx_connection::start()
{
    socket_.async_receive(boost::asio::buffer(data), [connection = shared_from_this()](const boost::system::error_code&error, std::size_t bytes_transferred)
    {
        connection->handle_read(error, bytes_transferred);
    });
}

void tcp_rx_connection::handle_read(const boost::system::error_code &error,
    size_t bytes_transferred)
{
    if (!error)
    {
        dataCall(data.data(), bytes_transferred);
    }
    else
    {
        if (errorCall)
        {
            if (!errorCall(error))
            {
                return;
            }
        }
        else if ((error != boost::asio::error::eof)&&(error!=boost::asio::error::operation_aborted))
        {
            std::cerr << "receive error " << error.message() << std::endl;
            return;
        }
    }
    start();
}


tcp_connection::pointer tcp_connection::create(boost::asio::io_service& io_service, const std::string &connection, const std::string &port, size_t bufferSize)
{
    return pointer(new tcp_connection(io_service, connection,port, bufferSize));
}

tcp_connection::tcp_connection(boost::asio::io_service& io_service, const std::string &connection, const std::string &port, size_t bufferSize)
    : socket_(io_service), data(bufferSize)
{
    tcp::resolver resolver(io_service);
    tcp::resolver::query query(tcp::v4(),connection,port);
    tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
    socket_.async_connect(*endpoint_iterator, [this](const boost::system::error_code &error) {connect_handler(error); });
}


void tcp_connection::connect_handler(const boost::system::error_code &error)
{
    if (!error)
    {
        connected.store(true);
    }
}
void tcp_connection::send(const void *buffer, size_t dataLength)
{
    socket_.send(boost::asio::buffer(buffer, dataLength));
}

void tcp_connection::send(const std::string &dataString)
{
    socket_.send(boost::asio::buffer(dataString));
}

size_t tcp_connection::receive(void *buffer, size_t maxDataLength)
{
    return socket_.receive(boost::asio::buffer(buffer, maxDataLength));
}

void tcp_server::start_accept()
{
    tcp_rx_connection::pointer new_connection =
        tcp_rx_connection::create(acceptor_.get_io_service(), bufferSize);
    
    acceptor_.async_accept(new_connection->socket(), [this, new_connection](const boost::system::error_code& error)
    {
        handle_accept(new_connection, error);
    });
}

void tcp_server::handle_accept(tcp_rx_connection::pointer new_connection,
    const boost::system::error_code& error)
{
    if (!error)
    {
        new_connection->setDataCall(dataCall);
        new_connection->setErrorCall(errorCall);
        new_connection->start();
        start_accept();
        connections.push_back(new_connection);
    }
    else if (error!= boost::asio::error::operation_aborted)
    {
        std::cerr << " error in accept::" << error.message() << std::endl;
    }
}

void tcp_server::haltServer()
{
    acceptor_.cancel();
    for (auto &conn : connections)
    {
        conn->stop();
    }
}