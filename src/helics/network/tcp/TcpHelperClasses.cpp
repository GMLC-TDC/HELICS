/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "TcpHelperClasses.h"

#include <algorithm>
#include <iostream>
#include <thread>
#include <utility>

namespace helics {
namespace tcp {
    using asio::ip::tcp;
    using namespace std::chrono_literals;  // NOLINT

    std::atomic<int> TcpConnection::idcounter{10};

    void TcpConnection::startReceive()
    {
        if (triggerhalt) {
            receivingHalt.trigger();
            return;
        }
        if (state == connection_state_t::prestart) {
            receivingHalt.activate();
            connected.activate();
            state = connection_state_t::waiting;
        }
        connection_state_t exp = connection_state_t::waiting;
        if (state.compare_exchange_strong(exp, connection_state_t::operating)) {
            if (!receivingHalt.isActive()) {
                receivingHalt.activate();
            }
            if (!triggerhalt) {
                socket_.async_receive(asio::buffer(data.data() + residBufferSize,
                                                   data.size() - residBufferSize),
                                      [ptr = shared_from_this()](const std::error_code& err,
                                                                 size_t bytes) {
                                          ptr->handle_read(err, bytes);
                                      });
                if (triggerhalt) {
                    // cancel previous operation if triggerhalt is now active
                    socket_.cancel();
                    // receivingHalt.trigger();
                }
            } else {
                state = connection_state_t::halted;
                receivingHalt.trigger();
            }
        } else if (exp != connection_state_t::operating) {
            /*either halted or closed*/
            receivingHalt.trigger();
        }
    }

    void TcpConnection::setDataCall(
        std::function<size_t(TcpConnection::pointer, const char*, size_t)> dataFunc)
    {
        if (state.load() == connection_state_t::prestart) {
            dataCall = std::move(dataFunc);
        } else {
            throw(std::runtime_error("cannot set data callback after socket is started"));
        }
    }
    void TcpConnection::setErrorCall(
        std::function<bool(TcpConnection::pointer, const std::error_code&)> errorFunc)
    {
        if (state.load() == connection_state_t::prestart) {
            errorCall = std::move(errorFunc);
        } else {
            throw(std::runtime_error("cannot set error callback after socket is started"));
        }
    }

    void TcpConnection::setLoggingFunction(
        std::function<void(int loglevel, const std::string& logMessage)> logFunc)
    {
        if (state.load() == connection_state_t::prestart) {
            logFunction = std::move(logFunc);
        } else {
            throw(std::runtime_error("cannot set logging function after socket is started"));
        }
    }

    void TcpConnection::handle_read(const std::error_code& error, size_t bytes_transferred)
    {
        if (triggerhalt.load(std::memory_order_acquire)) {
            state = connection_state_t::halted;
            receivingHalt.trigger();
            return;
        }
        if (!error) {
            auto used =
                dataCall(shared_from_this(), data.data(), bytes_transferred + residBufferSize);
            if (used < (bytes_transferred + residBufferSize)) {
                if (used > 0) {
                    std::copy(data.data() + used,
                              data.data() + bytes_transferred + residBufferSize,
                              data.data());
                }
                residBufferSize = bytes_transferred + residBufferSize - used;
            } else {
                residBufferSize = 0;
                data.assign(data.size(), 0);
            }
            state = connection_state_t::waiting;
            startReceive();
        } else if (error == asio::error::operation_aborted) {
            state = connection_state_t::halted;
            receivingHalt.trigger();
            return;
        } else {
            // there was an error
            if (bytes_transferred > 0) {
                auto used =
                    dataCall(shared_from_this(), data.data(), bytes_transferred + residBufferSize);
                if (used < (bytes_transferred + residBufferSize)) {
                    if (used > 0) {
                        std::copy(data.data() + used,
                                  data.data() + bytes_transferred + residBufferSize,
                                  data.data());
                    }
                    residBufferSize = bytes_transferred + residBufferSize - used;
                } else {
                    residBufferSize = 0;
                }
            }
            if (errorCall) {
                if (errorCall(shared_from_this(), error)) {
                    state = connection_state_t::waiting;
                    startReceive();
                } else {
                    state = connection_state_t::halted;
                    receivingHalt.trigger();
                }
            } else if (error != asio::error::eof) {
                if (error != asio::error::connection_reset) {
                    std::cerr << "receive error " << error.message() << std::endl;
                }
                state = connection_state_t::halted;
                receivingHalt.trigger();
            } else {
                state = connection_state_t::halted;
                receivingHalt.trigger();
            }
        }
    }

