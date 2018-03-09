/*

Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.
*/
#pragma once

#include "../../common/BlockingQueue.hpp"
#include "../ActionMessage.hpp"
#include "MpiComms.h"
#include "helics/helics-config.h"
#include <atomic>
#include <functional>
#include <list>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include <mpi.h>

namespace helics
{
namespace mpi
{
/** service for using MPI to communicate*/
class MpiService
{
  public:
    static MpiService &getInstance ();
    static void setMpiCommunicator (MPI_Comm communicator);
    static void setStartServiceThread (bool start);

    std::string addMpiComms (MpiComms *comm);
    void removeMpiComms (MpiComms *comm);
    std::string getAddress (MpiComms *comm);
    int getRank ();
    int getTag (MpiComms *comm);

    void sendMessage (std::string address, std::vector<char> message)
    {
        txMessageQueue.emplace (address, std::move (message));
    }

    void sendAndReceiveMessages ();
    void drainRemainingMessages ();

  private:
    MpiService ();
    ~MpiService ();
    MpiService (const MpiService &) = delete;
    MpiService &operator= (const MpiService &) = delete;

    int commRank;
    static MPI_Comm mpiCommunicator;
    static bool startServiceThread;

    std::vector<MpiComms *> comms;
    std::list<std::pair<MPI_Request, std::vector<char>>> send_requests;
    BlockingQueue<std::pair<std::string, std::vector<char>>> txMessageQueue;

    bool helics_initialized_mpi;
    std::atomic<int> comms_connected;
    std::atomic<bool> startup_flag;
    std::atomic<bool> stop_service;
    std::unique_ptr<std::thread> service_thread;

    void startService ();
    void serviceLoop ();

    bool initMPI ();
};

} // namespace mpi
}  // namespace helics

