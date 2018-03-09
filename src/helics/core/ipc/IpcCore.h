/*
Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.
*/

#pragma once

#include "../CommonCore.hpp"
#include "../CommsBroker.hpp"
namespace helics
{
namespace ipc
{
class IpcComms;

/** implementation for the core that uses Interprocess messages to communicate*/
class IpcCore final : public CommsBroker<IpcComms, CommonCore>
{
  public:
    /** default constructor*/
    IpcCore () noexcept;
    IpcCore (const std::string &core_name);
    /** destructor*/
    ~IpcCore ();
    virtual void initializeFromArgs (int argc, const char *const *argv) override;

  public:
    virtual std::string getAddress () const override;

  private:
    virtual bool brokerConnect () override;

    std::string fileloc;  //!< the location of the file queue
    std::string brokerloc;  //!< the location of the broker	queue
    std::string brokername;  //!< the name of the broker
};

}  // namespace ipc
}  // namespace helics