    // asio::socket_base::linger optionLinger(true, 2);
    // socket_.set_option(optionLinger, ec);
    void TcpConnection::close()
    {
        closeNoWait();
        waitOnClose();
    }

    void TcpConnection::closeNoWait()
    {
        triggerhalt.store(true);
        switch (state.load()) {
            case connection_state_t::prestart:
                if (receivingHalt.isActive()) {
                    receivingHalt.trigger();
                }
                break;
            case connection_state_t::halted:
            case connection_state_t::closed:
                receivingHalt.trigger();
                break;
            default:
                break;
        }

        std::error_code ec;
        if (socket_.is_open()) {
            socket_.shutdown(tcp::socket::shutdown_both, ec);
            if (ec) {
                if ((ec.value() != asio::error::not_connected) &&
                    (ec.value() != asio::error::connection_reset)) {
                    std::cerr << "error occurred sending shutdown::" << ec.message() << " "
                              << ec.value() << std::endl;
                }
                ec.clear();
            }
            socket_.close(ec);
        } else {
            socket_.close(ec);
        }
    }

    /** wait on the closing actions*/
    void TcpConnection::waitOnClose()
    {
        if (triggerhalt.load(std::memory_order_acquire)) {
            if (connecting) {
                connected.waitActivation();
            }

            while (!receivingHalt.wait_for(std::chrono::milliseconds(200))) {
                std::cout << "wait timeout " << static_cast<int>(state.load()) << " "
                          << socket_.is_open() << " " << receivingHalt.isTriggered() << std::endl;

                std::cout << "wait info " << context_.stopped() << " " << connecting << std::endl;
            }
        } else {
            close();
        }
        state.store(connection_state_t::closed);
    }

    TcpConnection::pointer TcpConnection::create(asio::io_context& io_context,
                                                 const std::string& connection,
                                                 const std::string& port,
                                                 size_t bufferSize)
    {
        return pointer(new TcpConnection(io_context, connection, port, bufferSize));
    }

    TcpConnection::TcpConnection(asio::io_context& io_context,
                                 const std::string& connection,
                                 const std::string& port,
                                 size_t bufferSize):
        socket_(io_context),
        context_(io_context), data(bufferSize), connecting(true), idcode(idcounter++)
    {
        tcp::resolver resolver(io_context);
        tcp::resolver::query query(tcp::v4(), connection, port);
        tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
        socket_.async_connect(*endpoint_iterator,
                              [this](const std::error_code& error) { connect_handler(error); });
    }

    void TcpConnection::connect_handler(const std::error_code& error)
    {
        if (!error) {
            connected.activate();
            socket_.set_option(asio::ip::tcp::no_delay(true));
        } else {
            std::cerr << "connection error " << error.message() << ": code =" << error.value()
                      << '\n';
            connectionError = true;
            connected.activate();
        }
    }
    size_t TcpConnection::send(const void* buffer, size_t dataLength)
    {
        if (!isConnected()) {
            if (!waitUntilConnected(300ms)) {
                std::cerr << "connection timeout waiting again" << std::endl;
            }
            if (!waitUntilConnected(200ms)) {
                std::cerr << "connection timeout twice, now returning" << std::endl;
                return 0;
            }
        }

        size_t sz{0};
        size_t sent_size{dataLength};
        size_t p{0};
        int count{0};
        while (count++ < 5 &&
               (sz = socket_.send(asio::buffer(reinterpret_cast<const char*>(buffer) + p,
                                               sent_size))) != sent_size) {
            sent_size -= sz;
            p += sz;
            //   std::cerr << "DEBUG partial buffer sent" << std::endl;
        }
        if (count >= 5) {
            std::cerr << "TcpConnection send terminated " << std::endl;
            return 0;
        }
        return dataLength;

        //  assert(sz == dataLength);
        //  return sz;
    }

    size_t TcpConnection::send(const std::string& dataString)
    {
        size_t sz;
        sz = send(&dataString[0], dataString.size());
        return sz;
        /*
                if (!isConnected()) {
                    if (!waitUntilConnected(300ms)) {
                        std::cerr << "connection timeout waiting again" << std::endl;
                    }
                    if (!waitUntilConnected(200ms)) {
                        std::cerr << "connection timeout twice, now returning" << std::endl;
                        return 0;
                    }
                }
                auto sz = socket_.send(asio::buffer(dataString));
                assert(sz == dataString.size());
                return sz;
        */
    }

