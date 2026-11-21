/*
Copyright (c) 2017-2026,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Energy
Innovation LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "helics/application_api/Federate.hpp"

#include "gtest/gtest.h"

#if defined(__linux__) && defined(HELICS_TEST_CRASH_TRACE)
#    include <csignal>
#    include <execinfo.h>
#    include <initializer_list>
#    include <unistd.h>

namespace {
void printCrashTrace(int signalNumber)
{
    constexpr char heading[] = "\nHELICS test fatal signal; crashing thread backtrace follows\n";
    static_cast<void>(::write(STDERR_FILENO, heading, sizeof(heading) - 1));
    void* frames[64];
    const int count = ::backtrace(frames, 64);
    ::backtrace_symbols_fd(frames, count, STDERR_FILENO);

    // SA_RESETHAND restores the default action, preserving the fatal signal for CTest.
    static_cast<void>(::raise(signalNumber));
}

void enableCrashTrace()
{
    // Load the unwinder before a crash to avoid its initial dynamic loading in the handler.
    void* frames[1];
    static_cast<void>(::backtrace(frames, 1));
    for (int signalNumber : {SIGSEGV, SIGBUS, SIGILL, SIGFPE, SIGABRT}) {
        struct sigaction previous{};
        if (::sigaction(signalNumber, nullptr, &previous) == 0 && previous.sa_handler == SIG_DFL) {
            struct sigaction action{};
            action.sa_handler = printCrashTrace;
            static_cast<void>(::sigemptyset(&action.sa_mask));
            action.sa_flags = static_cast<int>(SA_RESETHAND | SA_NODEFER);
            static_cast<void>(::sigaction(signalNumber, &action, nullptr));
        }
    }
}
}  // namespace
#endif

struct globalTestConfig: public ::testing::Environment {
    virtual void SetUp() override
    {
#if defined(__linux__) && defined(HELICS_TEST_CRASH_TRACE)
        enableCrashTrace();
#endif
    }
    virtual void TearDown() override { helics::cleanupHelicsLibrary(); }
};

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::AddGlobalTestEnvironment(new globalTestConfig);
    return RUN_ALL_TESTS();
}
