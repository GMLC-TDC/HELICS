/*
Copyright (c) 2017-2023,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "FilterOperations.hpp"

#include "../core/Core.hpp"
#include "../core/core-exceptions.hpp"
#include "MessageOperators.hpp"
#include "gmlc/utilities/timeStringOps.hpp"

#include <algorithm>
#include <iostream>
#include <map>
#include <memory>
#include <random>
#include <regex>
#include <thread>

namespace helics {
void FilterOperations::set(std::string_view /*property*/, double /*val*/) {}
void FilterOperations::setString(std::string_view /*property*/, std::string_view /*val*/) {}

DelayFilterOperation::DelayFilterOperation(Time delayTime): delay(delayTime)
{
    if (delayTime < timeZero) {
        delay = timeZero;
    }
    td = std::make_shared<MessageTimeOperator>(
        [this](Time messageTime) { return messageTime + delay; });
}

void DelayFilterOperation::set(std::string_view property, double val)
{
    if (property == "delay") {
        if (val >= timeZero) {
            delay = Time(val);
        }
    }
}

void DelayFilterOperation::setString(std::string_view property, std::string_view val)
{
    if (property == "delay") {
        try {
            delay = gmlc::utilities::loadTimeFromString<helics::Time>(val);
        }
        catch (const std::invalid_argument&) {
            throw(helics::InvalidParameter(std::string(val) + " is not a valid time string"));
        }
    }
}

std::shared_ptr<FilterOperator> DelayFilterOperation::getOperator()
{
    return std::static_pointer_cast<FilterOperator>(td);
}

/** enumeration of possible random number generator distributions */
enum class RandomDistributions : int {
    CONSTANT,
    UNIFORM,
    BERNOULLI,
    BINOMIAL,
    GEOMETRIC,
    POISSON,
    EXPONENTIAL,
    GAMMA,
    WEIBULL,
    EXTREME_VALUE,
    NORMAL,
    LOGNORMAL,
    CHI_SQUARED,
    CAUCHY,
    FISHER_F,
    STUDENT_T
};

static const std::map<std::string_view, RandomDistributions> distMap{
    {"constant", RandomDistributions::CONSTANT},
    {"uniform", RandomDistributions::UNIFORM},
    {"bernoulli", RandomDistributions::BERNOULLI},
    {"binomial", RandomDistributions::BINOMIAL},
    {"geometric", RandomDistributions::GEOMETRIC},
    {"poisson", RandomDistributions::POISSON},
    {"exponential", RandomDistributions::EXPONENTIAL},
    {"gamma", RandomDistributions::GAMMA},
    {"weibull", RandomDistributions::WEIBULL},
    {"extreme_value", RandomDistributions::EXTREME_VALUE},
    {"normal", RandomDistributions::NORMAL},
    {"lognormal", RandomDistributions::LOGNORMAL},
    {"chi_squared", RandomDistributions::CHI_SQUARED},
    {"cauchy", RandomDistributions::CAUCHY},
    {"fisher_f", RandomDistributions::FISHER_F},
    {"student_t", RandomDistributions::STUDENT_T}};

double randDouble(RandomDistributions dist, double p1, double p2)
{
    static thread_local std::mt19937 generator(
        std::random_device{}() +
        static_cast<unsigned int>(std::hash<std::thread::id>{}(std::this_thread::get_id())));

    switch (dist) {
        case RandomDistributions::CONSTANT:
        default:
            return p1;
        case RandomDistributions::UNIFORM: {
            std::uniform_real_distribution<double> distribution(p1, p2);
            return distribution(generator);
        }
        case RandomDistributions::NORMAL: {
            std::normal_distribution<double> distribution(p1, p2);
            return distribution(generator);
        }
        case RandomDistributions::LOGNORMAL: {
            std::lognormal_distribution<double> distribution(p1, p2);
            return distribution(generator);
        }
        case RandomDistributions::CAUCHY: {
            std::cauchy_distribution<double> distribution(p1, p2);
            return distribution(generator);
        }
        case RandomDistributions::CHI_SQUARED: {
            std::chi_squared_distribution<double> distribution(p1);
            return distribution(generator);
        }
        case RandomDistributions::EXPONENTIAL: {
            std::exponential_distribution<double> distribution(p1);
            return distribution(generator);
        }
        case RandomDistributions::EXTREME_VALUE: {
            std::extreme_value_distribution<double> distribution(p1, p2);
            return distribution(generator);
        }
        case RandomDistributions::FISHER_F: {
            std::fisher_f_distribution<double> distribution(p1, p2);
            return distribution(generator);
        }
        case RandomDistributions::WEIBULL: {
            std::weibull_distribution<double> distribution(p1, p2);
            return distribution(generator);
        }
        case RandomDistributions::STUDENT_T: {
            std::student_t_distribution<double> distribution(p1);
            return distribution(generator);
        }
        case RandomDistributions::GEOMETRIC: {  // integer multiples of some period
            std::geometric_distribution<int> distribution(p1);
            return distribution(generator) * p2;
        }
        case RandomDistributions::POISSON: {  // integer multiples of some period
            std::poisson_distribution<int> distribution(p1);
            return distribution(generator) * p2;
        }
        case RandomDistributions::BERNOULLI: {
            std::bernoulli_distribution distribution(p1);
            return distribution(generator) ? p2 : 0.0;
        }
        case RandomDistributions::BINOMIAL: {
            std::binomial_distribution<int> distribution(static_cast<int>(p1), p2);
            return static_cast<double>(distribution(generator));
        }
        case RandomDistributions::GAMMA: {
            std::gamma_distribution<double> distribution(p1, p2);
            return distribution(generator);
        } break;
    }

    // return 0.0;
}