    size_t TcpConnection::receive(void* buffer, size_t maxDataSize)
    {
        return socket_.receive(asio::buffer(buffer, maxDataSize));
    }

    bool TcpConnection::waitUntilConnected(std::chrono::milliseconds timeOut)
    {
        if (isConnected()) {
            return true;
        }
        if (timeOut < 0ms) {
            connected.waitActivation();
            return isConnected();
        }
        connected.wait_forActivation(timeOut);
        return isConnected();
    }

    TcpAcceptor::TcpAcceptor(asio::io_context& io_context, tcp::endpoint& ep):
        endpoint_(ep), acceptor_(io_context)
    {
        acceptor_.open(ep.protocol());
    }

    TcpAcceptor::TcpAcceptor(asio::io_context& io_context, uint16_t port):
        endpoint_(asio::ip::address_v4::any(), port), acceptor_(io_context, endpoint_.protocol()),
        state(accepting_state_t::connected)
    {
    }

    bool TcpAcceptor::connect()
    {
        accepting_state_t exp = accepting_state_t::opened;
        if (state.compare_exchange_strong(exp, accepting_state_t::connecting)) {
            std::error_code ec;
            acceptor_.bind(endpoint_, ec);
            if (ec) {
                state = accepting_state_t::opened;
                std::cout << "acceptor error" << ec << std::endl;
                return false;
            }
            state = accepting_state_t::connected;
            return true;
        }
        return (state == accepting_state_t::connected);
    }

    bool TcpAcceptor::connect(std::chrono::milliseconds timeOut)
    {
        if (state == accepting_state_t::halted) {
            state = accepting_state_t::opened;
        }
        accepting_state_t exp = accepting_state_t::opened;
        if (state.compare_exchange_strong(exp, accepting_state_t::connecting)) {
            bool bindsuccess = false;
            std::chrono::milliseconds tcount{0};
            while (!bindsuccess) {
                std::error_code ec;
                acceptor_.bind(endpoint_, ec);
                if (ec) {
                    if (tcount > timeOut) {
                        state = accepting_state_t::opened;
                        break;
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(200));
                    tcount += std::chrono::milliseconds(200);
                } else {
                    state = accepting_state_t::connected;
                    bindsuccess = true;
                }
            }
            return bindsuccess;
        }
        return (state == accepting_state_t::connected);
    }

    /** start the acceptor*/
    bool TcpAcceptor::start(TcpConnection::pointer conn)
    {
        if (!conn) {
            if (accepting.isActive()) {
                accepting.trigger();
            }
            std::cout << "tcpconnection is not valid" << std::endl;
            return false;
        }
        if (state != accepting_state_t::connected) {
            conn->close();
            if (accepting.isActive()) {
                accepting.trigger();
            }
            std::cout << "acceptor is not in a connected state" << std::endl;
            return false;
        }
        if (accepting.activate()) {
            auto& socket = conn->socket();
            acceptor_.listen();
            auto ptr = shared_from_this();
            acceptor_.async_accept(socket,
                                   [this, apointer = std::move(ptr), connection = std::move(conn)](
                                       const std::error_code& error) {
                                       handle_accept(apointer, connection, error);
                                   });
            return true;
        }

        std::cout << "acceptor is already active" << std::endl;
        conn->close();
        return false;
    }

    /** close the acceptor*/
    void TcpAcceptor::close()
    {
        state = accepting_state_t::halted;
        acceptor_.close();
        accepting.wait();
    }

    std::string TcpAcceptor::to_string() const
    {
        auto str = endpoint_.address().to_string();
        str += ':';
        str += std::to_string(endpoint_.port());
        return str;
    }
    void TcpAcceptor::handle_accept(TcpAcceptor::pointer ptr,
                                    TcpConnection::pointer new_connection,
                                    const std::error_code& error)
    {
        if (state.load() != accepting_state_t::connected) {
            asio::socket_base::linger optionLinger(true, 0);
            std::error_code ec;
            new_connection->socket().set_option(optionLinger, ec);
            new_connection->close();
            accepting.reset();
            return;
        }
        if (!error) {
            if (acceptCall) {
                accepting.reset();
                acceptCall(std::move(ptr), std::move(new_connection));
                if (!accepting.isActive()) {
                    accepting.trigger();
                }
            } else {
                asio::socket_base::linger optionLinger(true, 0);
                try {
                    new_connection->socket().set_option(optionLinger);
                }
                catch (...) {
                }
                new_connection->close();
                accepting.reset();
            }
        } else if (error != asio::error::operation_aborted) {
            if (errorCall) {
                errorCall(std::move(ptr), error);
            } else {
                std::cerr << " error in accept::" << error.message() << std::endl;
            }
            asio::socket_base::linger optionLinger(true, 0);
            try {
                new_connection->socket().set_option(optionLinger);
            }
            catch (...) {
            }
            new_connection->close();
            accepting.reset();
        } else {
            new_connection->close();
            accepting.reset();
        }
    }

