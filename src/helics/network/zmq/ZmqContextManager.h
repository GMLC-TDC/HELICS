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
#pragma once

#include <atomic>
#include <map>
#include <memory>
#include <string>
namespace zmq {
class context_t;
}  // namespace zmq

/** class defining a global context manager for all zmq usage
@details the zmq::context_t is stored in global structure so more than one can exist, but most use
cases it is just the equivalent of a singleton*/
class ZmqContextManager {
  private:
    static std::map<std::string, std::shared_ptr<ZmqContextManager>>
        contexts;  //!< container for pointers to all the available contexts
    std::string name;  //!< context name
    std::unique_ptr<zmq::context_t> zcontext;  //!< pointer to the actual context
    std::atomic<bool> leakOnDelete{true};  //!< this is done to prevent some errors if zmq is not
                                           //!< built properly or the OS shuts it down early

    /** the constructor is private to make sure it is created through the static functions*/
    explicit ZmqContextManager(const std::string& contextName);

  public:
    /** get a shared_ptr to the context by name*/
    static std::shared_ptr<ZmqContextManager>
        getContextPointer(const std::string& contextName = std::string{});
    /** get an underlying zmq::context_t reference by name */
    static zmq::context_t& getContext(const std::string& contextName = std::string{});
    /** start a ZMQ context if it hasn't been started already*/
    static void startContext(const std::string& contextName = std::string{});
    /** close a ZMQ context if it exists*/
    static void closeContext(const std::string& contextName = std::string{});
    /** tell the context to free the pointer and leak the memory on delete
    @details You may ask why, well in windows systems when operating in a DLL if this context is
    closed after certain other operations that happen when the DLL is unlinked bad things can
    happen, and since in nearly all cases this happens at Shutdown leaking really doesn't matter
    that much, it also seem to be required when the zmq library is built with curve instead of
    turning off the encryption or with sodium,  in that case there seems to be some issues in the
    zmq library itself when closing the context, causing some issue that eventually leads to
    something trying to access a deleted mutex on program shutdown.  Which is annoying.
    @return true if the context was found and the flag set, false otherwise
    */
    static bool setContextToLeakOnDelete(const std::string& contextName = std::string{});
    /** turn off the leak on delete mode*/
    static bool setContextToNotLeakOnDelete(const std::string& contextName = std::string{});
    /** destructor*/
    ~ZmqContextManager();
    /** get the name of the context*/
    const std::string& getName() const { return name; }
    /** get a reference to the underlying zmq::context_t for use with cppzmq library calls*/
    zmq::context_t& getBaseContext() const { return *zcontext; }
};
