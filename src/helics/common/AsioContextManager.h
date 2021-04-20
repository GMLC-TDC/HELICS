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
#include <future>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <utility>

// The choice for noexcept isn't set correctly in asio::io_context (including asio.hpp instead
// didn't help) With Boost 1.58 this resulted in a compile error, apparently from the BOOST_NOEXCEPT
// define being empty
#ifdef ASIO_ERROR_CATEGORY_NOEXCEPT
#    undef ASIO_ERROR_CATEGORY_NOEXCEPT
#endif

#define ASIO_ERROR_CATEGORY_NOEXCEPT noexcept(true)
#include <asio/io_context.hpp>
#undef ASIO_ERROR_CATEGORY_NOEXCEPT

/** class defining a (potential) singleton Asio io_context manager for all asio usage*/
class AsioContextManager: public std::enable_shared_from_this<AsioContextManager> {
  private:
    enum class loop_mode : int { stopped = 0, starting = 1, running = 2 };
    static std::map<std::string, std::shared_ptr<AsioContextManager>>
        contexts;  //!< container for pointers to all the available contexts
    std::atomic<int> runCounter{
        0};  //!< counter for the number of times the runContextLoop has been called
    std::string name;  //!< context name
    std::unique_ptr<asio::io_context> ictx;  //!< pointer to the actual context
    std::unique_ptr<asio::io_context::work>
        nullwork;  //!< pointer to an object used to keep a context running
    bool leakOnDelete = false;  //!< this is done to prevent some warning messages for use in DLL's
    std::atomic<loop_mode> running{loop_mode::stopped};  //!< flag indicating the loop is running
    std::mutex runningLoopLock;  //!< lock protecting the nullwork object and the return future
    std::atomic<bool> terminateLoop{false};  //!< flag indicating that the loop should terminate
    std::future<void> loopRet;
    /** constructor*/
    explicit AsioContextManager(const std::string& contextName);

    /** servicing helper class to manage lifetimes of a context loop*/
    class Servicer {
      public:
        explicit Servicer(std::shared_ptr<AsioContextManager> manager):
            contextManager(std::move(manager))
        {
        }
        /** this object halts the contextLoop when deleted*/
        ~Servicer()
        {
            if (contextManager) {
                try {
                    contextManager->haltContextLoop();
                }
                catch (...) {
                    // no exceptions in a destructor
                }
            }
        }
        /** move constructor*/
        Servicer(Servicer&& sv) = default;

      private:
        std::shared_ptr<AsioContextManager> contextManager;  //!< a pointer to the context manager
    };

  public:
    using LoopHandle = std::unique_ptr<Servicer>;

    /** return a pointer to a context manager
    @details the function will search for an existing context manager for the name
    if it doesn't find one it will create a new one
    @param contextName the name of the context to find or create*/
    static std::shared_ptr<AsioContextManager>
        getContextPointer(const std::string& contextName = std::string());
    /** return a pointer to a context manager
    @details the function will search for an existing context manager for the name
    if it doesn't find one it will return nullptr
    @param contextName the name of the context to find
    */
    static std::shared_ptr<AsioContextManager>
        getExistingContextPointer(const std::string& contextName = std::string());
    /** get the asio io_context associated with the context manager
     */
    static asio::io_context& getContext(const std::string& contextName = std::string());
    /** get the asio io_context associated with the context manager but only if the context exists
    if it doesn't this will throw and invalid_argument exception
    */
    static asio::io_context& getExistingContext(const std::string& contextName = std::string());

    static void closeContext(const std::string& contextName = std::string());
    /** tell the context to free the pointer and leak the memory on delete
    @details You may ask why, well in windows systems when operating in a DLL if this context is
    closed after certain other operations that happen when the DLL is unlinked bad things can
    happen, and since in nearly all cases this happens at Shutdown leaking really doesn't matter
    that much and if you don't the context could terminate before some other parts of the program
    which cause all sorts of odd errors and issues
    */
    static void setContextToLeakOnDelete(const std::string& contextName = std::string());
    virtual ~AsioContextManager();

    /** get the name  of the current context manager*/
    const std::string& getName() const { return name; }

    /** get the underlying asio::io_context reference*/
    asio::io_context& getBaseContext() const { return *ictx; }

    /** run a single thread for the context manager to execute asynchronous contexts in
    @details will run a single thread for the io_context,  it will not stop the thread until either
    the context manager is closed or the haltContextLoop function is called and there is no more
    work
    @param contextName the name of the context
    */
    static LoopHandle runContextLoop(const std::string& contextName = std::string{});

    /** run a single thread for the context manager to execute asynchronous contexts in
    @details will run a single thread for the io_context,  it will not stop the thread until either
    the context manager is closed or the haltContextLoop function is called and there is no more
    work
    */
    LoopHandle startContextLoop();
    /** check if the contextLoopo is running*/
    bool isRunning() const { return (running.load() != loop_mode::stopped); }

  private:
    /** halt the context loop thread if the counter==0
    @details decrements the loop request counter and if it is 0 then will halt the
    context loop
    */
    void haltContextLoop();

    friend void contextProcessingLoop(std::shared_ptr<AsioContextManager> ptr);
};

void contextProcessingLoop(std::shared_ptr<AsioContextManager> ptr);
