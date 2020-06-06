/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC. See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "MpiService.h"

#include <iostream>

namespace helics {
namespace mpi {
    MPI_Comm MpiService::mpiCommunicator = MPI_COMM_NULL;
    bool MpiService::startServiceThread = true;

    MpiService& MpiService::getInstance()
    {
        static MpiService instance;
        if (startServiceThread) {
            instance.startService();
        }
        return instance;
    }

    void MpiService::setMpiCommunicator(MPI_Comm communicator) { mpiCommunicator = communicator; }

    void MpiService::setStartServiceThread(bool start) { startServiceThread = start; }

    MpiService::~MpiService()
    {
        // Stop the service thread
        stop_service = true;
        if (service_thread->joinable()) {
            service_thread->join();
        }
    }

    void MpiService::startService()
    {
        if (service_thread == nullptr) {
            startup_flag = true;
            service_thread = std::make_unique<std::thread>(&MpiService::serviceLoop, this);
            while (startup_flag && !stop_service)
                ;
        }
    }

    void MpiService::serviceLoop()
    {
        // Startup/teardown for MPI can all go in separate functions
        // For integrating helics MPI calls with existing MPI applications, user may want to call
        // from their own thread

        // Initialize MPI
        if (initMPI()) {
            // if mpiCommunicator isn't set by user, make it a duplicate of MPI_COMM_WORLD
            if (mpiCommunicator == MPI_COMM_NULL) {
                MPI_Comm_dup(MPI_COMM_WORLD, &mpiCommunicator);
            }

            // set commRank to our process rank
            MPI_Comm_rank(mpiCommunicator, &commRank);
        }

        // signal that we have finished starting
        startup_flag = false;

        std::cout << "Started MPI service loop for rank " << commRank << std::endl;

        // Run as long as we have something in the send queue or the chance of getting something
        while (!stop_service || comms_connected > 0 || !(txMessageQueue.empty())) {
            // send/receive MPI messages
            sendAndReceiveMessages();
            std::this_thread::yield();
        }

        MPI_Barrier(mpiCommunicator);

        // Make sure that receives get posted for any remaining sends
        drainRemainingMessages();

        MPI_Barrier(mpiCommunicator);

        // If HELICS initialized MPI, also finalize MPI
        if (helics_initialized_mpi) {
            // Finalize MPI
            int mpi_initialized;
            MPI_Initialized(&mpi_initialized);

            // Probably not a necessary check, a user using MPI should have also initialized it
            // themselves
            if (mpi_initialized != 0) {
                std::cout << "About to finalize MPI for rank " << commRank << std::endl;
                MPI_Finalize();
                std::cout << "MPI Finalized for rank " << commRank << std::endl;
            }
        }
    }

    std::string MpiService::addMpiComms(MpiComms* comm)
    {
        std::unique_lock<std::mutex> dataLock(mpiDataLock);
        comms.push_back(comm);
        comms_connected++;
        auto tag = comms.size() - 1;
        dataLock.unlock();
        // If somehow this gets called while MPI is still initializing, wait until MPI
        // initialization completes
        while (startup_flag && !stop_service) {
            ;
        }

        // return the rank:tag for the MpiComms object
        return std::to_string(commRank) + ":" + std::to_string(tag);
    }

    void MpiService::removeMpiComms(MpiComms* comm)
    {
        std::unique_lock<std::mutex> dataLock(mpiDataLock);
        for (auto& cm : comms) {
            if (cm == comm) {
                cm = nullptr;
                --comms_connected;
                break;
            }
        }
    }

    std::string MpiService::getAddress(MpiComms* comm)
    {
        std::unique_lock<std::mutex> dataLock(mpiDataLock);
        for (unsigned int i = 0; i < comms.size(); i++) {
            if (comms[i] == comm) {
                // If somehow this gets called while MPI is still initializing, wait until MPI
                // initialization completes
                while (startup_flag && !stop_service) {
                    ;
                }

                return std::to_string(commRank) + ":" + std::to_string(i);
            }
        }

        // Comm not found
        return "";
    }

    int MpiService::getRank()
    {
        // If somehow this gets called while MPI is still initializing, wait until MPI
        // initialization completes
        while (startup_flag && !stop_service) {
            ;
        }

        return commRank;
    }

    int MpiService::getTag(MpiComms* comm)
    {
        std::unique_lock<std::mutex> dataLock(mpiDataLock);
        for (unsigned int i = 0; i < comms.size(); i++) {
            if (comms[i] == comm) {
                return i;
            }
        }

        return -1;
    }

