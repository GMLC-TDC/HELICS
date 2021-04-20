/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "MessageOperators.hpp"

#include "../core/flagOperations.hpp"

#include <utility>

namespace helics {
MessageTimeOperator::MessageTimeOperator(std::function<Time(Time)> userTimeFunction):
    TimeFunction(std::move(userTimeFunction))
{
}

std::unique_ptr<Message> MessageTimeOperator::process(std::unique_ptr<Message> message)
{
    if (TimeFunction) {
        message->time = TimeFunction(message->time);
    }
    return message;
}

void MessageTimeOperator::setTimeFunction(std::function<Time(Time)> userTimeFunction)
{
    TimeFunction = std::move(userTimeFunction);
}

MessageDataOperator::MessageDataOperator(std::function<void(data_block&)> userDataFunction):
    dataFunction(std::move(userDataFunction))
{
}

void MessageDataOperator::setDataFunction(std::function<void(data_block&)> userDataFunction)
{
    dataFunction = std::move(userDataFunction);
}

std::unique_ptr<Message> MessageDataOperator::process(std::unique_ptr<Message> message)
{
    if (dataFunction) {
        dataFunction(message->data);
    }
    return message;
}

MessageDestOperator::MessageDestOperator(
    std::function<std::string(const std::string&, const std::string&)> userDestFunction):
    DestUpdateFunction(std::move(userDestFunction))
{
}

void MessageDestOperator::setDestFunction(
    std::function<std::string(const std::string&, const std::string&)> userDestFunction)
{
    DestUpdateFunction = std::move(userDestFunction);
}

std::unique_ptr<Message> MessageDestOperator::process(std::unique_ptr<Message> message)
{
    if (DestUpdateFunction) {
        if (message->original_dest.empty()) {
            message->original_dest = message->dest;
        }
        message->dest = DestUpdateFunction(message->source, message->dest);
    }
    return message;
}

MessageConditionalOperator::MessageConditionalOperator(
    std::function<bool(const Message*)> userConditionalFunction):
    evalFunction(std::move(userConditionalFunction))
{
}

void MessageConditionalOperator::setConditionFunction(
    std::function<bool(const Message*)> userConditionalFunction)
{
    evalFunction = std::move(userConditionalFunction);
}

std::unique_ptr<Message> MessageConditionalOperator::process(std::unique_ptr<Message> message)
{
    if (evalFunction) {
        if (evalFunction(message.get())) {
            return message;
        }
        return nullptr;
    }
    return message;
}

CloneOperator::CloneOperator(
    std::function<std::vector<std::unique_ptr<Message>>(const Message*)> userCloneFunction):
    evalFunction(std::move(userCloneFunction))
{
}

void CloneOperator::setCloneFunction(
    std::function<std::vector<std::unique_ptr<Message>>(const Message*)> userCloneFunction)
{
    evalFunction = std::move(userCloneFunction);
}

std::unique_ptr<Message> CloneOperator::process(std::unique_ptr<Message> message)
{
    if (evalFunction) {
        auto res = evalFunction(message.get());
        if (res.size() == 1) {
            return std::move(res.front());
        }
    }
    return message;
}

std::vector<std::unique_ptr<Message>> CloneOperator::processVector(std::unique_ptr<Message> message)
{
    if (evalFunction) {
        return evalFunction(message.get());
    }
    return {};
}

FirewallOperator::FirewallOperator(std::function<bool(const Message*)> userCheckFunction):
    checkFunction(std::move(userCheckFunction))
{
}

void FirewallOperator::setCheckFunction(std::function<bool(const Message*)> userCheckFunction)
{
    checkFunction = std::move(userCheckFunction);
}

std::unique_ptr<Message> FirewallOperator::process(std::unique_ptr<Message> message)
{
    if (checkFunction) {
        bool res = checkFunction(message.get());
        switch (operation) {
            case operations::drop:
                if (res) {
                    message = nullptr;
                }
                break;
            case operations::pass:
                if (!res) {
                    message = nullptr;
                }
                break;
            case operations::set_flag1:
                if (res) {
                    setActionFlag(*message, extra_flag1);
                }
                break;
            case operations::set_flag2:
                if (res) {
                    setActionFlag(*message, extra_flag2);
                }
                break;
            case operations::set_flag3:
                if (res) {
                    setActionFlag(*message, extra_flag3);
                }
                break;
            case operations::none:
                break;
        }
    }
    return message;
}

CustomMessageOperator::CustomMessageOperator(
    std::function<std::unique_ptr<Message>(std::unique_ptr<Message>)> userMessageFunction):
    messageFunction(std::move(userMessageFunction))
{
}

void CustomMessageOperator::setMessageFunction(
    std::function<std::unique_ptr<Message>(std::unique_ptr<Message>)> userMessageFunction)
{
    messageFunction = std::move(userMessageFunction);
}

std::unique_ptr<Message> CustomMessageOperator::process(std::unique_ptr<Message> message)
{
    if (messageFunction) {
        return messageFunction(std::move(message));
    }
    return message;
}

}  // namespace helics
