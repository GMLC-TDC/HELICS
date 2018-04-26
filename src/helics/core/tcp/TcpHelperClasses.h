/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once

#include <functional>
#include <memory>
#include <string>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include "../../common/GuardedTypes.hpp"

namespace helics
{
namespace tcp
{
/** tcp socket generation for a receiving server*/
class TcpRxConnection : public std::enable_shared_from_this<TcpRxConnection>
{
  public:
      enum class connection_state_t
      {
          prestart = -1,
          halted = 0,
          receiving=1,
          closed=2,
      };

    typedef std::shared_ptr<TcpRxConnection> pointer;
    /** create an RxConnection object using the specified service and bufferSize*/
    static pointer create (boost::asio::io_service &io_service, size_t bufferSize)
    {
        return pointer (new TcpRxConnection (io_service, bufferSize));
    }
    /** get the underlying socket object*/
    boost::asio::ip::tcp::socket &socket () { return socket_; }
    /** start the receiving loop*/
    void start();
    /** close the socket*/
    void close ();
    bool isReceiving() const
    {
        return receiving.load();
    }
    /** set the callback for the data object*/
    void setDataCall(std::function<size_t(TcpRxConnection::pointer, const char *, size_t)> dataFunc);
    /** set the callback for an error*/
    void setErrorCall(std::function<bool(TcpRxConnection::pointer, const boost::system::error_code &)> errorFunc);

    /** send raw data
    @throws boost::system::system_error on failure*/
    void send (const void *buffer, size_t dataLength);
    /** send a string
    @throws boost::system::system_error on failure*/
    void send (const std::string &dataString);

    //int index = 0;

  private:
    TcpRxConnection (boost::asio::io_service &io_service, size_t bufferSize)
        : socket_ (io_service), data (bufferSize)
    {
    }
    /** function for handling the asynchronous return from a read request*/
    void handle_read (const boost::system::error_code &error, size_t bytes_transferred);
    std::atomic<size_t> residBufferSize{0};
    boost::asio::ip::tcp::socket socket_;
    std::vector<char> data;
    std::atomic<bool> triggerhalt{ false };
    std::atomic<bool> receiving{ false };
    std::function<size_t (TcpRxConnection::pointer, const char *, size_t)> dataCall;
    std::function<bool(TcpRxConnection::pointer, const boost::system::error_code &)> errorCall;
    std::atomic<connection_state_t> state{connection_state_t::prestart};
};

/** tcp socket connection for connecting to a server*/
class TcpConnection : public std::enable_shared_from_this<TcpConnection>
{
  public:
    typedef std::shared_ptr<TcpConnection> pointer;

    static pointer create (boost::asio::io_service &io_service,
                           const std::string &connection,
                           const std::string &port,
                           size_t bufferSize = 10192);

    boost::asio::ip::tcp::socket &socket () { return socket_; }
    void cancel () { socket_.cancel (); }
    /** close the socket connection
    @param*/
    void close ();
    /** send raw data
    @throws boost::system::system_error on failure*/
    void send (const void *buffer, size_t dataLength);
    /** send a string
    @throws boost::system::system_error on failure*/
    void send (const std::string &dataString);
    /** do a blocking receive on the socket
    @throw boost::system::system_error on failure
    @return the number of bytes received
    */
    size_t receive (void *buffer, size_t maxDataSize);

    /**perform an asynchronous send operation
    @param buffer the data to send
    @param dataLength the length of the data
    @param callback a callback function of the form void handler(
  const boost::system::error_code& error, // Result of operation.
  std::size_t bytes_transferred           // Number of bytes received.
);
*/
    template <typename Process>
    void send_async (const void *buffer, size_t dataLength, Process callback)
    {
        socket_.async_send (boost::asio::buffer (buffer, dataLength), callback);
    }

    /**perform an asynchronous receive operation
    @param buffer the data to send
    @param dataLength the length of the data
    @param callback a callback function of the form void handler(
    const boost::system::error_code& error, // Result of operation.
    std::size_t bytes_transferred           // Number of bytes received.
    );
    */
    template <typename Process>
    void async_receive (void *buffer, size_t dataLength, Process callback)
    {
        socket_.async_receive (boost::asio::buffer (buffer, dataLength), callback);
    }

    /**perform an asynchronous receive operation
    @param buffer the data to send
    @param dataLength the length of the data
    @param callback a callback function of the form void handler(
    const boost::system::error_code& error, // Result of operation.
    std::size_t bytes_transferred           // Number of bytes received.
    );
    */
    void async_receive (
      std::function<void(TcpConnection::pointer, const char *, size_t, const boost::system::error_code &error)>
        callback)
    {
        socket_.async_receive (boost::asio::buffer (data, data.size ()),
                               [connection = shared_from_this (), callback](const boost::system::error_code &error,
                                                                            size_t bytes_transferred) {
                                   connection->handle_read (bytes_transferred, error, callback);
                               });
    }

  private:
    void handle_read (
      size_t message_size,
      const boost::system::error_code &error,
      std::function<void(TcpConnection::pointer, const char *, size_t, const boost::system::error_code &error)>
        callback)
    {
        callback (shared_from_this (), data.data (), message_size, error);
    }

  public:
    /** check if the socket has finished the connection process*/
    bool isConnected () const { return connected.load (); }
    /** wait until the socket has finished the connection process
    @param timeOut the number of ms to wait for the connection process to finish (-1) for no limit
    @return 0 if connected, -1 if the timeout was reached, -2 if error
    */
    int waitUntilConnected (int timeOut);

  private:
    TcpConnection (boost::asio::io_service &io_service,
                   const std::string &connection,
                   const std::string &port,
                   size_t bufferSize);

    boost::asio::ip::tcp::socket socket_;
    std::atomic<bool> connected{false};  //!< flag indicating connectivity
    std::vector<char> data;

    void connect_handler (const boost::system::error_code &error);
};

/** helper class for a server*/

class TcpServer : public std::enable_shared_from_this<TcpServer>
{
  public:
    typedef std::shared_ptr<TcpServer> pointer;

    static pointer create (boost::asio::io_service &io_service, int PortNum, int nominalBufferSize = 10192);

  public:
      /** start accepting new connections*/
    void start ();
    /** close the server*/
    void close ();

    void setDataCall (std::function<size_t (TcpRxConnection::pointer, const char *, size_t)> dataFunc)
    {
        dataCall = std::move (dataFunc);
    }
    void setErrorCall (std::function<bool(TcpRxConnection::pointer, const boost::system::error_code &)> errorFunc)
    {
        errorCall = std::move (errorFunc);
    }
    void handle_accept (TcpRxConnection::pointer new_connection, const boost::system::error_code &error);

  private:
    TcpServer (boost::asio::io_service &io_service, int PortNum, int nominalBufferSize)
        : acceptor_ (io_service, boost::asio::ip::tcp::endpoint (boost::asio::ip::tcp::v4 (), PortNum)),
          bufferSize (nominalBufferSize)
    {
    }

    boost::asio::ip::tcp::acceptor acceptor_;
    std::atomic<bool> accepting{false};
    size_t bufferSize;
    std::function<size_t (TcpRxConnection::pointer, const char *, size_t)> dataCall;
    std::function<bool(TcpRxConnection::pointer, const boost::system::error_code &error)> errorCall;
    std::atomic<bool> halted{ false };
    guarded<std::vector<std::shared_ptr<TcpRxConnection>>> connections;
};

}  // namespace tcp
}  // namespace helics
