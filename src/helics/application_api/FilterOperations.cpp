/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/

#include "FilterOperations.h"
#include "Filters.hpp"
#include "MessageOperators.h"
#include "../core/core-exceptions.h"

#include <algorithm>
#include <iostream>
#include <map>
#include <memory>
#include <random>
#include <regex>
#include <thread>

namespace helics
{
void FilterOperations::set (const std::string & /*property*/, double /*val*/) {}
void FilterOperations::setString (const std::string & /*property*/, const std::string & /*val*/) {}

DelayFilterOperation::DelayFilterOperation (Time delayTime) : delay (delayTime)
{
    if (delayTime < timeZero)
    {
        delay = timeZero;
    }
    td = std::make_shared<MessageTimeOperator> ([this](Time messageTime) { return messageTime + delay; });
}

void DelayFilterOperation::set (const std::string &property, double val)
{
    if (property == "delay")
    {
        if (val >= timeZero)
        {
            delay = Time (val);
        }
    }
}

std::shared_ptr<FilterOperator> DelayFilterOperation::getOperator ()
{
    return std::static_pointer_cast<FilterOperator> (td);
}

// enumeration of possible random number generator distributions
enum class random_dists_t : int
{
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
  {"constant", random_dists_t::constant},       {"uniform", random_dists_t::uniform},
  {"bernoulli", random_dists_t::bernoulli},     {"binomial", random_dists_t::binomial},
  {"geometric", random_dists_t::geometric},     {"poisson", random_dists_t::poisson},
  {"exponential", random_dists_t::exponential}, {"gamma", random_dists_t::gamma},
  {"weibull", random_dists_t::weibull},         {"extreme_value", random_dists_t::extreme_value},
  {"normal", random_dists_t::normal},           {"lognormal", random_dists_t::lognormal},
  {"chi_squared", random_dists_t::chi_squared}, {"cauchy", random_dists_t::cauchy},
  {"fisher_f", random_dists_t::fisher_f},       {"student_t", random_dists_t::student_t}};

double randDouble (random_dists_t dist, double p1, double p2)
{
#ifndef __apple_build_version__
    static thread_local std::mt19937 generator (
      std::random_device{}() +
      static_cast<unsigned int> (std::hash<std::thread::id>{}(std::this_thread::get_id ())));
#else
#if __clang_major__ >= 8
    static thread_local std::mt19937 generator (
      std::random_device{}() +
      static_cast<unsigned int> (std::hash<std::thread::id>{}(std::this_thread::get_id ())));
#else
    // this will leak on thread termination,  older apple clang does not have proper thread_local variables so
    // there really isn't any option
    //  static __thread std::mt19937 *genPtr =
    //    new std::mt19937(std::random_device{}() +
    //        static_cast<unsigned int> (std::hash<std::thread::id>{}(std::this_thread::get_id())));

    static __thread std::mt19937 *genPtr = nullptr;
    if (genPtr == nullptr)
    {
        genPtr =
          new std::mt19937 (std::random_device{}() +
                            static_cast<unsigned int> (std::hash<std::thread::id>{}(std::this_thread::get_id ())));
    }

    auto &generator = *genPtr;

#endif
#endif
    switch (dist)
    {
    case random_dists_t::constant:
    default:
        return p1;
    case random_dists_t::uniform:
    {
        std::uniform_real_distribution<double> distribution (p1, p2);
        return distribution (generator);
    }
    case random_dists_t::normal:
    {
        std::normal_distribution<double> distribution (p1, p2);
        return distribution (generator);
    }
    case random_dists_t::lognormal:
    {
        std::lognormal_distribution<double> distribution (p1, p2);
        return distribution (generator);
    }
    case random_dists_t::cauchy:
    {
        std::cauchy_distribution<double> distribution (p1, p2);
        return distribution (generator);
    }
    case random_dists_t::chi_squared:
    {
        std::chi_squared_distribution<double> distribution (p1);
        return distribution (generator);
    }
    case random_dists_t::exponential:
    {
        std::exponential_distribution<double> distribution (p1);
        return distribution (generator);
    }
    case random_dists_t::extreme_value:
    {
        std::extreme_value_distribution<double> distribution (p1, p2);
        return distribution (generator);
    }
    case random_dists_t::fisher_f:
    {
        std::fisher_f_distribution<double> distribution (p1, p2);
        return distribution (generator);
    }
    case random_dists_t::weibull:
    {
        std::weibull_distribution<double> distribution (p1, p2);
        return distribution (generator);
    }
    case random_dists_t::student_t:
    {
        std::student_t_distribution<double> distribution (p1);
        return distribution (generator);
    }
    case random_dists_t::geometric:
    {  // integer multiples of some period
        std::geometric_distribution<int> distribution (p1);
        return distribution (generator) * p2;
    }
    case random_dists_t::poisson:
    {  // integer multiples of some period
        std::poisson_distribution<int> distribution (p1);
        return distribution (generator) * p2;
    }
    case random_dists_t::bernoulli:
    {
        std::bernoulli_distribution distribution (p1);
        return distribution (generator) ? p2 : 0.0;
    }
    case random_dists_t::binomial:
    {
        std::binomial_distribution<int> distribution (static_cast<int> (p1), p2);
        return static_cast<double> (distribution (generator));
    }
    case random_dists_t::gamma:
    {
        std::gamma_distribution<double> distribution (p1, p2);
        return distribution (generator);
    }
    break;
    }

    // return 0.0;
}

/** class wrapping the distribution generation functions and parameters*/
class randomDelayGenerator
{
  public:
    std::atomic<random_dists_t> dist;  //!< the distribution
    std::atomic<double> param1{0.0};  //!< parameter 1 typically mean or min
    std::atomic<double> param2{0.0};  //!< parameter 2 typically stddev or max

