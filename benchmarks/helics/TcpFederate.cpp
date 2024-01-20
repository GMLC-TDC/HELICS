/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "gmlc/networking/AsioContextManager.h"
#include "gmlc/networking/TcpHelperClasses.h"
#include "helics/network/networkDefaults.hpp"
#include "helics/network/tcp/TcpBroker.h"
#include "helics/network/tcp/TcpComms.h"
#include "helics/network/tcp/TcpCore.h"

#include <iostream>
#include <string>
#include <vector>

using namespace std::literals::chrono_literals;  // NOLINT
using asio::ip::tcp;
using gmlc::networking::AsioContextManager;
using helics::Core;

int singleMessage(int msg_size)
{
    helics::tcp::TcpComms tcpFed;
    std::string localhost = "localhost";
    int count = 0;
    tcpFed.loadTargetInfo(localhost, localhost);

    auto contextp = AsioContextManager::getContextPointer();

    auto server = gmlc::networking::TcpServer::create(contextp->getBaseContext(),
                                                      localhost,
                                                      helics::network::DEFAULT_TCP_PORT);

    while (!server->isReady()) {
        std::this_thread::sleep_for(100ms);
    }

    auto contextloop = contextp->startContextLoop();

    size_t data_recv_size;
    server->setDataCall(
        [&](const gmlc::networking::TcpConnection::pointer&, const char* data, size_t data_size) {
            data_recv_size = data_size;
            count++;
            std::cout << "received msg: " << data << '\n';
            return data_size;
        });

    server->start();

    auto connection = gmlc::networking::TcpConnection::create(contextp->getBaseContext(),
                                                              localhost,
                                                              "24160",
                                                              1024);
    auto connected = connection->waitUntilConnected(1000ms);
    if (!connected) {
        connected = connection->waitUntilConnected(1000ms);
    }
    if (!connected) {
        std::cerr << "unable to connect" << std::endl;
        return -1;
    }
    std::this_thread::sleep_for(200ms);
    std::string txstring(msg_size, '1');
    connection->send(txstring);
    std::this_thread::sleep_for(100ms);

    connection->close();
    server->close();
    return 0;
}

int singleConnection(int n)
{
    helics::tcp::TcpComms tcpFed;
    std::string localhost = "localhost";
    int count = 0;
    tcpFed.loadTargetInfo(localhost, localhost);

    auto contextp = AsioContextManager::getContextPointer();

    auto server = gmlc::networking::TcpServer::create(contextp->getBaseContext(),
                                                      localhost,
                                                      helics::network::DEFAULT_TCP_PORT);

    while (!server->isReady()) {
        std::this_thread::sleep_for(100ms);
    }

    auto contextloop = contextp->startContextLoop();

    size_t data_recv_size;
    server->setDataCall([&](const gmlc::networking::TcpConnection::pointer&,
                            [[maybe_unused]] const char* data,
                            size_t data_size) {
        data_recv_size = data_size;
        count++;
        return data_size;
    });

    server->start();

    auto connection = gmlc::networking::TcpConnection::create(contextp->getBaseContext(),
                                                              localhost,
                                                              "24160",
                                                              1024);
    auto connected = connection->waitUntilConnected(1000ms);

    std::this_thread::sleep_for(200ms);
    if (!connected) {
        connected = connection->waitUntilConnected(1000ms);
    }
    if (!connected) {
        std::cerr << "unable to connect" << std::endl;
        return -1;
    }
    for (int i = 1; i <= n; i++) {
        // send messages the size of i
        std::string txstring(i, '1');
        connection->send(txstring);
        std::this_thread::sleep_for(100ms);
    }
    std::cout << "num msgs received: " << count;

    connection->close();
    server->close();
    return 0;
}

int main()
{
    [[maybe_unused]] int msg_size = 1000;
    int num_messages = 1000;

    // send a single message of a certain size
    // singleMessage(msg_size);

    // send increasing larger messages up to size num_messages
    return singleConnection(num_messages);
}
