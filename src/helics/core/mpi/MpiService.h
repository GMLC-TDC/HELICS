/*

Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#pragma once

#include "MpiComms.h"
#include "../../common/BlockingQueue.hpp"
#include "../ActionMessage.hpp"
#include <atomic>
#include <set>
#include <string>
#include <future>
#include <functional>
#include <vector>
#include <thread>
#include <memory>
#include <mutex>
#include "helics/helics-config.h"

#include <mpi.h>

namespace helics {

/** service for using MPI to communicate*/
class MpiService {

public:
    static MpiService& getInstance();
    static void setMpiCommunicator (MPI_Comm communicator);

    int getRank();
    int addMpiComms (MpiComms *comm);
    void removeMpiComms (MpiComms *comm);
    int getTag (MpiComms *comm);

private:
    MpiService();
    ~MpiService();
    MpiService(const MpiService&) = delete;
    MpiService& operator=(const MpiService&) = delete;

    std::vector<MpiComms*> comms;
    static std::atomic<int> commRank; //!< the mpi rank of this process object
    static MPI_Comm mpiCommunicator;
    std::atomic<bool> stop_service;
    std::unique_ptr<std::thread> service_thread;

    void startService();
    void serviceLoop();

    bool initMPI();
};


} // namespace helics

