/*

Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
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
    /** deleted copy constructor*/
    MpiService (const MpiService &) = delete;
    /** deleted copy assignment*/
    MpiService &operator= (const MpiService &) = delete;

    static MpiService &getInstance ();
    static void setMpiCommunicator (MPI_Comm communicator);
    static void setStartServiceThread (bool start);

    std::string addMpiComms (MpiComms *comm);
    void removeMpiComms (MpiComms *comm);
    std::string getAddress (MpiComms *comm);
    int getRank ();
    int getTag (MpiComms *comm);

    void sendMessage (std::pair<int, int> address, std::vector<char> message)
    {
        txMessageQueue.emplace (address, std::move (message));
    }

    void sendAndReceiveMessages ();
    void drainRemainingMessages ();

  private:
    MpiService () = default;
    ~MpiService ();

    int commRank = -1;
    static MPI_Comm mpiCommunicator;
    static bool startServiceThread;

    std::mutex mpiDataLock;  //!< lock for the comms and send_requests
    std::vector<MpiComms *> comms;
    std::list<std::pair<MPI_Request, std::vector<char>>> send_requests;
    BlockingQueue<std::pair<std::pair<int, int>, std::vector<char>>> txMessageQueue;

    bool helics_initialized_mpi{false};
    std::atomic<int> comms_connected{0};
    std::atomic<bool> startup_flag{false};
    std::atomic<bool> stop_service{false};
    std::unique_ptr<std::thread> service_thread;

    void startService ();
    void serviceLoop ();

    bool initMPI ();
};

}  // namespace mpi
}  // namespace helics