/** class wrapping the distribution generation functions and parameters*/
class RandomDelayGenerator {
  public:
    std::atomic<RandomDistributions> dist{RandomDistributions::UNIFORM};  //!< the distribution
    std::atomic<double> param1{0.0};  //!< parameter 1 typically mean or min
    std::atomic<double> param2{0.0};  //!< parameter 2 typically stddev or max

    double generate() const { return randDouble(dist.load(), param1.load(), param2.load()); }
};

RandomDelayFilterOperation::RandomDelayFilterOperation():
    td(std::make_shared<MessageTimeOperator>(
        [this](Time messageTime) { return messageTime + rdelayGen->generate(); })),
    rdelayGen(std::make_unique<RandomDelayGenerator>())
{
}
RandomDelayFilterOperation::~RandomDelayFilterOperation() = default;

void RandomDelayFilterOperation::set(std::string_view property, double val)
{
    if ((property == "param1") || (property == "mean") || (property == "min") ||
        (property == "alpha")) {
        rdelayGen->param1.store(val);
    } else if ((property == "param2") || (property == "stddev") || (property == "max") ||
               (property == "beta")) {
        rdelayGen->param2.store(val);
    }
}
void RandomDelayFilterOperation::setString(std::string_view property, std::string_view val)
{
    if ((property == "dist") || (property == "distribution")) {
        auto res = distMap.find(val);
        if (res != distMap.end()) {
            rdelayGen->dist.store(res->second);
        }
    } else if ((property == "param1") || (property == "mean") || (property == "min") ||
               (property == "alpha")) {
        auto tm = gmlc::utilities::loadTimeFromString<helics::Time>(val);
        rdelayGen->param1.store(static_cast<double>(tm));
    } else if ((property == "param2") || (property == "stddev") || (property == "max") ||
               (property == "beta")) {
        auto tm = gmlc::utilities::loadTimeFromString<helics::Time>(val);
        rdelayGen->param2.store(static_cast<double>(tm));
    }
}

std::shared_ptr<FilterOperator> RandomDelayFilterOperation::getOperator()
{
    return std::static_pointer_cast<FilterOperator>(td);
}

RandomDropFilterOperation::RandomDropFilterOperation():
    tcond(std::make_shared<MessageConditionalOperator>([this](const Message* /*unused*/) {
        return (randDouble(RandomDistributions::BERNOULLI, (1.0 - dropProb), 1.0) > 0.1);
    }))
{
}

RandomDropFilterOperation::~RandomDropFilterOperation() = default;
void RandomDropFilterOperation::set(std::string_view property, double val)
{
    if ((property == "dropprob") || (property == "prob")) {
        dropProb = val;
    }
}
void RandomDropFilterOperation::setString(std::string_view /*property*/, std::string_view /*val*/)
{
}

std::shared_ptr<FilterOperator> RandomDropFilterOperation::getOperator()
{
    return std::static_pointer_cast<FilterOperator>(tcond);
}

RerouteFilterOperation::RerouteFilterOperation():
    op(std::make_shared<MessageDestOperator>(
        [this](const std::string& src, const std::string& dest) {
            return rerouteOperation(src, dest);
        }))
{
}

RerouteFilterOperation::~RerouteFilterOperation() = default;