    TcpServer::TcpServer(asio::io_context& io_context,
                         const std::string& address,
                         uint16_t portNum,
                         bool port_reuse,
                         int nominalBufferSize):
        ioctx(io_context),
        bufferSize(nominalBufferSize), reuse_address(port_reuse)
    {
        if ((address == "*") || (address == "tcp://*")) {
            endpoints.emplace_back(asio::ip::address_v4::any(), portNum);
            //      endpoints.emplace_back (asio::ip::address_v6::any (), portNum);
        } else if (address == "localhost") {
            endpoints.emplace_back(asio::ip::tcp::v4(), portNum);
        } else {
            tcp::resolver resolver(io_context);
            tcp::resolver::query query(tcp::v4(),
                                       address,
                                       std::to_string(portNum),
                                       tcp::resolver::query::canonical_name);
            tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
            tcp::resolver::iterator end;
            if (endpoint_iterator != end) {
                while (endpoint_iterator != end) {
                    endpoints.push_back(*endpoint_iterator);
                    ++endpoint_iterator;
                }
            } else {
                halted = true;
                return;
            }
        }
        initialConnect();
    }

    TcpServer::TcpServer(asio::io_context& io_context,
                         const std::string& address,
                         const std::string& port,
                         bool port_reuse,
                         int nominalBufferSize):
        ioctx(io_context),
        bufferSize(nominalBufferSize), reuse_address(port_reuse)
    {
        tcp::resolver resolver(io_context);
        tcp::resolver::query query(tcp::v4(), address, port, tcp::resolver::query::canonical_name);
        tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
        tcp::resolver::iterator end;
        if (endpoint_iterator != end) {
            while (endpoint_iterator != end) {
                endpoints.push_back(*endpoint_iterator);
                ++endpoint_iterator;
            }
        } else {
            halted = true;
            return;
        }
        initialConnect();
    }

    TcpServer::TcpServer(asio::io_context& io_context, uint16_t portNum, int nominalBufferSize):
        ioctx(io_context), bufferSize(nominalBufferSize)
    {
        endpoints.emplace_back(asio::ip::tcp::v4(), portNum);
        initialConnect();
    }

    TcpServer::~TcpServer()
    {
        try {
            close();
        }
        catch (...) {
        }
    }

    void TcpServer::initialConnect()
    {
        if (halted.load(std::memory_order_acquire)) {
            std::cout << "previously halted server" << std::endl;
            return;
        }
        for (auto& ep : endpoints) {
            auto acc = TcpAcceptor::create(ioctx, ep);
            if (reuse_address) {
                acc->set_option(tcp::acceptor::reuse_address(true));
            } else {
                acc->set_option(tcp::acceptor::reuse_address(false));
            }
            acc->setAcceptCall([this](TcpAcceptor::pointer accPtr, TcpConnection::pointer conn) {
                handle_accept(std::move(accPtr), std::move(conn));
            });
            acceptors.push_back(std::move(acc));
        }
        bool anyConnect = false;
        size_t connectedAcceptors = 0;
        int index = 0;
        for (auto& acc : acceptors) {
            ++index;
            if (!acc->connect()) {
                std::cout << "unable to connect acceptor " << index << " of " << acceptors.size()
                          << std::endl;
                continue;
            }
            ++connectedAcceptors;
            anyConnect = true;
        }
        if (!anyConnect) {
            halted = true;
            std::cout << "halting server operation";
            return;
        }
        if (connectedAcceptors < acceptors.size()) {
            std::cout << "partial connection on the server " << connectedAcceptors << " of "
                      << acceptors.size() << " were connected" << std::endl;
        }
    }

