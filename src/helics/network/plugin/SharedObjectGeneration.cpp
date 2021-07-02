/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "SharedObjectGeneration.hpp"
#include "../../core/core-exceptions.hpp"
#if !_WIN32
#include <dlfcn.h>
#endif

namespace helics::plugins {

    FunctionRef::FunctionRef(Library* lib, const std::string& functionName) {
        #if _WIN32
        fcall = GetProcAddress(lib->library, TEXT(functionName.c_str()));
        #else
        fcall = dlsym(lib->library, functionName.c_str());
        #endif
        if (fcall==nullptr) {
            throw FunctionExecutionFailure("unable to load function symbol");
        }
     }


    Library::Library(const std::string& libraryPath) {
        #if _WIN32
        library = LoadLibrary(TEXT(libraryPath.c_str()));
        #else
        library = dlopen(libraryPath.c_str(), RTLD_LAZY);
        #endif
        if (library == nullptr) {
            throw FunctionExecutionFailure("unable to load library");
        }
    }
    Library::~Library() {
#if _WIN32
        if (library!=NULL) {
            FreeLibrary(library);
        }
#else
        if (library != nullptr) {
            dlclose(library);
        }
#endif

}

std::shared_ptr<FunctionRef> Library::getFunction(const std::string& functionName) {
    return std::make_shared<FunctionRef>(this, functionName);
}
}
