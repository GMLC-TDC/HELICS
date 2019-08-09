/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
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
#pragma once

#include <atomic>
#include <map>
#include <memory>
#include <string>
#include <vector>
namespace zmq
{
class context_t;
}  // namespace zmq

/** class defining a singleton context manager for all zmq usage in gridDyn*/
class ZmqContextManager
{
  private:
    static std::map<std::string, std::shared_ptr<ZmqContextManager>>
      contexts;  //!< container for pointers to all the available contexts
    std::string name;  //!< context name
    std::unique_ptr<zmq::context_t> zcontext;  //!< pointer to the actual context
    std::atomic<bool> leakOnDelete{true};  //!< this is done to prevent some errors if zmq is not built properly or the OS shuts it down early
    explicit ZmqContextManager (const std::string &contextName);

  public:
    static std::shared_ptr<ZmqContextManager> getContextPointer (const std::string &contextName = std::string{});

    static zmq::context_t &getContext (const std::string &contextName = std::string{});
    /** start a ZMQ context if it hasn't been started already*/
    static void startContext (const std::string &contextName = std::string{});
    /** close a ZMQ context if it exists already regardless of leakOnDelete Status*/
    static void closeContext (const std::string &contextName = std::string{});
    /** tell the context to free the pointer and leak the memory on delete
    @details You may ask why, well in windows systems when operating in a DLL if this context is closed after
    certain other operations that happen when the DLL is unlinked bad things can happen, and since in nearly all
    cases this happens at Shutdown leaking really doesn't matter that much
    @return true if the context was found and the flag set, false otherwise
    */
    static bool setContextToLeakOnDelete (const std::string &contextName = std::string{});
    static bool setContextToNotLeakOnDelete (const std::string &contextName = std::string{});
    virtual ~ZmqContextManager ();

    const std::string &getName () const { return name; }

    zmq::context_t &getBaseContext () const { return *zcontext; }
};
