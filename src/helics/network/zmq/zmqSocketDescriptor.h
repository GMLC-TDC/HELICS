/*
Copyright (c) 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
/*
 * LLNS Copyright Start
 * Copyright (c) 2017, Lawrence Livermore National Security
 * This work was performed under the auspices of the U.S. Department
 * of Energy by Lawrence Livermore National Laboratory in part under
 * Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
 * Produced at the Lawrence Livermore National Laboratory.
 * All rights reserved.
 * For details, see the LICENSE file.
 * LLNS Copyright End
 */

#pragma once

#include "cppzmq/zmq_addon.hpp"

#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <vector>

/** enumeration of possible operations on a socket*/
enum class socket_ops {
    bind,
    connect,
    unbind,
    disconnect,
    subscribe,
    unsubscribe,
};

using socketOperation =
    std::pair<socket_ops, std::string>;  //!< easy definition for operation instruction

/** data class describing a socket and some operations on it*/
class ZmqSocketDescriptor {
  public:
    std::string name;  //!< name of the socket for later reference
    zmq::socket_type type = zmq::socket_type::sub;  //!< the socket type
    std::vector<socketOperation> ops;  //!< a list of connections of make through bind
    std::function<void(const zmq::multipart_t& res)> callback;  //!< the message handler
    ZmqSocketDescriptor(std::string socketName = ""):
        name(std::move(socketName)) {}  // purposefully implicit
    ZmqSocketDescriptor(std::string socketName, zmq::socket_type stype):
        name(std::move(socketName)), type(stype)
    {
    }
    inline void addOperation(socket_ops op, const std::string& desc) { ops.emplace_back(op, desc); }
    zmq::socket_t makeSocket(zmq::context_t& ctx) const;
    std::unique_ptr<zmq::socket_t> makeSocketPtr(zmq::context_t& ctx) const;
    void modifySocket(zmq::socket_t& sock) const;
};
