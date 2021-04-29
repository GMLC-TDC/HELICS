/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "../../common/GuardedTypes.hpp"
#include "gmlc/concurrency/TriggerVariable.hpp"

#include <asio/io_context.hpp>
#include <asio/ip/tcp.hpp>
#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <vector>

/** @file
various helper classes and functions for handling TCP connections
*/
namespace helics {
namespace tcp {
    /** tcp socket generation for a receiving server*/
    class TcpConnection: public std::enable_shared_from_this<TcpConnection> {
      public:
        /** enumeration of the possible states of a connection*/
        enum class connection_state_t {
            prestart = -1,
            waiting = 0,
            operating = 1,
            halted = 3,
            closed = 4,
        };

        using pointer = std::shared_ptr<TcpConnection>;
        static pointer create(asio::io_context& io_context,
                              const std::string& connection,
                              const std::string& port,
                              size_t bufferSize = 10192);
        /** create an RxConnection object using the specified context and bufferSize*/
        static pointer create(asio::io_context& io_context, size_t bufferSize)
        {
            return pointer(new TcpConnection(io_context, bufferSize));
        }
        /** get the underlying socket object*/
        asio::ip::tcp::socket& socket() { return socket_; }
        /** start the receiving loop*/
        void startReceive();
        /** cancel ongoing socket operations*/
        void cancel() { socket_.cancel(); }
        /** close the socket*/
        void close();
        /** perform the close actions but don't wait for them to be processed*/
        void closeNoWait();
        /** wait on the closing actions*/
        void waitOnClose();
        /**check if the connection is receiving data*/
        bool isReceiving() const { return receivingHalt.isActive(); }
        /** set the callback for the data object*/
        void setDataCall(
            std::function<size_t(TcpConnection::pointer, const char*, size_t)> dataFunc);
        /** set the callback for an error*/
        void setErrorCall(
            std::function<bool(TcpConnection::pointer, const std::error_code&)> errorFunc);
        /** set a logging function */
        void setLoggingFunction(
            std::function<void(int loglevel, const std::string& logMessage)> logFunc);
        /** send raw data
    @throws std::system_error on failure*/
        size_t send(const void* buffer, size_t dataLength);
        /** send a string
    @throws std::system_error on failure*/
        size_t send(const std::string& dataString);

        /** do a blocking receive on the socket
    @throw std::system_error on failure
    @return the number of bytes received
    */
        size_t receive(void* buffer, size_t maxDataSize);
        /**perform an asynchronous send operation
    @param buffer the data to send
    @param dataLength the length of the data
    @param callback a callback function of the form void handler(
    const std::error_code& error, // Result of operation.
    std::size_t bytes_transferred           // Number of bytes received.
    );
    */
        template<typename Process>
        void send_async(const void* buffer, size_t dataLength, Process callback)
        {
            socket_.async_send(asio::buffer(buffer, dataLength), callback);
        }
        /**perform an asynchronous receive operation
    @param buffer the data to send
    @param dataLength the length of the data
    @param callback a callback function of the form void handler(
    const std::error_code& error, // Result of operation.
    std::size_t bytes_transferred           // Number of bytes received.
    );
    */
        template<typename Process>
        void async_receive(void* buffer, size_t dataLength, Process callback)
        {
            socket_.async_receive(asio::buffer(buffer, dataLength), callback);
        }

        /**perform an asynchronous receive operation
   @param callback the callback function to execute when data has been received with signature
   void(TcpConnection::pointer, const char *buffer, size_t dataLength, const std::error_code &error)
    */
        void async_receive(std::function<void(TcpConnection::pointer,
                                              const char* buffer,
                                              size_t dataLength,
                                              const std::error_code& error)> callback)
        {
            socket_.async_receive(asio::buffer(data, data.size()),
                                  [connection = shared_from_this(),
                                   callback = std::move(callback)](const std::error_code& error,
                                                                   size_t bytes_transferred) {
                                      connection->handle_read(bytes_transferred, error, callback);
                                  });
        }
        /** check if the socket has finished the connection process*/
        bool isConnected() const
        {
            return (connected.isActive()) && (!connectionError.load(std::memory_order_acquire));
        }
        /** wait until the socket has finished the connection process
    @param timeOut the number of ms to wait for the connection process to finish (<0) for no limit
    @return true if connected, false if the timeout was reached
    */
        bool waitUntilConnected(std::chrono::milliseconds timeOut);
        /** get the id code for the socket*/
        int getIdentifier() const { return idcode; }

