/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef _HELICS_TCP_HELPER_CLASSES_
#define _HELICS_TCP_HELPER_CLASSES_
#pragma once


#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/io_service.hpp>
#include <memory>
#include <functional>
#include <string>

/** tcp socket generation for a receiving server*/
class tcp_rx_connection
    : public std::enable_shared_from_this<tcp_rx_connection>
{
public:
    typedef std::shared_ptr<tcp_rx_connection> pointer;

    static pointer create(boost::asio::io_service& io_service, size_t bufferSize)
    {
        return pointer(new tcp_rx_connection(io_service,bufferSize));
    }

    boost::asio::ip::tcp::socket& socket()
    {
        return socket_;
    }
    void start();
    void stop()
    {
        socket_.cancel();
    }

    void close();

    void setDataCall(std::function<size_t (tcp_rx_connection::pointer,const char *, size_t)> dataFunc)
    {
        dataCall = std::move(dataFunc);
    }
    void setErrorCall(std::function<bool(int, const boost::system::error_code&)> errorFunc)
    {
        errorCall = std::move(errorFunc);
    }
    
    int index = 0;
private:
    tcp_rx_connection(boost::asio::io_service& io_service, size_t bufferSize)
        : socket_(io_service), data(bufferSize)
    {
    }

    void handle_read(const boost::system::error_code &error,
        size_t bytes_transferred);

    boost::asio::ip::tcp::socket socket_;
    std::vector<char> data;
    std::function<size_t (tcp_rx_connection::pointer, const char *, size_t)> dataCall;
    std::function<bool(int index, const boost::system::error_code&)> errorCall;
};


/** tcp socket connection for connecting to a server*/
class tcp_connection
    : public std::enable_shared_from_this<tcp_connection>
{
public:
    typedef std::shared_ptr<tcp_connection> pointer;

    static pointer create(boost::asio::io_service& io_service, const std::string &connection, const std::string &port, size_t bufferSize = 10192);

    boost::asio::ip::tcp::socket& socket()
    {
        return socket_;
    }
    void cancel()
    {
        socket_.cancel();
    }
    /** close the socket connection*/
    void close();
    /** send raw data
    @throws boost::system::system_error on failure*/
    void send(const void *buffer, size_t dataLength);
    /** send a string
    @throws boost::system::system_error on failure*/
    void send(const std::string &dataString);
    /** do a blocking receive on the socket
    @throw boost::system::system_error on failure
    @return the number of bytes received
    */
    size_t receive(void *buffer, size_t maxDataSize);

    /**perform an asynchronous send operation
    @param buffer the data to send
    @param dataLength the length of the data
    @param callback a callback function of the form void handler(
  const boost::system::error_code& error, // Result of operation.
  std::size_t bytes_transferred           // Number of bytes received.
); 
*/
    template<typename Process>
    void send_async(const void *buffer, size_t dataLength, Process callback)
    {
        socket_.async_send(boost::asio::buffer(buffer, dataLength), callback);
    }

    /**perform an asynchronous receive operation
    @param buffer the data to send
    @param dataLength the length of the data
    @param callback a callback function of the form void handler(
    const boost::system::error_code& error, // Result of operation.
    std::size_t bytes_transferred           // Number of bytes received.
    );
    */
    template<typename Process>
    void async_receive(void *buffer, size_t dataLength, Process callback)
    {
        socket_.async_receive(boost::asio::buffer(buffer, dataLength), callback);
    }

    bool isConnected() const
    {
        return connected.load();
    }
private:
    tcp_connection(boost::asio::io_service& io_service, const std::string &connection, const std::string &port, size_t bufferSize);

    boost::asio::ip::tcp::socket socket_;
    std::atomic<bool> connected{ false };  //!< flag indicating connectivity
    std::vector<char> data;

    void connect_handler(const boost::system::error_code &error);

};

/** helper class for a server*/

class tcp_server
{
public:
    tcp_server(boost::asio::io_service& io_service, int PortNum, int nominalBufferSize=10192)
        : acceptor_(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), PortNum)),bufferSize(nominalBufferSize)
    {

    }
     
    void stop();

    void start();

    void close();

    void setDataCall(std::function<size_t(tcp_rx_connection::pointer, const char *, size_t)> dataFunc)
    {
        dataCall = std::move(dataFunc);
    }
    void setErrorCall(std::function<bool(int, const boost::system::error_code&)> errorFunc)
    {
        errorCall = std::move(errorFunc);
    }

private:
    void handle_accept(tcp_rx_connection::pointer new_connection,
        const boost::system::error_code& error);

    boost::asio::ip::tcp::acceptor acceptor_;
    
    size_t bufferSize;
    std::function<size_t(tcp_rx_connection::pointer, const char *, size_t)> dataCall;
    std::function<bool(int index, const boost::system::error_code& error)> errorCall;
    std::vector<std::shared_ptr<tcp_rx_connection>> connections;
};
#endif /* _HELICS_TCP_HELPER_CLASSES_*/