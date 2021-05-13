/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
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

#include "ZmqHelper.h"

#include "cppzmq/zmq.hpp"

#include <algorithm>
#include <cctype>
#include <map>
using zmq::socket_type;
/*
req = ZMQ_REQ,
rep = ZMQ_REP,
dealer = ZMQ_DEALER,
router = ZMQ_ROUTER,
pub = ZMQ_PUB,
sub = ZMQ_SUB,
xpub = ZMQ_XPUB,
xsub = ZMQ_XSUB,
push = ZMQ_PUSH,
pull = ZMQ_PULL,
#if ZMQ_VERSION_MAJOR < 4
    pair = ZMQ_PAIR
#else
    pair = ZMQ_PAIR,
    stream = ZMQ_STREAM
    */

/* *INDENT-OFF* */
static const std::map<std::string, socket_type> socketMap{{"req", socket_type::req},
                                                          {"request", socket_type::req},
                                                          {"rep", socket_type::rep},
                                                          {"reply", socket_type::rep},
                                                          {"dealer", socket_type::dealer},
                                                          {"router", socket_type::router},
                                                          {"pub", socket_type::pub},
                                                          {"publish", socket_type::pub},
                                                          {"sub", socket_type::sub},
                                                          {"subscribe", socket_type::sub},
                                                          {"xpub", socket_type::xpub},
                                                          {"xsub", socket_type::xsub},
                                                          {"push", socket_type::push},
                                                          {"pull", socket_type::pull},
                                                          {"pair", socket_type::pair},
                                                          {"stream", socket_type::stream}};
/* *INDENT-ON* */

socket_type socketTypeFromString(const std::string& socketType)
{
    auto fnd = socketMap.find(socketType);
    if (fnd != socketMap.end()) {
        return fnd->second;
    }

    /* try making it lower case*/
    std::string lowerCase(socketType);
    std::transform(socketType.cbegin(), socketType.cend(), lowerCase.begin(), ::tolower);
    fnd = socketMap.find(lowerCase);
    if (fnd != socketMap.end()) {
        return fnd->second;
    }
    assert(false);  // NEED to make this a throw operation instead once exceptions are integrated
    return socket_type::req;
}
