/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#pragma once

#include "../src/helics/core/ActionMessage.hpp"

#include <deque>

namespace helics
{
namespace test
{
class MessageGenerator
{
  private:
    std::deque<ActionMessage> messages;

  public:
    MessageGenerator () = default;
    MessageGenerator (const std::string &messageFile);
    void loadMessages (const std::string &messageFile);
    void addMessage (ActionMessage message);

    ActionMessage nextMessage ();
};
}  // namespace test
}  // namespace helics
