/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#pragma once

/**@ file
@detail different signal generators for the source app
*/

#include "Source.hpp"
#include <complex>

namespace helics
{
namespace apps
{
class RampGenerator : public SignalGenerator
{
private:
    double level = 0.0;
    double ramp = 0.0;
public:

    virtual void set(const std::string &parameter, double val);

    virtual defV generate(Time signalTime);
};

class SineGenerator : public SignalGenerator
{
private:
    double level = 0.0;
    double frequency = 0.0;
    double offset = 0.0;
    double Amplitude = 0.0;
    double dAdt = 0.0;
    double dfdt = 0.0;
    double period;
    Time lastCycle = timeZero;
public:

    virtual void set(const std::string &parameter, double val);

    virtual defV generate(Time signalTime);
};

class PhasorGenerator : public SignalGenerator
{
private:
    double bias_real = 0.0;
    double bias_imag = 0.0;
    double frequency = 0.0;
    double offset = 0.0;
    double Amplitude = 0.0;
    double dAdt = 0.0;
    double dfdt = 0.0;
    Time lastCycle = timeZero;
    std::complex<double> state{ 1.0, 0 };
    std::complex<double> rotation{ 1.0,0 };
public:

    virtual void set(const std::string &parameter, double val);

    virtual defV generate(Time signalTime);
};
}
}

