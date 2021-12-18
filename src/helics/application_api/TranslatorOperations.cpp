/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "FilterOperations.hpp"

#include "../core/Core.hpp"
#include "../core/core-exceptions.hpp"
#include "../utilities/timeStringOps.hpp"
#include "MessageOperators.hpp"

#include <algorithm>
#include <iostream>
#include <map>
#include <memory>
#include <random>
#include <regex>
#include <thread>

namespace helics {
void FilterOperations::set(const std::string& /*property*/, double /*val*/) {}
void FilterOperations::setString(const std::string& /*property*/, const std::string& /*val*/) {}

DelayFilterOperation::DelayFilterOperation(Time delayTime): delay(delayTime)
{
    if (delayTime < timeZero) {
        delay = timeZero;
    }
    td = std::make_shared<MessageTimeOperator>(
        [this](Time messageTime) { return messageTime + delay; });
}

void DelayFilterOperation::set(const std::string& property, double val)
{
    if (property == "delay") {
        if (val >= timeZero) {
            delay = Time(val);
        }
    }
}

void DelayFilterOperation::setString(const std::string& property, const std::string& val)
{
    if (property == "delay") {
        try {
            delay = gmlc::utilities::loadTimeFromString<Time>(val);
        }
        catch (const std::invalid_argument&) {
            throw(helics::InvalidParameter(val + " is not a valid time string"));
        }
    }
}

std::shared_ptr<FilterOperator> DelayFilterOperation::getOperator()
{
    return std::static_pointer_cast<FilterOperator>(td);
}

/** enumeration of possible random number generator distributions */
enum class random_dists_t : int {
    constant,
    uniform,
    bernoulli,
    binomial,
    geometric,
    poisson,
    exponential,
    gamma,
    weibull,
    extreme_value,
    normal,
    lognormal,
    chi_squared,
    cauchy,
    fisher_f,
    student_t
};

static const std::map<std::string, random_dists_t> distMap{
    {"constant", random_dists_t::constant},
    {"uniform", random_dists_t::uniform},
    {"bernoulli", random_dists_t::bernoulli},
    {"binomial", random_dists_t::binomial},
    {"geometric", random_dists_t::geometric},
    {"poisson", random_dists_t::poisson},
    {"exponential", random_dists_t::exponential},
    {"gamma", random_dists_t::gamma},
    {"weibull", random_dists_t::weibull},
    {"extreme_value", random_dists_t::extreme_value},
    {"normal", random_dists_t::normal},
    {"lognormal", random_dists_t::lognormal},
    {"chi_squared", random_dists_t::chi_squared},
    {"cauchy", random_dists_t::cauchy},
    {"fisher_f", random_dists_t::fisher_f},
    {"student_t", random_dists_t::student_t}};

double randDouble(random_dists_t dist, double p1, double p2)
{
#ifndef __apple_build_version__
    static thread_local std::mt19937 generator(
        std::random_device{}() +
        static_cast<unsigned int>(std::hash<std::thread::id>{}(std::this_thread::get_id())));
#else
#    if __clang_major__ >= 8
    static thread_local std::mt19937 generator(
        std::random_device{}() +
        static_cast<unsigned int>(std::hash<std::thread::id>{}(std::this_thread::get_id())));
#    else
    // this will leak on thread termination,  older apple clang does not have proper thread_local
    // variables so there really isn't any option
    //  static __thread std::mt19937 *genPtr =
    //    new std::mt19937(std::random_device{}() +
    //        static_cast<unsigned int> (std::hash<std::thread::id>{}(std::this_thread::get_id())));

    static __thread std::mt19937* genPtr = nullptr;
    if (genPtr == nullptr) {
        genPtr = new std::mt19937(
            std::random_device{}() +
            static_cast<unsigned int>(std::hash<std::thread::id>{}(std::this_thread::get_id())));
    }
    if (genPtr == nullptr) {
        throw(helics::FunctionExecutionFailure("unable to allocate random generator"));
    }
    auto& generator = *genPtr;

#    endif
#endif
    switch (dist) {
        case random_dists_t::constant:
        default:
            return p1;
        case random_dists_t::uniform: {
            std::uniform_real_distribution<double> distribution(p1, p2);
            return distribution(generator);
        }
        case random_dists_t::normal: {
            std::normal_distribution<double> distribution(p1, p2);
            return distribution(generator);
        }
        case random_dists_t::lognormal: {
            std::lognormal_distribution<double> distribution(p1, p2);
            return distribution(generator);
        }
        case random_dists_t::cauchy: {
            std::cauchy_distribution<double> distribution(p1, p2);
            return distribution(generator);
        }
        case random_dists_t::chi_squared: {
            std::chi_squared_distribution<double> distribution(p1);
            return distribution(generator);
        }
        case random_dists_t::exponential: {
            std::exponential_distribution<double> distribution(p1);
            return distribution(generator);
        }
        case random_dists_t::extreme_value: {
            std::extreme_value_distribution<double> distribution(p1, p2);
            return distribution(generator);
        }
        case random_dists_t::fisher_f: {
            std::fisher_f_distribution<double> distribution(p1, p2);
            return distribution(generator);
        }
        case random_dists_t::weibull: {
            std::weibull_distribution<double> distribution(p1, p2);
            return distribution(generator);
        }
        case random_dists_t::student_t: {
            std::student_t_distribution<double> distribution(p1);
            return distribution(generator);
        }
        case random_dists_t::geometric: {  // integer multiples of some period
            std::geometric_distribution<int> distribution(p1);
            return distribution(generator) * p2;
        }
        case random_dists_t::poisson: {  // integer multiples of some period
            std::poisson_distribution<int> distribution(p1);
            return distribution(generator) * p2;
        }
        case random_dists_t::bernoulli: {
            std::bernoulli_distribution distribution(p1);
            return distribution(generator) ? p2 : 0.0;
        }
        case random_dists_t::binomial: {
            std::binomial_distribution<int> distribution(static_cast<int>(p1), p2);
            return static_cast<double>(distribution(generator));
        }
        case random_dists_t::gamma: {
            std::gamma_distribution<double> distribution(p1, p2);
            return distribution(generator);
        } break;
    }

    // return 0.0;
}

/** class wrapping the distribution generation functions and parameters*/
class randomDelayGenerator {
  public:
    std::atomic<random_dists_t> dist{random_dists_t::uniform};  //!< the distribution
    std::atomic<double> param1{0.0};  //!< parameter 1 typically mean or min
    std::atomic<double> param2{0.0};  //!< parameter 2 typically stddev or max

