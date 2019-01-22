/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include "MessageOperators.hpp"
#include "../core/flagOperations.hpp"

namespace helics
{
MessageTimeOperator::MessageTimeOperator (std::function<Time (Time)> userTimeFunction)
    : TimeFunction (std::move (userTimeFunction))
{
}

std::unique_ptr<Message> MessageTimeOperator::process (std::unique_ptr<Message> message)
{
    if (TimeFunction)
    {
        message->time = TimeFunction (message->time);
    }
    return message;
}

void MessageTimeOperator::setTimeFunction (std::function<Time (Time)> userTimeFunction)
{
    TimeFunction = std::move (userTimeFunction);
}

MessageDataOperator::MessageDataOperator (std::function<data_view (data_view)> userDataFunction)
    : dataFunction (std::move (userDataFunction))
{
}

void MessageDataOperator::setDataFunction (std::function<data_view (data_view)> userDataFunction)
{
    dataFunction = std::move (userDataFunction);
}

std::unique_ptr<Message> MessageDataOperator::process (std::unique_ptr<Message> message)
{
    if (dataFunction)
    {
        auto dv = dataFunction (data_view (message->data));
        message->data = dv.to_data_block ();
    }
    return message;
}

MessageDestOperator::MessageDestOperator (
  std::function<std::string (const std::string &, const std::string &)> userDestFunction)
    : DestUpdateFunction (std::move (userDestFunction))
{
}

void MessageDestOperator::setDestFunction (
  std::function<std::string (const std::string &, const std::string &)> userDestFunction)
{
    DestUpdateFunction = std::move (userDestFunction);
}

std::unique_ptr<Message> MessageDestOperator::process (std::unique_ptr<Message> message)
{
    if (DestUpdateFunction)
    {
        message->original_dest = message->dest;
        message->dest = DestUpdateFunction (message->source, message->dest);
    }
    return message;
}

MessageConditionalOperator::MessageConditionalOperator (
  std::function<bool(const Message *)> userConditionalFunction)
    : evalFunction (std::move (userConditionalFunction))
{
}

void MessageConditionalOperator::setConditionFunction (
  std::function<bool(const Message *)> userConditionalFunction)
{
    evalFunction = std::move (userConditionalFunction);
}

std::unique_ptr<Message> MessageConditionalOperator::process (std::unique_ptr<Message> message)
{
    if (evalFunction)
    {
        if (evalFunction (message.get ()))
        {
            return message;
        }
        return nullptr;
    }
    return message;
}

CloneOperator::CloneOperator (std::function<void(const Message *)> userCloneFunction)
    : evalFunction (std::move (userCloneFunction))
{
}

void CloneOperator::setCloneFunction (std::function<void(const Message *)> userCloneFunction)
{
    evalFunction = std::move (userCloneFunction);
}

std::unique_ptr<Message> CloneOperator::process (std::unique_ptr<Message> message)
{
    if (evalFunction)
    {
        evalFunction (message.get ());
    }
    return message;
}

FirewallOperator::FirewallOperator (std::function<bool(const Message *)> userCheckFunction)
    : checkFunction (std::move (userCheckFunction))
{
}

void FirewallOperator::setCheckFunction (std::function<bool(const Message *)> userCheckFunction)
{
    checkFunction = std::move (userCheckFunction);
}

std::unique_ptr<Message> FirewallOperator::process (std::unique_ptr<Message> message)
{
    if (checkFunction)
    {
        bool res = checkFunction (message.get ());
        switch (operation)
        {
        case operations::drop:
            if (res)
            {
                message = nullptr;
            }
            break;
        case operations::pass:
            if (!res)
            {
                message = nullptr;
            }
            break;
        case operations::set_flag1:
            if (res)
            {
                setActionFlag (*message, extra_flag1);
            }
            break;
        case operations::set_flag2:
            if (res)
            {
                setActionFlag (*message, extra_flag2);
            }
            break;
        case operations::set_flag3:
            if (res)
            {
                setActionFlag (*message, extra_flag3);
            }
            break;
        case operations::none:
            break;
        }
    }
    return message;
}

}  // namespace helics