    double generate () { return randDouble (dist.load (), param1.load (), param2.load ()); }
};

RandomDelayFilterOperation::RandomDelayFilterOperation ()
{
    rdelayGen = std::make_unique<randomDelayGenerator> ();
    td = std::make_shared<MessageTimeOperator> (
      [this](Time messageTime) { return messageTime + rdelayGen->generate (); });
}
RandomDelayFilterOperation::~RandomDelayFilterOperation () = default;

void RandomDelayFilterOperation::set (const std::string &property, double val)
{
    if ((property == "param1") || (property == "mean") || (property == "min") || (property == "alpha"))
    {
        rdelayGen->param1.store (val);
    }
    else if ((property == "param2") || (property == "stddev") || (property == "max") || (property == "beta"))
    {
        rdelayGen->param2.store (val);
    }
}
void RandomDelayFilterOperation::setString (const std::string &property, const std::string &val)
{
    if ((property == "dist") || (property == "distribution"))
    {
        auto res = distMap.find (val);
        if (res != distMap.end ())
        {
            rdelayGen->dist.store (res->second);
        }
    }
}

std::shared_ptr<FilterOperator> RandomDelayFilterOperation::getOperator ()
{
    return std::static_pointer_cast<FilterOperator> (td);
}

RandomDropFilterOperation::RandomDropFilterOperation ()
{
    tcond = std::make_shared<MessageConditionalOperator> (
      [this](const Message *) { return (randDouble (random_dists_t::bernoulli, dropProb, 1.0) > 0.1); });
}

RandomDropFilterOperation::~RandomDropFilterOperation () = default;
void RandomDropFilterOperation::set (const std::string &property, double val)
{
    if ((property == "dropprob") || (property == "prob"))
    {
        dropProb = val;
    }
}
void RandomDropFilterOperation::setString (const std::string & /*property*/, const std::string & /*val*/) {}

std::shared_ptr<FilterOperator> RandomDropFilterOperation::getOperator ()
{
    return std::static_pointer_cast<FilterOperator> (tcond);
}

RerouteFilterOperation::RerouteFilterOperation ()
{
    op =
      std::make_shared<MessageDestOperator> ([this](const std::string &dest) { return rerouteOperation (dest); });
}

RerouteFilterOperation::~RerouteFilterOperation () = default;

void RerouteFilterOperation::set (const std::string & /*property*/, double /*val*/) {}

void RerouteFilterOperation::setString (const std::string &property, const std::string &val)
{
    if (property == "target")
    {
        newTarget = val;
    }
    else if (property == "filter")
    {
        try
        {
            auto test = std::regex (val);
            auto cond = conditions.lock ();
            cond->insert (val);
        }
        catch (const std::regex_error &re)
        {
            std::cerr << "filter expression is not a valid Regular expression " << re.what () << std::endl;
            throw (helics::InvalidParameter (
              (std::string ("filter expression is not a valid Regular expression ") + re.what ()).c_str ()));
        }
    }
}

std::shared_ptr<FilterOperator> RerouteFilterOperation::getOperator ()
{
    return std::static_pointer_cast<FilterOperator> (op);
}

std::string RerouteFilterOperation::rerouteOperation (const std::string &dest) const
{
    auto cond = conditions.lock_shared ();
    if (cond->empty ())
    {
        return newTarget.load ();
    }
    for (auto &sr : *cond)
    {
        std::regex reg (sr);
        if (std::regex_match (dest, reg))
        {
            return newTarget.load ();
        }
    }
    return dest;
}

CloneFilterOperation::CloneFilterOperation (Core *core) : coreptr (core)
{
    op = std::make_shared<CloneOperator> ([this](const Message *mess) { sendMessage (mess); });
}

CloneFilterOperation::~CloneFilterOperation () = default;

void CloneFilterOperation::set (const std::string & /*property*/, double /*val*/) {}

void CloneFilterOperation::setString (const std::string &property, const std::string &val)
{
    if (property == "delivery")
    {
        deliveryAddresses = std::vector<std::string>{val};
    }
    else if (property == "add delivery")
    {
        auto lock = deliveryAddresses.lock ();
        auto fnd = std::find (lock->cbegin (), lock->cend (), val);
        if (fnd == lock->cend ())
        {
            lock->push_back (val);
        }
    }
    else if (property == "remove delivery")
    {
        auto lock = deliveryAddresses.lock ();
        auto fnd = std::find (lock->cbegin (), lock->cend (), val);
        if (fnd != lock->cend ())
        {
            lock->erase (fnd);
        }
    }
}

std::shared_ptr<FilterOperator> CloneFilterOperation::getOperator ()
{
    return std::static_pointer_cast<FilterOperator> (op);
}

void CloneFilterOperation::sendMessage (const Message *mess) const
{
    auto lock = deliveryAddresses.lock_shared ();
    for (auto &add : *lock)
    {
        auto m = std::make_unique<Message> (*mess);
        m->original_dest = m->dest;
        m->dest = add;
        coreptr->sendMessage (direct_send_handle, std::move (m));
    }
}
}  // namespace helics