    bool MpiService::initMPI()
    {
        // Initialize MPI with MPI_THREAD_FUNNELED
        int mpi_initialized;
        int mpi_thread_level;

        MPI_Initialized(&mpi_initialized);

        if (mpi_initialized == 0) {
            MPI_Init_thread(nullptr, nullptr, MPI_THREAD_FUNNELED, &mpi_thread_level);

            MPI_Initialized(&mpi_initialized);
            if (mpi_initialized == 0) {
                std::cerr << "MPI initialization failed" << std::endl;
                return false;
            }

            helics_initialized_mpi = true;
        }

        MPI_Query_thread(&mpi_thread_level);

        if (mpi_thread_level < MPI_THREAD_FUNNELED) {
            std::cerr << "MPI_THREAD_FUNNELED support required" << std::endl;
            return false;
        }

        return true;
    }

    void MpiService::sendAndReceiveMessages()
    {
        // Eventually this entire loop should be redone
        // Using fixed size chunks for sending messages would allow posting blocks of irecv requests
        // If we know that a message will get received, a blocking MPI_Wait_any could be used for
        // send requests Also, a method of doing time synchronization using MPI reductions should be
        // added
        std::list<std::pair<MPI_Request, std::vector<char>>> send_requests;
        std::unique_lock<std::mutex> mpilock(mpiDataLock);
        for (unsigned int i = 0; i < comms.size(); i++) {
            // Skip any nullptr entries
            if (comms[i] == nullptr) {
                continue;
            }

            // Handle receives for the MpiComms object
            int message_waiting = 1;
            MPI_Status status;

            while (message_waiting != 0) {
                // Check if there is a messages waiting
                MPI_Iprobe(MPI_ANY_SOURCE, i, mpiCommunicator, &message_waiting, &status);

                if (message_waiting != 0) {
                    // Get the size of the message waiting to be received
                    int recv_size;
                    std::vector<char> buffer;
                    MPI_Get_count(&status, MPI_CHAR, &recv_size);
                    buffer.resize(recv_size);

                    // Post an asynchronous receive
                    MPI_Request req;
                    MPI_Irecv(buffer.data(),
                              static_cast<int>(buffer.size()),
                              MPI_CHAR,
                              status.MPI_SOURCE,
                              status.MPI_TAG,
                              mpiCommunicator,
                              &req);

                    // Wait until the asynchronous receive request has finished
                    int message_received = 0;
                    while (message_received == 0) {
                        MPI_Test(&req, &message_received, MPI_STATUS_IGNORE);
                    }

                    // Deserialized the received action message
                    ActionMessage M(buffer);

                    // Add to the received message queue for the MpiComms object
                    if (comms[i] != nullptr) {
                        comms[i]->getRxMessageQueue().push(M);
                    }
                }
            }
        }

        mpilock.unlock();
        // Send messages from the queue
        auto sendMsg = txMessageQueue.try_pop();
        while (sendMsg) {
            // std::vector<char> msg;
            // std::string address;
            // std::tie (address, msg) = sendMsg.value ();

            MPI_Request req;
            auto sendRequestData =
                std::pair<MPI_Request, std::vector<char>>(req, std::move(sendMsg->second));

            int destRank = sendMsg->first.first;
            int destTag = sendMsg->first.second;

            if (destRank != commRank) {
                send_requests.push_back(std::move(sendRequestData));
                auto& sreq = send_requests.back();
                // Send the message using asynchronous send
                MPI_Isend(sreq.second.data(),
                          static_cast<int>(sreq.second.size()),
                          MPI_CHAR,
                          destRank,
                          destTag,
                          mpiCommunicator,
                          &sreq.first);
            } else {
                mpilock.lock();
                if (comms[destTag] != nullptr) {
                    // Add the message directly to the destination rx queue (same process)
                    ActionMessage M(sendRequestData.second);
                    comms[destTag]->getRxMessageQueue().push(M);
                }
                mpilock.unlock();
            }
            sendMsg = txMessageQueue.try_pop();
        }

        send_requests.remove_if([](std::pair<MPI_Request, std::vector<char>>& req) {
            int send_finished;
            MPI_Test(&req.first, &send_finished, MPI_STATUS_IGNORE);

            if (send_finished == 1) {
                // Any cleanup needed here? Freeing the vector or MPI_Request?
            }

            return (send_finished == 1);
        });
    }

    void MpiService::drainRemainingMessages()
    {
        // Post receives for any waiting sends
        int message_waiting = 1;
        MPI_Status status;
        while (message_waiting != 0) {
            MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, mpiCommunicator, &message_waiting, &status);
            if (message_waiting != 0) {
                // Get the size of the message waiting to be received
                int recv_size;
                std::vector<char> buffer;
                MPI_Get_count(&status, MPI_CHAR, &recv_size);
                buffer.resize(recv_size);

                // Receive the message
                MPI_Recv(buffer.data(),
                         static_cast<int>(buffer.size()),
                         MPI_CHAR,
                         status.MPI_SOURCE,
                         status.MPI_TAG,
                         mpiCommunicator,
                         &status);
            }
        }
    }

}  // namespace mpi
}  // namespace helics
