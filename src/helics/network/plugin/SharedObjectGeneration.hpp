/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once
#include <string>
#include <memory>


#ifdef _WIN32
#include <windows.h>
#    define LIBTYPE HMODULE
#    define FCALLTYPE FARPROC
#else
#define LIBTYPE void *
#define FCALLTYPE void *
#endif
namespace helics::plugins {
class Library;

    class FunctionRef {
  private:
        FCALLTYPE fcall{nullptr};

  public:
        FunctionRef(Library* lib, const std::string& functionName);
    ~FunctionRef();

    };

    class Library {
        private:
        LIBTYPE library{nullptr};

        public:
        Library(const std::string& libraryPath);
          ~Library();

          std::shared_ptr<FunctionRef> getFunction(const std::string& functionName);

          friend class FunctionRef;
    };
}
