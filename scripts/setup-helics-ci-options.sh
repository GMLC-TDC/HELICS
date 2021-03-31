#!/bin/bash

# Save the bash setting for not matching case
SHELLNOCASEMATCH=$(
    shopt -p nocasematch
    true
)
shopt -s nocasematch

# Setup the flags for configuring HELICS with CMake
OPTION_FLAGS_ARR=()
OPTION_FLAGS_ARR+=("-DHELICS_BUILD_TESTS=ON -DHELICS_BUILD_EXAMPLES=ON -DHELICS_BUILD_CXX_SHARED_LIB=ON" "-DHELICS_EXAMPLES_WARNINGS_AS_ERROR=ON")

# Enable adding the slower subproject tests; will not run for CI builds unless they run ctest with the Packaging label
OPTION_FLAGS_ARR+=("-DHELICS_ENABLE_SUBPROJECT_TESTS=ON")

# Options to control building zeromq
if [[ "$ZMQ_SUBPROJECT" ]]; then
    OPTION_FLAGS_ARR+=("-DHELICS_ZMQ_SUBPROJECT=ON")
fi

if [[ "$ZMQ_FORCE_SUBPROJECT" ]]; then
    OPTION_FLAGS_ARR+=("-DHELICS_ZMQ_FORCE_SUBPROJECT=ON")
fi

if [[ "$ZMQ_STATIC" ]]; then
    OPTION_FLAGS_ARR+=("-DHELICS_USE_ZMQ_STATIC_LIBRARY=ON")
fi

# Options to control building swig interfaces
if [[ "${DISABLE_INTERFACES}" != *"Java"* ]]; then
    OPTION_FLAGS_ARR+=("-DBUILD_JAVA_INTERFACE=ON")
fi
if [[ "${DISABLE_INTERFACES}" != *"Python"* ]]; then
    OPTION_FLAGS_ARR+=("-DBUILD_PYTHON_INTERFACE=ON")

    if [[ "$PYTHON_LIB_PATH" ]]; then
        OPTION_FLAGS_ARR+=("-DPYTHON_LIBRARY=${PYTHON_LIB_PATH}")
    fi
    if [[ "$PYTHON_INCLUDE_PATH" ]]; then
        OPTION_FLAGS_ARR+=("-DPYTHON_INCLUDE_DIR=${PYTHON_INCLUDE_PATH}")
    fi
    if [[ "$PYTHON_EXECUTABLE" ]]; then
        OPTION_FLAGS_ARR+=("-DPYTHON_EXECUTABLE=${PYTHON_EXECUTABLE}")
    fi
fi
if [[ "$USE_SWIG" == 'true' ]]; then
    OPTION_FLAGS_ARR+=("-DHELICS_ENABLE_SWIG=ON")
fi

if [[ "$BUILD_BENCHMARKS" == 'true' ]]; then
    OPTION_FLAGS_ARR+=("-DHELICS_BUILD_BENCHMARKS=ON")
fi

if [[ "$DISABLE_EXAMPLES" ]]; then
    OPTION_FLAGS_ARR+=("-DHELICS_BUILD_EXAMPLES=OFF")
fi

# Options related to the CMake build type
if [[ "$BUILD_TYPE" ]]; then
    OPTION_FLAGS_ARR+=("-DCMAKE_BUILD_TYPE=${BUILD_TYPE}")
    if [[ "$BUILD_TYPE" == "Coverage" ]]; then
        OPTION_FLAGS_ARR+=("-DHELICS_TEST_CODE_COVERAGE=ON")
    fi
else
    OPTION_FLAGS_ARR+=("-DCMAKE_BUILD_TYPE=Release")
fi

# CPack/Install options
if [[ "$ENABLE_CPACK" ]]; then
    OPTION_FLAGS_ARR+=("-DHELICS_ENABLE_PACKAGE_BUILD=ON")
fi
if [[ "$INSTALL_SYSTEM_LIBRARIES" ]]; then
    OPTION_FLAGS_ARR+=("-DINSTALL_SYSTEM_LIBRARIES=ON")
fi

# MPI options
if [[ "$USE_MPI" ]]; then
    OPTION_FLAGS_ARR+=("-DENABLE_MPI_CORE=ON")
    CC=mpicc
    export CC
    CXX=mpic++
    export CXX
fi

# Compiler/language options
if [[ "$CXX_STANDARD" ]]; then
    OPTION_FLAGS_ARR+=("-DCMAKE_CXX_STANDARD=${CXX_STANDARD}")
fi

# Travis related options
if [[ "$TRAVIS_OS_NAME" == "osx" ]]; then
    OPTION_FLAGS_ARR+=("-DHELICS_DISABLE_SYSTEM_CALL_TESTS=ON")
fi
export HELICS_OPTION_FLAGS=${OPTION_FLAGS_ARR[*]}

# Set any HELICS flags for finding dependencies
if [[ "$TRAVIS_OS_NAME" == "linux" ]]; then
    export HELICS_DEPENDENCY_FLAGS+="-DBOOST_INSTALL_PATH=${CI_DEPENDENCY_DIR}/boost"
fi

# Setup the flags for controlling test execution
TEST_FLAGS_ARR=("$TEST_TYPE")
if [[ "$CI_TEST_CONFIG" ]]; then
    TEST_FLAGS_ARR+=("$CI_TEST_CONFIG")
fi
if [[ "$CI_TEST_MATCH" ]]; then
    TEST_FLAGS_ARR+=("--match-tests" "$CI_TEST_MATCH")
fi
if [[ "$CI_TEST_EXCLUDE" ]]; then
    TEST_FLAGS_ARR+=("--exclude-tests" "$CI_TEST_EXCLUDE")
fi

# Valgrind options
if [[ "$RUN_VALGRIND" ]]; then
    TEST_FLAGS_ARR+=("--valgrind" "--disable-unit-tests")
fi
if [[ "$RUN_CACHEGRIND" ]]; then
    TEST_FLAGS_ARR+=("--cachegrind")
fi

# Sanitizer tests (supported: asan, msan, tsan, ubsan)
if [[ "$RUN_SANITIZER" ]]; then
    TEST_FLAGS_ARR+=("--${RUN_SANITIZER}")
fi

# Misc options
if [[ "$NO_CTEST" ]]; then
    TEST_FLAGS_ARR+=("--no-ctest")
fi
if [[ "$CTEST_VERBOSE" ]]; then
    TEST_FLAGS_ARR+=("--ctest-verbose")
fi
if [[ "$DISABLE_CI_TESTS" ]]; then
    TEST_FLAGS_ARR+=("--disable-unit-tests")
fi
export CI_TEST_FLAGS=${TEST_FLAGS_ARR[*]}

# Restore the origin nocasematch setting
$SHELLNOCASEMATCH
