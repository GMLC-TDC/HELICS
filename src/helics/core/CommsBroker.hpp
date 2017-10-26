/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef COMMS_BROKER_H_
#define COMMS_BROKER_H_
#pragma once
#include <mutex>
#include <atomic>
#include <memory>
#include <string>

namespace helics
{
    class ActionMessage;
    class CommsInterface;

    template <class COMMS, class Broker>
    class CommsBroker :public Broker
    {
       
    protected:
        std::atomic<int> disconnectionStage{ 0 };  //!< the stage of disconnection
        mutable std::mutex dataMutex;
        std::unique_ptr<COMMS> comms;
        std::atomic<bool> initialized_{ false };  //!< atomic protecting local initialization
    public:
        CommsBroker() noexcept;
        CommsBroker(bool isRootObject) noexcept;
        CommsBroker(const std::string &obj_name);
        ~CommsBroker();
    private:
        virtual void brokerDisconnect() override;
    public:
        virtual void transmit(int route_id, const ActionMessage &cmd) override;

        virtual void addRoute(int route_id, const std::string &routeInfo) override;

    };
}
#endif
