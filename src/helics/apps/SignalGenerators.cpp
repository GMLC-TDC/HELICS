/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "SignalGenerators.hpp"

#include <cmath>
#include <string>

constexpr double pi = 3.14159265358979323846;

namespace helics {
namespace apps {
    void RampGenerator::set(const std::string& parameter, double val)
    {
        if (parameter == "level") {
            level = val;
            if (lastTime > keyTime) {
                keyTime = lastTime;
            }
        } else if (parameter == "ramp") {
            ramp = val;
            if (lastTime > keyTime) {
                keyTime = lastTime;
            }
        } else {
            SignalGenerator::set(parameter, val);
        }
    }

    defV RampGenerator::generate(Time signalTime)
    {
        double newVal = level + ramp * (signalTime - keyTime);
        lastTime = signalTime;
        return newVal;
    }

    void SineGenerator::set(const std::string& parameter, double val)
    {
        if ((parameter == "frequency") || (parameter == "freq") || (parameter == "f")) {
            frequency = val;
        } else if (parameter == "period") {
            frequency = 1.0 / val;
        } else if (parameter == "dfdt") {
            dfdt = val;
        } else if (parameter == "dadt") {
            dAdt = val;
        } else if ((parameter == "amplitude") || (parameter == "amp") || (parameter == "a")) {
            amplitude = val;
        } else if (parameter == "level") {
            level = val;
        } else if (parameter == "offset") {
            offset = val;
        } else {
            SignalGenerator::set(parameter, val);
        }
    }

    defV SineGenerator::generate(Time signalTime)
    {
        auto dt = signalTime - lastTime;
        auto tdiff = signalTime - lastCycle;
        // account for the frequency shift
        frequency += dfdt * dt;
        amplitude += dAdt * dt;
        // compute the sine wave component
        double newValue = level + amplitude * sin(2.0 * pi * (frequency * tdiff) + offset);
        period = (frequency > 0.0) ? 1.0 / frequency : 1e36;
        while (tdiff > period) {
            tdiff -= period;
            lastCycle += period;
        }
        lastTime = signalTime;
        return newValue;
    }

    void PhasorGenerator::set(const std::string& parameter, double val)
    {
        if ((parameter == "frequency") || (parameter == "freq") || (parameter == "f")) {
            frequency = val;
        } else if (parameter == "period") {
            frequency = 1.0 / val;
        } else if (parameter == "dfdt") {
            dfdt = val;
        } else if (parameter == "dadt") {
            dAdt = val;
        } else if ((parameter == "amplitude") || (parameter == "amp") || (parameter == "a")) {
            amplitude = val;
        } else if (parameter == "bias_real") {
            bias_real = val;
        } else if (parameter == "bias_imag") {
            bias_imag = val;
        } else if (parameter == "offset") {
            state *= std::polar(1.0, (val - offset));
            offset = val;
        } else {
            SignalGenerator::set(parameter, val);
        }
    }

    void PhasorGenerator::set(const std::string& parameter, std::complex<double> val)
    {
        if ((parameter == "bias") || (parameter == "level")) {
            bias_real = val.real();
            bias_imag = val.imag();
        }
    }

    void PhasorGenerator::setString(const std::string& parameter, const std::string& val)
    {
        auto valc = helicsGetComplex(val);
        if (std::abs(valc) < 1e12) {
            set(parameter, valc);
        } else {
            SignalGenerator::setString(parameter, val);
        }
    }

    defV PhasorGenerator::generate(Time signalTime)
    {
        auto dt = signalTime - lastTime;

        frequency += dfdt * dt;
        amplitude += dAdt * dt;
        rotation = std::polar(1.0, frequency * dt * (2.0 * pi));
        state *= rotation;
        lastTime = signalTime;
        return amplitude * state + std::complex<double>(bias_real, bias_imag);
    }
}  // namespace apps
}  // namespace helics
