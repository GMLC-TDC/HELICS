# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2021, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

set(NETWORK_SRC_FILES NetworkCommsInterface.cpp NetworkBrokerData.cpp CommsInterface.cpp
                      CommsBroker.cpp loadCores.cpp
)

set(TESTCORE_SOURCE_FILES test/TestBroker.cpp test/TestCore.cpp test/TestComms.cpp)

set(INPROCCORE_SOURCE_FILES inproc/InprocBroker.cpp inproc/InprocCore.cpp inproc/InprocComms.cpp)

set(IPC_SOURCE_FILES ipc/IpcCore.cpp ipc/IpcBroker.cpp ipc/IpcComms.cpp ipc/IpcQueueHelper.cpp
                     # ipc/IpcBlockingPriorityQueue.cpp ipc/IpcBlockingPriorityQueueImpl.cpp
)

set(MPI_SOURCE_FILES mpi/MpiCore.cpp mpi/MpiBroker.cpp mpi/MpiComms.cpp mpi/MpiService.cpp)

set(ZMQ_SOURCE_FILES
    zmq/ZmqCore.cpp
    zmq/ZmqBroker.cpp
    zmq/ZmqComms.cpp
    zmq/ZmqCommsSS.cpp
    zmq/ZmqRequestSets.cpp
    zmq/ZmqCommsCommon.cpp
    zmq/ZmqContextManager.cpp
    # zmq/zmqSocketDescriptor.cpp
    zmq/ZmqHelper.cpp
)

set(UDP_SOURCE_FILES udp/UdpCore.cpp udp/UdpBroker.cpp udp/UdpComms.cpp)

set(TCP_SOURCE_FILES
    tcp/TcpCore.cpp
    tcp/TcpBroker.cpp
    tcp/TcpComms.cpp
    tcp/TcpCommsSS.cpp
    tcp/TcpHelperClasses.cpp
    tcp/TcpCommsCommon.cpp
)

set(NETWORK_INCLUDE_FILES
    NetworkCommsInterface.hpp
    NetworkBrokerData.hpp
    NetworkBroker.hpp
    NetworkCore.hpp
    NetworkBroker_impl.hpp
    NetworkCore_impl.hpp
    networkDefaults.hpp
    CommsBroker.hpp
    CommsBroker_impl.hpp
    CommsInterface.hpp
    loadCores.hpp
)

set(TESTCORE_HEADER_FILES test/TestCore.h test/TestBroker.h test/TestComms.h)

set(INPROCCORE_HEADER_FILES inproc/InprocCore.h inproc/InprocBroker.h inproc/InprocComms.h)

set(IPC_HEADER_FILES ipc/IpcCore.h ipc/IpcBroker.h ipc/IpcComms.h ipc/IpcQueueHelper.h
                     # ipc/IpcBlockingPriorityQueue.hpp ipc/IpcBlockingPriorityQueueImpl.hpp
)

set(ZMQ_HEADER_FILES
    zmq/ZmqCore.h
    zmq/ZmqBroker.h
    zmq/ZmqComms.h
    zmq/ZmqCommsSS.h
    zmq/ZmqRequestSets.h
    zmq/ZmqCommsCommon.h
    zmq/ZmqContextManager.h
    # zmq/zmqSocketDescriptor.h
    zmq/ZmqHelper.h
    ${HELICS_SOURCE_DIR}/ThirdParty/cppzmq/zmq.hpp
    ${HELICS_SOURCE_DIR}/ThirdParty/cppzmq/zmq_addon.hpp
)

set(MPI_HEADER_FILES mpi/MpiCore.h mpi/MpiBroker.h mpi/MpiComms.h mpi/MpiService.h)

set(UDP_HEADER_FILES udp/UdpCore.h udp/UdpBroker.h udp/UdpComms.h)

set(TCP_HEADER_FILES
    tcp/TcpCore.h
    tcp/TcpBroker.h
    tcp/TcpComms.h
    tcp/TcpCommsSS.h
    tcp/TcpHelperClasses.h
    tcp/TcpCommsCommon.h
)