    bool TcpServer::reConnect(std::chrono::milliseconds timeOut)
    {
        halted = false;
        bool partialConnect = false;
        for (auto& acc : acceptors) {
            if (!acc->isConnected()) {
                if (!acc->connect(timeOut)) {
                    if (partialConnect) {
                        std::cerr << "unable to connect all acceptors on " << acc->to_string()
                                  << '\n';
                    } else {
                        std::cerr << "unable to connect on " << acc->to_string() << '\n';
                    }

                    halted = true;
                    continue;
                }
            }
            partialConnect = true;
        }
        if ((halted.load()) && (partialConnect)) {
            std::cerr << "partial connection on acceptor\n";
        }
        return !halted;
    }

    TcpServer::pointer TcpServer::create(asio::io_context& io_context,
                                         const std::string& address,
                                         uint16_t PortNum,
                                         bool reuse_port,
                                         int nominalBufferSize)
    {
        return pointer(new TcpServer(io_context, address, PortNum, reuse_port, nominalBufferSize));
    }

    TcpServer::pointer TcpServer::create(asio::io_context& io_context,
                                         const std::string& address,
                                         const std::string& port,
                                         bool reuse_port,
                                         int nominalBufferSize)
    {
        return pointer(new TcpServer(io_context, address, port, reuse_port, nominalBufferSize));
    }

    TcpServer::pointer
        TcpServer::create(asio::io_context& io_context, uint16_t PortNum, int nominalBufferSize)
    {
        return pointer(new TcpServer(io_context, PortNum, nominalBufferSize));
    }

    bool TcpServer::start()
    {
        if (halted.load(std::memory_order_acquire)) {
            if (!reConnect(std::chrono::milliseconds(1000))) {
                std::cout << "reconnect failed" << std::endl;
                acceptors.clear();
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
                halted.store(false);
                initialConnect();
                if (halted) {
                    if (!reConnect(std::chrono::milliseconds(1000))) {
                        std::cout << "reconnect part 2 failed" << std::endl;
                        return false;
                    }
                }
            }
        }

        {  // scope for the lock_guard
            std::lock_guard<std::mutex> lock(accepting);
            if (!connections.empty()) {
                for (auto& conn : connections) {
                    if (!conn->isReceiving()) {
                        conn->startReceive();
                    }
                }
            }
        }
        bool success = true;
        for (auto& acc : acceptors) {
            if (!acc->start(TcpConnection::create(ioctx, bufferSize))) {
                std::cout << "acceptor has failed to start" << std::endl;
                success = false;
            }
        }
        return success;
    }

    void TcpServer::handle_accept(TcpAcceptor::pointer acc, TcpConnection::pointer new_connection)
    {
        /*setting linger to 1 second*/
        asio::socket_base::linger optionLinger(true, 0);
        new_connection->socket().set_option(optionLinger);
        new_connection->socket().set_option(asio::ip::tcp::no_delay(true));
        // Set options here
        if (halted.load()) {
            new_connection->close();
            return;
        }

        new_connection->setDataCall(dataCall);
        new_connection->setErrorCall(errorCall);
        new_connection->startReceive();
        {  // scope for the lock_guard

            std::unique_lock<std::mutex> lock(accepting);
            if (!halted.load()) {
                connections.push_back(std::move(new_connection));
            } else {
                lock.unlock();
                new_connection->close();
                return;
            }
        }
        acc->start(TcpConnection::create(ioctx, bufferSize));
    }

    TcpConnection::pointer TcpServer::findSocket(int connectorID) const
    {
        std::unique_lock<std::mutex> lock(accepting);
        auto ptr =
            std::find_if(connections.begin(), connections.end(), [connectorID](const auto& conn) {
                return (conn->getIdentifier() == connectorID);
            });
        if (ptr != connections.end()) {
            return *ptr;
        }
        return nullptr;
    }

    void TcpServer::close()
    {
        halted = true;
        if (acceptors.size() == 1) {
            acceptors[0]->close();
        } else if (!acceptors.empty()) {
            // cancel first to give the threads some time to process
            for (auto& acc : acceptors) {
                acc->cancel();
            }
            for (auto& acc : acceptors) {
                acc->close();
            }
            acceptors.clear();
        }

        std::unique_lock<std::mutex> lock(accepting);
        auto sz = connections.size();
        lock.unlock();
        if (sz > 0) {
            for (decltype(sz) ii = 0; ii < sz; ++ii) {
                connections[ii]->closeNoWait();
            }
            for (decltype(sz) ii = 0; ii < sz; ++ii) {
                connections[ii]->waitOnClose();
            }
            connections.clear();
        }
    }

}  // namespace tcp
}  // namespace helics
