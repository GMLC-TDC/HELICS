/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

/**@file
@details different signal generators for the source app
*/

#include "Source.hpp"

#include <complex>
#include <string>

namespace helics {
namespace apps {
    /** generate a ramp function*/
    class RampGenerator: public SignalGenerator {
      private:
        double level = 0.0;  //!< the starting level of the ramp
        double ramp = 0.0;  //!< the ramp rate in  X/sec;

      public:
        RampGenerator() = default;
        explicit RampGenerator(std::string_view name): SignalGenerator(name) {}
        virtual void set(std::string_view parameter, double val) override;

        virtual defV generate(Time signalTime) override;
    };

    /** generate a sinusoidal signal*/
    class SineGenerator: public SignalGenerator {
      private:
        double level = 0.0;  //!< the dc level of the sinusoid
        double frequency = 0.0;  //!< the oscillation rate of the sinusoid
        double offset = 0.0;  //!< the phase offset of the sinusoid
        double amplitude = 0.0;  //!< the Peak amplitude of the sinusoid
        double dAdt = 0.0;  //!< the rate of change of the amplitude
        double dfdt = 0.0;  //!< the rate of change of frequency
        double period = 1e7;  //!< the period of the sinusoid.  the inverse of the frequency
        Time lastCycle = timeZero;

      public:
        SineGenerator() = default;
        explicit SineGenerator(std::string_view name): SignalGenerator(name) {}
        virtual void set(std::string_view parameter, double val) override;

        virtual defV generate(Time signalTime) override;
    };

    /** generate a rotating phasor
@details this is a coupled sinusoidal oscillator*/
    class PhasorGenerator: public SignalGenerator {
      private:
        double bias_real = 0.0;  //!< the bias level in the real component
        double bias_imag = 0.0;  //!< the bias level in the imaginary component
        double frequency = 0.0;  //!< the frequency of the phasor
        double offset = 0.0;  //! the phase offset of the phasor
        double amplitude = 0.0;  //!< the peak amplitude of the phasor
        double dAdt = 0.0;  //!< the rate of change in the amplitude
        double dfdt = 0.0;  //!< the rate of change in the frequency
        // Time lastCycle = timeZero;
        std::complex<double> state{1.0, 0};
        std::complex<double> rotation{1.0, 0};

      public:
        PhasorGenerator() = default;
        explicit PhasorGenerator(std::string_view name): SignalGenerator(name) {}
        virtual void set(std::string_view parameter, double val) override;
        /** set overload for a complex parameter*/
        void set(std::string_view parameter, std::complex<double> val);
        virtual void setString(std::string_view parameter, std::string_view val) override;
        virtual defV generate(Time signalTime) override;
    };
}  // namespace apps
}  // namespace helics
