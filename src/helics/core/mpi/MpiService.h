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
#include <string>
#include <functional>
#include <list>
#include <vector>
#include <utility>
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
    static void setStartServiceThread (bool start);

    std::string addMpiComms (MpiComms *comm);
    void removeMpiComms (MpiComms *comm);
    std::string getAddress (MpiComms *comm);
    int getRank();
    int getTag (MpiComms *comm);

    void sendMessage (std::string address, std::vector<char> message) { txMessageQueue.emplace (address, std::move (message)); }

    void sendAndReceiveMessages ();
    void drainRemainingMessages ();

private:
    MpiService();
    ~MpiService();
    MpiService(const MpiService&) = delete;
    MpiService& operator=(const MpiService&) = delete;

    int commRank;
    static MPI_Comm mpiCommunicator;
    static bool startServiceThread;

    std::vector<MpiComms*> comms;
    std::list<std::pair<MPI_Request, std::vector<char>>> send_requests;
    BlockingQueue<std::pair<std::string, std::vector<char>>> txMessageQueue;

    bool helics_initialized_mpi;
    std::atomic<int> comms_connected;
    std::atomic<bool> startup_flag;
    std::atomic<bool> stop_service;
    std::unique_ptr<std::thread> service_thread;

    void startService();
    void serviceLoop();

    bool initMPI();
};


} // namespace helics

