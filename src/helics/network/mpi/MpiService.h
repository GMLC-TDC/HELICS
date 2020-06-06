/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC. See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "../../core/ActionMessage.hpp"
#include "MpiComms.h"
#include "gmlc/containers/BlockingQueue.hpp"
#include "helics/helics-config.h"

#include <atomic>
#include <functional>
#include <list>
#include <memory>
#include <mpi.h>
#include <mutex>
#include <string>
#include <thread>
#include <utility>
#include <vector>

namespace helics {
namespace mpi {
    /** service for using MPI to communicate*/
    class MpiService {
      public:
        /** deleted copy constructor*/
        MpiService(const MpiService&) = delete;
        /** deleted copy assignment*/
        MpiService& operator=(const MpiService&) = delete;

        static MpiService& getInstance();
        static void setMpiCommunicator(MPI_Comm communicator);
        static void setStartServiceThread(bool start);

        std::string addMpiComms(MpiComms* comm);
        void removeMpiComms(MpiComms* comm);
        std::string getAddress(MpiComms* comm);
        int getRank();
        int getTag(MpiComms* comm);

        void sendMessage(std::pair<int, int> address, std::vector<char> message)
        {
            txMessageQueue.emplace(address, std::move(message));
        }

        void sendAndReceiveMessages();
        void drainRemainingMessages();

      private:
        MpiService() = default;
        ~MpiService();

        int commRank = -1;
        static MPI_Comm mpiCommunicator;
        static bool startServiceThread;

        std::mutex mpiDataLock;  //!< lock for the comms and send_requests
        std::vector<MpiComms*> comms;
        gmlc::containers::BlockingQueue<std::pair<std::pair<int, int>, std::vector<char>>>
            txMessageQueue;

        bool helics_initialized_mpi{false};
        std::atomic<int> comms_connected{0};
        std::atomic<bool> startup_flag{false};
        std::atomic<bool> stop_service{false};
        std::unique_ptr<std::thread> service_thread;

        void startService();
        void serviceLoop();

        bool initMPI();
    };

}  // namespace mpi
}  // namespace helics
