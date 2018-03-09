/*
Copyright © 2017-2018, Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once

#include "../CommsBroker.hpp"
#include "../CoreBroker.hpp"

namespace helics
{
namespace ipc
{
class IpcComms;

class IpcBroker final : public CommsBroker<IpcComms, CoreBroker>
{
public:
    /** default constructor*/
    IpcBroker(bool rootBroker = false) noexcept;
    IpcBroker(const std::string &broker_name);

    static void displayHelp(bool local_only = false);
    void initializeFromArgs(int argc, const char *const *argv) override;

    /**destructor*/
    virtual ~IpcBroker();

    virtual std::string getAddress() const override;

private:
    virtual bool brokerConnect() override;

private:
    std::string fileloc;  //!< the name of the file that the shared memory queue is located
    std::string brokerloc;  //!< the name of the shared queue of the broker
    std::string brokername;  //!< the name of the broker
};
} // namespace ipc
}  // namespace helics