if(ENABLE_TEST_CORE)
    list(APPEND NETWORK_SRC_FILES ${TESTCORE_SOURCE_FILES})
    list(APPEND NETWORK_INCLUDE_FILES ${TESTCORE_HEADER_FILES})
endif()

if(ENABLE_INPROC_CORE)
    list(APPEND NETWORK_SRC_FILES ${INPROCCORE_SOURCE_FILES})
    list(APPEND NETWORK_INCLUDE_FILES ${INPROCCORE_HEADER_FILES})
endif()

if(ENABLE_UDP_CORE)
    list(APPEND NETWORK_SRC_FILES ${UDP_SOURCE_FILES})
    list(APPEND NETWORK_INCLUDE_FILES ${UDP_HEADER_FILES})
endif()

if(ENABLE_IPC_CORE)
    list(APPEND NETWORK_SRC_FILES ${IPC_SOURCE_FILES})
    list(APPEND NETWORK_INCLUDE_FILES ${IPC_HEADER_FILES})
endif()

if(ENABLE_TCP_CORE)
    list(APPEND NETWORK_SRC_FILES ${TCP_SOURCE_FILES})
    list(APPEND NETWORK_INCLUDE_FILES ${TCP_HEADER_FILES})
endif()

if(ENABLE_ZMQ_CORE)
    list(APPEND NETWORK_SRC_FILES ${ZMQ_SOURCE_FILES})
    list(APPEND NETWORK_INCLUDE_FILES ${ZMQ_HEADER_FILES})
endif(ENABLE_ZMQ_CORE)

if(ENABLE_MPI_CORE)
    list(APPEND NETWORK_SRC_FILES ${MPI_SOURCE_FILES})
    list(APPEND NETWORK_INCLUDE_FILES ${MPI_HEADER_FILES})
endif(ENABLE_MPI_CORE)

add_library(helics_network STATIC ${NETWORK_SRC_FILES} ${NETWORK_INCLUDE_FILES})

target_link_libraries(
    helics_network PUBLIC helics_core PRIVATE fmt::fmt compile_flags_target helics::zmq
)

if(TARGET Boost::boost AND NOT HELICS_DISABLE_BOOST)
    target_compile_definitions(helics_network PRIVATE BOOST_DATE_TIME_NO_LIB)
    target_link_libraries(helics_network PRIVATE Boost::boost)
endif()

if(ENABLE_MPI_CORE)
    source_group("mpi" FILES ${MPI_SOURCE_FILES} ${MPI_HEADER_FILES})
endif(ENABLE_MPI_CORE)

if(ENABLE_ZMQ_CORE)
    source_group("zmq" FILES ${ZMQ_SOURCE_FILES} ${ZMQ_HEADER_FILES})
endif(ENABLE_ZMQ_CORE)

if(ENABLE_UDP_CORE)
    source_group("udp" FILES ${UDP_SOURCE_FILES} ${UDP_HEADER_FILES})
endif()

if(ENABLE_IPC_CORE)
    source_group("ipc" FILES ${IPC_SOURCE_FILES} ${IPC_HEADER_FILES})
endif()

if(ENABLE_TEST_CORE)
    source_group("test" FILES ${TESTCORE_SOURCE_FILES} ${TESTCORE_HEADER_FILES})
endif()

if(ENABLE_TCP_CORE)
    source_group("tcp" FILES ${TCP_SOURCE_FILES} ${TCP_HEADER_FILES})
endif()

if(ENABLE_INPROC_CORE)
    source_group("inproc" FILES ${INPROCCORE_SOURCE_FILES} ${INPROCCORE_HEADER_FILES})
endif()

if(ENABLE_MPI_CORE)
    target_include_directories(
        helics_network PRIVATE "${MPI_C_INCLUDE_DIR}" "${MPI_C_HEADER_DIR}"
                               "${MPI_C_ADDITIONAL_INCLUDE_DIRS}"
    )
endif(ENABLE_MPI_CORE)

add_library(HELICS::network ALIAS helics_network)

target_compile_options(
    helics_network
    PRIVATE $<$<COMPILE_LANGUAGE:CXX>:$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wno-strict-aliasing>>
)