    double generate() { return randDouble(dist.load(), param1.load(), param2.load()); }
};

RandomDelayFilterOperation::RandomDelayFilterOperation():
    td(std::make_shared<MessageTimeOperator>(
        [this](Time messageTime) { return messageTime + rdelayGen->generate(); })),
    rdelayGen(std::make_unique<randomDelayGenerator>())
{
}
RandomDelayFilterOperation::~RandomDelayFilterOperation() = default;

void RandomDelayFilterOperation::set(const std::string& property, double val)
{
    if ((property == "param1") || (property == "mean") || (property == "min") ||
        (property == "alpha")) {
        rdelayGen->param1.store(val);
    } else if ((property == "param2") || (property == "stddev") || (property == "max") ||
               (property == "beta")) {
        rdelayGen->param2.store(val);
    }
}
void RandomDelayFilterOperation::setString(const std::string& property, const std::string& val)
{
    if ((property == "dist") || (property == "distribution")) {
        auto res = distMap.find(val);
        if (res != distMap.end()) {
            rdelayGen->dist.store(res->second);
        }
    } else if ((property == "param1") || (property == "mean") || (property == "min") ||
               (property == "alpha")) {
        auto tm = gmlc::utilities::loadTimeFromString<Time>(val);
        rdelayGen->param1.store(static_cast<double>(tm));
    } else if ((property == "param2") || (property == "stddev") || (property == "max") ||
               (property == "beta")) {
        auto tm = gmlc::utilities::loadTimeFromString<Time>(val);
        rdelayGen->param2.store(static_cast<double>(tm));
    }
}

std::shared_ptr<FilterOperator> RandomDelayFilterOperation::getOperator()
{
    return std::static_pointer_cast<FilterOperator>(td);
}

RandomDropFilterOperation::RandomDropFilterOperation():
    tcond(std::make_shared<MessageConditionalOperator>([this](const Message*) {
        return (randDouble(random_dists_t::bernoulli, (1.0 - dropProb), 1.0) > 0.1);
    }))
{
}

RandomDropFilterOperation::~RandomDropFilterOperation() = default;
void RandomDropFilterOperation::set(const std::string& property, double val)
{
    if ((property == "dropprob") || (property == "prob")) {
        dropProb = val;
    }
}
void RandomDropFilterOperation::setString(const std::string& /*property*/,
                                          const std::string& /*val*/)
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

void RerouteFilterOperation::set(const std::string& /*property*/, double /*val*/) {}

void RerouteFilterOperation::setString(const std::string& property, const std::string& val)
{
    if (property == "newdestination") {
        newDest = val;
    } else if (property == "condition") {
        try {
            // this line is to verify that it is a valid regex
            auto test = std::regex(val);
            auto cond = conditions.lock();
            cond->insert(val);
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

std::string
    newDestGeneration(const std::string& src, const std::string& dest, const std::string& formula)
{
    if (formula.find_first_of('$') == std::string::npos) {
        return formula;
    }
    std::string newDest = formula;
    std::regex srcreg(R"(\$\{source\})");
    newDest = std::regex_replace(newDest, srcreg, src);
    std::regex destreg(R"(\$\{dest\})");
    newDest = std::regex_replace(newDest, destreg, dest);
    return newDest;
}

std::string RerouteFilterOperation::rerouteOperation(const std::string& src,
                                                     const std::string& dest) const
{
    auto cond = conditions.lock_shared();
    if (cond->empty()) {
        return newDestGeneration(src, dest, newDest.load());
    }

    for (auto& sr : *cond) {
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

void FirewallFilterOperation::set(const std::string& /*property*/, double /*val*/) {}

void FirewallFilterOperation::setString(const std::string& /*property*/, const std::string& /*val*/)
{
}

std::shared_ptr<FilterOperator> FirewallFilterOperation::getOperator()
{
    return std::static_pointer_cast<FilterOperator>(op);
}

bool FirewallFilterOperation::allowPassed(const Message* /*mess*/) const
{
    return true;
}

CloneFilterOperation::CloneFilterOperation():
    op(std::make_shared<CloneOperator>([this](const Message* mess) { return sendMessage(mess); }))
{
}

CloneFilterOperation::~CloneFilterOperation() = default;

void CloneFilterOperation::set(const std::string& property, double /*val*/)
{
    throw(
        helics::InvalidParameter(std::string("property " + property + " is not a known property")));
}

void CloneFilterOperation::setString(const std::string& property, const std::string& val)
{
    if (property == "delivery") {
        auto handle = deliveryAddresses.lock();
        *handle = std::vector<std::string>{val};
    } else if (property == "add delivery") {
        auto handle = deliveryAddresses.lock();
        if (handle->empty()) {
            handle->push_back(val);
        } else {
            auto fnd = std::find(handle->cbegin(), handle->cend(), val);
            if (fnd == handle->cend()) {
                handle->push_back(val);
            }
        }
    } else if (property == "remove delivery") {
        auto handle = deliveryAddresses.lock();
        auto fnd = std::find(handle->cbegin(), handle->cend(), val);
        if (fnd != handle->cend()) {
            handle->erase(fnd);
        }
    } else {
        throw(helics::InvalidParameter(
            std::string("property " + property + " is not a known property")));
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
    for (auto& add : *lock) {
        messages.push_back(std::make_unique<Message>(*mess));
        messages.back()->original_dest = messages.back()->dest;
        messages.back()->dest = add;
    }
    return messages;
}
}  // namespace helics