      private:
        TcpConnection(asio::io_context& io_context, size_t bufferSize):
            socket_(io_context), context_(io_context), data(bufferSize), idcode(idcounter++)
        {
        }
        TcpConnection(asio::io_context& io_context,
                      const std::string& connection,
                      const std::string& port,
                      size_t bufferSize);
        /** function for handling the asynchronous return from a read request*/
        void handle_read(const std::error_code& error, size_t bytes_transferred);
        void handle_read(
            size_t message_size,
            const std::error_code& error,
            std::function<
                void(TcpConnection::pointer, const char*, size_t, const std::error_code& error)>
                callback)
        {
            callback(shared_from_this(), data.data(), message_size, error);
        }
        static std::atomic<int> idcounter;

        std::atomic<size_t> residBufferSize{0};
        asio::ip::tcp::socket socket_;
        asio::io_context& context_;
        std::vector<char> data;
        std::atomic<bool> triggerhalt{false};
        const bool connecting{false};
        gmlc::concurrency::TriggerVariable receivingHalt;
        std::atomic<bool> connectionError{false};
        gmlc::concurrency::TriggerVariable connected;  //!< variable indicating connectivity
        std::function<size_t(TcpConnection::pointer, const char*, size_t)> dataCall;
        std::function<bool(TcpConnection::pointer, const std::error_code&)> errorCall;
        std::function<void(int level, const std::string& logMessage)> logFunction;
        std::atomic<connection_state_t> state{connection_state_t::prestart};
        const int idcode;
        void connect_handler(const std::error_code& error);
    };

    /** tcp acceptor*/
    class TcpAcceptor: public std::enable_shared_from_this<TcpAcceptor> {
      public:
        enum class accepting_state_t {
            opened = 0,
            connecting = 1,
            connected = 2,
            halted = 3,
            closed = 4,
        };
        using pointer = std::shared_ptr<TcpAcceptor>;
        /** create an RxConnection object using the specified context and bufferSize*/
        static pointer create(asio::io_context& io_context, asio::ip::tcp::endpoint& ep)
        {
            return pointer(new TcpAcceptor(io_context, ep));
        }

        static pointer create(asio::io_context& io_context, uint16_t port)
        {
            return pointer(new TcpAcceptor(io_context, port));
        }
        /** destructor to make sure everything is closed without threading issues*/
        ~TcpAcceptor()
        {
            try {
                close();
            }
            catch (...) {
            }
        }

        /** connect the acceptor to the socket*/
        bool connect();
        /** connect the acceptor to the socket if disconnected and try up to timeout*/
        bool connect(std::chrono::milliseconds timeOut);
        /** start the acceptor*/
        bool start(TcpConnection::pointer conn);
        /** cancel pending operations*/
        void cancel() { acceptor_.cancel(); }
        /** close the socket*/
        void close();
        /** check if the acceptor is current accepting new connections*/
        bool isAccepting() const { return accepting.isActive(); }
        /** check if the acceptor is ready to begin accepting*/
        bool isConnected() const { return (state.load() == accepting_state_t::connected); }
        /** set the callback for the data object*/
        void
            setAcceptCall(std::function<void(TcpAcceptor::pointer, TcpConnection::pointer)> accFunc)
        {
            acceptCall = std::move(accFunc);
        }