void RerouteFilterOperation::set(std::string_view /*property*/, double /*val*/) {}

void RerouteFilterOperation::setString(std::string_view property, std::string_view val)
{
    if (property == "newdestination") {
        newDest = val;
    } else if (property == "condition") {
        try {
            // this line is to verify that it is a valid regex
            auto test = std::regex(val.data(), val.size());
            auto cond = conditions.lock();
            cond->emplace(val);
        }
        catch (const std::regex_error& re) {
            std::cerr << "filter expression is not a valid Regular expression " << re.what()
                      << std::endl;
            throw(helics::InvalidParameter(
                std::string("filter expression is not a valid Regular expression ") + re.what()));
        }
    }
}

std::shared_ptr<FilterOperator> RerouteFilterOperation::getOperator()
{
    return std::static_pointer_cast<FilterOperator>(op);
}

std::string newDestGeneration(const std::string& src, const std::string& dest, std::string formula)
{
    if (formula.find_first_of('$') == std::string::npos) {
        return formula;
    }
    std::regex srcreg(R"(\$\{source\})");
    formula = std::regex_replace(formula, srcreg, std::string{src});
    std::regex destreg(R"(\$\{dest\})");
    formula = std::regex_replace(formula, destreg, std::string{dest});
    return formula;
}

std::string RerouteFilterOperation::rerouteOperation(const std::string& src,
                                                     const std::string& dest) const
{
    auto cond = conditions.lock_shared();
    if (cond->empty()) {
        return newDestGeneration(src, dest, newDest.load());
    }

    for (const auto& sr : *cond) {
        std::regex reg(sr);
        if (std::regex_search(dest, reg, std::regex_constants::match_any)) {
            return newDestGeneration(src, dest, newDest.load());
        }
    }
    return dest;
}

FirewallFilterOperation::FirewallFilterOperation():
    op(std::make_shared<FirewallOperator>(
        [this](const Message* mess) { return allowPassed(mess); }))
{
}

FirewallFilterOperation::~FirewallFilterOperation() = default;

void FirewallFilterOperation::set(std::string_view /*property*/, double /*val*/) {}

void FirewallFilterOperation::setString(std::string_view /*property*/, std::string_view /*val*/) {}

std::shared_ptr<FilterOperator> FirewallFilterOperation::getOperator()
{
    return std::static_pointer_cast<FilterOperator>(op);
}

// NOLINTNEXTLINE
bool FirewallFilterOperation::allowPassed(const Message* /*mess*/) const
{
    /* TODO (PT) this has not been completed yet*/
    return true;
}

CloneFilterOperation::CloneFilterOperation():
    op(std::make_shared<CloneOperator>([this](const Message* mess) { return sendMessage(mess); }))
{
}

CloneFilterOperation::~CloneFilterOperation() = default;

void CloneFilterOperation::set(std::string_view property, double /*val*/)
{
    throw(helics::InvalidParameter(std::string("property ") + std::string(property) +
                                   " is not a known property"));
}

void CloneFilterOperation::setString(std::string_view property, std::string_view val)
{
    if (property == "delivery") {
        auto handle = deliveryAddresses.lock();
        handle->clear();
        handle->emplace_back(val);
    } else if (property == "add delivery") {
        auto handle = deliveryAddresses.lock();
        if (handle->empty()) {
            handle->emplace_back(val);
        } else {
            auto fnd = std::find(handle->cbegin(), handle->cend(), val);
            if (fnd == handle->cend()) {
                handle->emplace_back(val);
            }
        }
    } else if (property == "remove delivery") {
        auto handle = deliveryAddresses.lock();
        auto fnd = std::find(handle->cbegin(), handle->cend(), val);
        if (fnd != handle->cend()) {
            handle->erase(fnd);
        }
    } else {
        throw(helics::InvalidParameter(std::string(
            std::string("property ") + std::string(property) + " is not a known property")));
    }
}

std::shared_ptr<FilterOperator> CloneFilterOperation::getOperator()
{
    return std::static_pointer_cast<FilterOperator>(op);
}

std::vector<std::unique_ptr<Message>> CloneFilterOperation::sendMessage(const Message* mess) const
{
    std::vector<std::unique_ptr<Message>> messages;
    auto lock = deliveryAddresses.lock_shared();
    for (const auto& add : *lock) {
        messages.push_back(std::make_unique<Message>(*mess));
        messages.back()->original_dest = messages.back()->dest;
        messages.back()->dest = add;
    }
    return messages;
}
}  // namespace helics