        /** set the error path callback*/
        void setErrorCall(
            std::function<bool(TcpAcceptor::pointer, const std::error_code&)> errorFunc)
        {
            errorCall = std::move(errorFunc);
        }
        /** set an option on the underlying acceptor*/
        template<class X>
        void set_option(const X& option)
        {
            acceptor_.set_option(option);
        }
        /** generate a string from the associated endpoint*/
        std::string to_string() const;

      private:
        TcpAcceptor(asio::io_context& io_context, asio::ip::tcp::endpoint& ep);
        TcpAcceptor(asio::io_context& io_context, uint16_t port);
        /** function for handling the asynchronous return from a read request*/
        void handle_accept(TcpAcceptor::pointer ptr,
                           TcpConnection::pointer new_connection,
                           const std::error_code& error);
        asio::ip::tcp::endpoint endpoint_;
        asio::ip::tcp::acceptor acceptor_;
        std::function<void(TcpAcceptor::pointer, TcpConnection::pointer)> acceptCall;
        std::function<bool(TcpAcceptor::pointer, const std::error_code&)> errorCall;
        std::atomic<accepting_state_t> state{accepting_state_t::opened};
        gmlc::concurrency::TriggerVariable accepting;
    };

    /** helper class for a server*/

    class TcpServer: public std::enable_shared_from_this<TcpServer> {
      public:
        using pointer = std::shared_ptr<TcpServer>;

        static pointer create(asio::io_context& io_context,
                              const std::string& address,
                              const std::string& port,
                              bool reuse_port = false,
                              int nominalBufferSize = 10192);

        static pointer create(asio::io_context& io_context,
                              const std::string& address,
                              uint16_t PortNum,
                              bool reuse_port = false,
                              int nominalBufferSize = 10192);
        static pointer
            create(asio::io_context& io_context, uint16_t PortNum, int nominalBufferSize = 10192);

      public:
        ~TcpServer();
        /**set the port reuse flag */
        void setPortReuse(bool reuse) { reuse_address = reuse; }
        /** start accepting new connections
    @return true if the start up was successful*/
        bool start();
        /** close the server*/
        void close();
        /** check if the server is ready to start*/
        bool isReady() const { return !(halted.load()); }
        /** reConnect the server with the same address*/
        bool reConnect(std::chrono::milliseconds timeOut);
        /** set the data callback*/
        void
            setDataCall(std::function<size_t(TcpConnection::pointer, const char*, size_t)> dataFunc)
        {
            dataCall = std::move(dataFunc);
        }
        /** set the error path callback*/
        void setErrorCall(
            std::function<bool(TcpConnection::pointer, const std::error_code&)> errorFunc)
        {
            errorCall = std::move(errorFunc);
        }
        void handle_accept(TcpAcceptor::pointer acc, TcpConnection::pointer new_connection);
        /** get a socket by it identification code*/
        TcpConnection::pointer findSocket(int connectorID) const;

      private:
        TcpServer(asio::io_context& io_context,
                  const std::string& address,
                  uint16_t portNum,
                  bool port_reuse,
                  int nominalBufferSize);
        TcpServer(asio::io_context& io_context,
                  const std::string& address,
                  const std::string& port,
                  bool port_reuse,
                  int nominalBufferSize);
        TcpServer(asio::io_context& io_context, uint16_t portNum, int nominalBufferSize);

        void initialConnect();
        asio::io_context& ioctx;
        mutable std::mutex accepting;
        std::vector<TcpAcceptor::pointer> acceptors;
        std::vector<asio::ip::tcp::endpoint> endpoints;
        size_t bufferSize;
        std::function<size_t(TcpConnection::pointer, const char*, size_t)> dataCall;
        std::function<bool(TcpConnection::pointer, const std::error_code& error)> errorCall;
        std::atomic<bool> halted{false};
        bool reuse_address = false;
        // this data structure is protected by the accepting mutex
        std::vector<TcpConnection::pointer> connections;
    };

}  // namespace tcp
}  // namespace helics
