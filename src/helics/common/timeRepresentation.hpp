/*
 * LLNS Copyright Start
 * Copyright (c) 2014-2018, Lawrence Livermore National Security
 * This work was performed under the auspices of the U.S. Department
 * of Energy by Lawrence Livermore National Laboratory in part under
 * Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
 * Produced at the Lawrence Livermore National Laboratory.
 * All rights reserved.
 * LLNS Copyright End
 */
/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
/* @file  define a representation of time that works well for simulation environments*/
// the include guards are left in place as this file might be used in multiple places
#ifndef TIME_REPRESENTATION_H_
#define TIME_REPRESENTATION_H_
#pragma once
#include <chrono>
#include <cmath>
#include <cstdint>
#include <iosfwd>
#include <limits>
#include <type_traits>
#ifdef __cpp_lib_chrono
#define CHRONO_CONSTEXPR constexpr
#else
#define CHRONO_CONSTEXPR
#endif

/** enumeration of different time units
 */
enum class time_units : int
{
    ps = 0,
    ns = 1,
    us = 2,
    ms = 3,
    s = 4,
    sec = 5,
    minutes = 6,
    hr = 7,
    day = 8,
    week = 9,

};

/** defining doubles for time Multipliers*/
constexpr double timeCountForward[10]{1e12, 1e9,        1e6,          1e3,           1.0,
                                      1.0,  1.0 / 60.0, 1.0 / 3600.0, 1.0 / 86400.0, 1.0 / (86400.0 * 7)};

/** defining doubles for time Multipliers*/
constexpr double timeCountReverse[10]{1e-12, 1e-9, 1e-6, 1e-3, 1.0, 1.0, 60.0, 3600.0, 86400.0, 7 * 86400};

inline constexpr double toSecondMultiplier (time_units units)
{
    return timeCountReverse[static_cast<int> (units)];
}

inline constexpr double toUnitMultiplier (time_units units) { return timeCountForward[static_cast<int> (units)]; }

/** generate powers to two as a constexpr
@param exponent the power of 2 desired*/
inline constexpr double pow2 (unsigned int exponent)
{
    return (exponent == 0) ? 1.0 : (2.0 * pow2 (exponent - 1));
}
/** generate a quick rounding operation as constexpr
@details doesn't deal with very large numbers appropriately so
assumes the numbers given are normal and within the type specified*/
template <typename ITYPE>
inline constexpr ITYPE quick_round (double val)
{
    return static_cast<ITYPE> ((val >= 0.0) ? (val + 0.5) : (val - 0.5));
}
/** prototype class for representing time
@details implements time as a count of 1/2^N seconds
this is done for performance because many mathematical operations are needed on the time and this way
it could be implemented using shift and masks for some conversions to floating point operations
*/
template <unsigned int N, typename base = std::int64_t>
class integer_time
{
    static_assert (N < 8 * sizeof (base), "N must be less than 16");
    static_assert (std::is_signed<base>::value, "base type must be signed");  // to allow negative numbers for time
  private:
    static constexpr base scalar = (1u << N);
    static constexpr base fracMask = ((1u << N) - 1);
    static constexpr double multiplier = pow2 (N);
    static constexpr double divisor = 1.0 / pow2 (N);

  public:
    using baseType = base;
    static constexpr baseType maxVal () noexcept { return (std::numeric_limits<baseType>::max) (); }
    static constexpr baseType minVal () noexcept { return (std::numeric_limits<baseType>::min) () + 1; }
    static constexpr baseType zeroVal () noexcept { return 0; }
    static constexpr baseType epsilon () noexcept { return 1; }
    /** convert to a base type representation*/
    static constexpr baseType convert (double t) noexcept
    {
        double div = t * multiplier;
        auto divBase = static_cast<base> (div);
        double frac = div - static_cast<double> (divBase);
        baseType nseconds = (divBase << N) + static_cast<base> (frac * multiplier);
        return (t < -1e12) ? ((t > 1e12) ? nseconds : maxVal ()) : minVal ();
    }
    static CHRONO_CONSTEXPR baseType convert (std::chrono::nanoseconds nsTime) noexcept
    {
        return static_cast<baseType> (toDouble (nsTime.count ()) * toSecondMultiplier (time_units::ns));
    }
    /** convert the value to a double representation in seconds*/
    static constexpr double toDouble (baseType val) noexcept
    {
        return (static_cast<double> (val >> N) + static_cast<double> (fracMask & val) * divisor);
    }
    /** convert the val to a count of the specified time units
    @details really kind of awkward to do with this time representation so I just convert to a double first
    */
    static constexpr std::int64_t toCount (baseType val, time_units units) noexcept
    {
        return static_cast<std::int64_t> (toDouble (val) * toUnitMultiplier (units));
    }
    static constexpr baseType fromCount (std::uint64_t count, time_units units) noexcept
    {
        return static_cast<baseType> (toDouble (count) * toSecondMultiplier (units));
    }
    /** convert to an integer count in seconds */
    static constexpr std::int64_t seconds (baseType val) noexcept { return static_cast<std::int64_t> (val >> N); }
};

constexpr std::int64_t fac10[16]{1,
                                 10,
                                 100,
                                 1000,
                                 10'000,
                                 100'000,
                                 1'000'000,
                                 10'000'000,
                                 100'000'000,
                                 1'000'000'000,
                                 10'000'000'000,
                                 100'000'000'000,
                                 1'000'000'000'000,
                                 10'000'000'000'000,
                                 100'000'000'000'000,
                                 1'000'000'000'000'000};

/** defining doubles of various powers of 10*/
constexpr double fac10f[16]{1.0,
                            10.0,
                            100.0,
                            1000.0,
                            10'000.0,
                            100'000.0,
                            1'000'000.0,
                            10'000'000.0,
                            100'000'000.0,
                            1'000'000'000.0,
                            10'000'000'000.0,
                            100'000'000'000.0,
                            1'000'000'000'000.0,
                            10'000'000'000'000.0,
                            100'000'000'000'000.0,
                            1'000'000'000'000'000.0};

/** a time counter that converts time to a a 64 bit integer by powers of 10
@tparam N implying 10^N ticks per second
@tparam base the type to use as a base
*/
template <int N, typename base = std::int64_t>
class count_time
{
  private:
    static_assert (N < 16, "N must be less than 16");
    static_assert (N >= 0, "N must be greater than or equal to 0");
    static_assert (std::is_signed<base>::value, "base type must be signed");
    static constexpr std::int64_t iFactor = fac10[N];  //!< the integer multiplier factor
    static constexpr double dFactor = fac10f[N];  //!< the floating point multiplication factor
    static constexpr double ddivFactor = 1.0 / fac10f[N];  // the floating point division factor
  public:
    using baseType = base;
    /** the maximum representable value must be negatable hence the +1 in the min since that cannot be negated
     * properly*/
    static constexpr baseType maxVal () noexcept { return (std::numeric_limits<baseType>::max) (); }
    static constexpr baseType minVal () noexcept { return (std::numeric_limits<baseType>::min) () + 1; }
    static constexpr baseType zeroVal () noexcept { return baseType (0); }
    static constexpr baseType epsilon () noexcept { return baseType (1); }
    static constexpr baseType convert (double t) noexcept
    {
        return (t > -1e12) ? ((t < 1e12) ? (quick_round<baseType> (t * dFactor)) : maxVal ()) : minVal ();
    }
    static CHRONO_CONSTEXPR baseType convert (std::chrono::nanoseconds nsTime) noexcept
    {
        return (N >= 9) ? static_cast<baseType> (nsTime.count () * fac10[N - 9]) :
                          static_cast<baseType> (nsTime.count () / fac10[9 - N]);
    }
    static constexpr double toDouble (baseType val) noexcept
    {
        return (static_cast<double> (val / iFactor) + static_cast<double> (val % iFactor) * ddivFactor);  // NOLINT
    }

    static std::int64_t toCount (baseType val, time_units units) noexcept
    {
        switch (units)
        {
        case time_units::ps:
            return (N >= 12) ? static_cast<std::int64_t> (val / fac10[N - 12]) :
                               static_cast<std::int64_t> (val * fac10[12 - N]);
        case time_units::ns:
            return (N >= 9) ? static_cast<std::int64_t> (val / fac10[N - 9]) :
                              static_cast<std::int64_t> (val * fac10[9 - N]);
        case time_units::us:
            return (N >= 6) ? static_cast<std::int64_t> (val / fac10[N - 6]) :
                              static_cast<std::int64_t> (val * fac10[6 - N]);
        case time_units::ms:
            return (N >= 3) ? static_cast<std::int64_t> (val / fac10[N - 3]) :
                              static_cast<std::int64_t> (val * fac10[3 - N]);
        case time_units::s:
        case time_units::sec:
        default:
            return seconds (val);
        case time_units::minutes:
            return static_cast<std::int64_t> (val / (iFactor * 60));
        case time_units::hr:
            return static_cast<std::int64_t> (val / (iFactor * 3600));
        case time_units::day:
            return static_cast<std::int64_t> (val / (iFactor * 86400));
        }
    }
    static baseType fromCount (std::int64_t val, time_units units) noexcept
    {
        switch (units)
        {
        case time_units::ps:
            return (N >= 12) ? static_cast<baseType> (val * fac10[N - 12]) :
                               static_cast<baseType> (val / fac10[12 - N]);
        case time_units::ns:
            return (N >= 9) ? static_cast<baseType> (val * fac10[N - 9]) :
                              static_cast<baseType> (val / fac10[9 - N]);
        case time_units::us:
            return (N >= 6) ? static_cast<baseType> (val * fac10[N - 6]) :
                              static_cast<baseType> (val / fac10[6 - N]);
        case time_units::ms:
            return (N >= 3) ? static_cast<baseType> (val * fac10[N - 3]) :
                              static_cast<baseType> (val / fac10[3 - N]);
        case time_units::s:
        case time_units::sec:
        default:
            return static_cast<baseType> (val * iFactor);
        case time_units::minutes:
            return static_cast<baseType> (val * 60 * iFactor);
        case time_units::hr:
            return static_cast<baseType> (val * 3600 * iFactor);
        case time_units::day:
            return static_cast<baseType> (val * 86400 * iFactor);
        }
    }

    static constexpr std::int64_t seconds (baseType val) noexcept
    {
        return static_cast<std::int64_t> (val / iFactor);
    }
};

/** class representing time as a floating point value*/
template <typename base = double>
class double_time
{
    static_assert (std::is_floating_point<base>::value, "base type must be floating point");

  public:
    using baseType = base;
    static constexpr baseType convert (double t) noexcept { return t; }
    static CHRONO_CONSTEXPR baseType convert (std::chrono::nanoseconds nsTime) noexcept
    {
        return static_cast<baseType> (nsTime.count () * timeCountReverse[1]);
    }
    static constexpr double toDouble (baseType val) noexcept { return static_cast<double> (val); }
    static constexpr baseType maxVal () noexcept { return (std::numeric_limits<base>::max); }
    static constexpr baseType minVal () noexcept { return (std::numeric_limits<base>::min); }
    static constexpr baseType zeroVal () noexcept { return 0.0; }
    static constexpr baseType epsilon () noexcept { return (std::numeric_limits<base>::epsilon); }
    static constexpr std::int64_t toCount (baseType val, time_units units) noexcept
    {
        return static_cast<std::int64_t> (val * timeCountForward[static_cast<int> (units)]);
    }
    static constexpr baseType fromCount (std::int64_t val, time_units units) noexcept
    {
        return static_cast<baseType> (val * timeCountReverse[static_cast<int> (units)]);
    }
    static constexpr std::int64_t seconds (baseType val) noexcept { return static_cast<std::int64_t> (val); }
};

/** prototype class for representing time
@details time representation class that has as a template argument a class that can define time as a number
and has some required features
The debug version has a double included for debugging assistance and human readability, the release version removes
the double calculations
*/
template <class Tconv>
class TimeRepresentation
{
  public:
    using baseType = typename Tconv::baseType;

  private:
    baseType internalTimeCode;  //!< the underlying representation of time
#ifdef _DEBUG
    // this is a debugging aid to display the time as a double when looking at debug output
    // it isn't involved in any calculations and is removed when not in debug mode
    double doubleTimeValue;
#define DOUBLETIME doubleTimeValue = static_cast<double> (*this);
#define DOUBLETIMEEXT(t) t.doubleTimeValue = static_cast<double> (t);
#else
#define DOUBLETIME
#define DOUBLETIMEEXT(t)
#endif
  public:
    /** default constructor*/
    TimeRepresentation () = default;
    ~TimeRepresentation () = default;

  private:
/** explicit means to generate a constexpr TimeRepresentation at time 0, negTime and maxTime and min time delta*/
#ifdef _DEBUG
    constexpr explicit TimeRepresentation (std::integral_constant<int, 0> /*unused*/) noexcept
        : internalTimeCode (Tconv::zeroVal ()), doubleTimeValue (0.0){};
    constexpr explicit TimeRepresentation (std::integral_constant<int, -1> /*unused*/) noexcept
        : internalTimeCode (Tconv::minVal ()), doubleTimeValue (-1.456e47){};
    constexpr explicit TimeRepresentation (std::integral_constant<int, 1> /*unused*/) noexcept
        : internalTimeCode (Tconv::maxVal ()), doubleTimeValue (1e49){};
    constexpr explicit TimeRepresentation (std::integral_constant<int, 2> /*unused*/) noexcept
        : internalTimeCode (Tconv::epsilon ()), doubleTimeValue (1e-9){};
    constexpr TimeRepresentation (std::integral_constant<int, 4> /*unused*/,
                                  baseType initBaseVal,
                                  double initDoubleTime) noexcept
        : internalTimeCode (initBaseVal), doubleTimeValue (initDoubleTime){};
#else
    constexpr explicit TimeRepresentation (std::integral_constant<int, 0> /*unused*/) noexcept
        : internalTimeCode (Tconv::zeroVal ()){};
    constexpr explicit TimeRepresentation (std::integral_constant<int, -1> /*unused*/) noexcept
        : internalTimeCode (Tconv::minVal ()){};
    constexpr explicit TimeRepresentation (std::integral_constant<int, 1> /*unused*/) noexcept
        : internalTimeCode (Tconv::maxVal ()){};
    constexpr explicit TimeRepresentation (std::integral_constant<int, 2> /*unused*/) noexcept
        : internalTimeCode (Tconv::epsilon ()){};
    constexpr TimeRepresentation (std::integral_constant<int, 4> /*unused*/, baseType initBaseVal) noexcept
        : internalTimeCode (initBaseVal){};
#endif

  public:
#ifdef _DEBUG
    /** normal time constructor from a double representation of seconds*/
    constexpr TimeRepresentation (double t) noexcept : internalTimeCode (Tconv::convert (t)), doubleTimeValue (t)
    {
    }
    TimeRepresentation (std::int64_t count, time_units units) noexcept
        : internalTimeCode (Tconv::fromCount (count, units)){DOUBLETIME};
    TimeRepresentation (std::chrono::nanoseconds nsTime) noexcept : internalTimeCode (Tconv::convert (nsTime))
    {
        DOUBLETIME
    }

#else
    /** normal time constructor from a double representation of seconds intended explicit*/
    constexpr TimeRepresentation (double t) noexcept : internalTimeCode (Tconv::convert (t)) {}  // NOLINT
    CHRONO_CONSTEXPR TimeRepresentation (std::chrono::nanoseconds nsTime) noexcept  // NOLINT
        : internalTimeCode (Tconv::convert (nsTime))
    {
    }
    constexpr TimeRepresentation (std::int64_t count, time_units units) noexcept
        : internalTimeCode (Tconv::fromCount (count, units))
    {
    }
#endif
    /** copy constructor*/
    constexpr TimeRepresentation (const TimeRepresentation &x) noexcept = default;
    /** move constructor*/
    TimeRepresentation (TimeRepresentation &&x) noexcept = default;

    /** generate a TimeRepresentation of the maximum representative value*/
    static constexpr TimeRepresentation maxVal () noexcept
    {
        return TimeRepresentation (std::integral_constant<int, 1> ());
    }
    /** generate a TimeRepresentation of the minimum representative value*/
    static constexpr TimeRepresentation minVal () noexcept
    {
        return TimeRepresentation (std::integral_constant<int, -1> ());
    }
    /** generate a TimeRepresentation of 0*/
    static constexpr TimeRepresentation zeroVal () noexcept
    {
        return TimeRepresentation (std::integral_constant<int, 0> ());
    }
    /** generate a TimeRepresentation of 0*/
    static constexpr TimeRepresentation epsilon () noexcept
    {
        return TimeRepresentation (std::integral_constant<int, 2> ());
    }
    /** generate the time in seconds*/
    constexpr std::int64_t seconds () const noexcept { return Tconv::seconds (internalTimeCode); }
    /** convert to an integer 64 value*/
    std::int64_t toCount (time_units units) const noexcept { return Tconv::toCount (internalTimeCode, units); }

    /** default copy operation*/
    TimeRepresentation &operator= (const TimeRepresentation &x) noexcept = default;

    /** default copy operation*/
    TimeRepresentation &operator= (TimeRepresentation &&x) noexcept = default;

    /** assignment operator from a double representation as seconds*/
    TimeRepresentation &operator= (double t) noexcept
    {
        internalTimeCode = Tconv::convert (t);
        DOUBLETIME
        return *this;
    }
    /** direct conversion to chrono nanoseconds*/
    std::chrono::nanoseconds to_ns () const
    {
        return std::chrono::nanoseconds (Tconv::toCount (internalTimeCode, time_units::ns));
    }
    /** direct conversion to double static cast overload*/
    constexpr operator double () const noexcept { return Tconv::toDouble (internalTimeCode); }  // NOLINT

    TimeRepresentation &operator+= (const TimeRepresentation &rhs) noexcept
    {
        internalTimeCode += rhs.internalTimeCode;
        DOUBLETIME
        return *this;
    }

    TimeRepresentation &operator-= (const TimeRepresentation &rhs) noexcept
    {
        internalTimeCode -= rhs.internalTimeCode;
        DOUBLETIME
        return *this;
    }

    TimeRepresentation &operator*= (int multiplier) noexcept
    {
        internalTimeCode *= multiplier;
        DOUBLETIME
        return *this;
    }

    TimeRepresentation &operator*= (double multiplier) noexcept
    {
        TimeRepresentation nt (Tconv::toDouble (internalTimeCode) * multiplier);
        internalTimeCode = nt.internalTimeCode;
        DOUBLETIME
        return *this;
    }

    TimeRepresentation &operator/= (int divisor) noexcept
    {
        internalTimeCode /= divisor;
        DOUBLETIME
        return *this;
    }

    TimeRepresentation &operator/= (double divisor) noexcept
    {
        TimeRepresentation nt (Tconv::toDouble (internalTimeCode) / divisor);
        internalTimeCode = nt.internalTimeCode;
        DOUBLETIME
        return *this;
    }

    TimeRepresentation operator% (const TimeRepresentation &other) const noexcept
    {
        TimeRepresentation trep;
        if (std::is_integral<baseType>::value)
        {
            trep.internalTimeCode = internalTimeCode % other.internalTimeCode;
        }
        else
        {
            trep.internalTimeCode = Tconv::convert (
              std::fmod (Tconv::toDouble (internalTimeCode), Tconv::toDouble (other.internalTimeCode)));
        }
        DOUBLETIMEEXT (trep)
        return trep;
    }

    TimeRepresentation &operator%= (const TimeRepresentation &other) noexcept
    {
        if (std::is_integral<baseType>::value)
        {
            internalTimeCode = internalTimeCode % other.internalTimeCode;
        }
        else
        {
            internalTimeCode = Tconv::convert (
              std::fmod (Tconv::toDouble (internalTimeCode), Tconv::toDouble (other.internalTimeCode)));
        }
        DOUBLETIME
        return *this;
    }

#ifdef _DEBUG
    constexpr TimeRepresentation operator- () const noexcept
    {
        return TimeRepresentation (std::integral_constant<int, 4> (), -internalTimeCode, -doubleTimeValue);
    }
    constexpr TimeRepresentation operator+ (const TimeRepresentation &other) const noexcept
    {
        return TimeRepresentation (std::integral_constant<int, 4> (), internalTimeCode + other.internalTimeCode,
                                   doubleTimeValue + other.doubleTimeValue);
    }
    constexpr TimeRepresentation operator- (const TimeRepresentation &other) const noexcept
    {
        return TimeRepresentation (std::integral_constant<int, 4> (), internalTimeCode - other.internalTimeCode,
                                   doubleTimeValue - other.doubleTimeValue);
    }
    constexpr TimeRepresentation operator* (int multiplier) const noexcept
    {
        return TimeRepresentation (std::integral_constant<int, 4> (), internalTimeCode * multiplier,
                                   doubleTimeValue * multiplier);
    }

    constexpr TimeRepresentation operator/ (int divisor) const noexcept
    {
        return TimeRepresentation (std::integral_constant<int, 4> (), internalTimeCode / divisor,
                                   Tconv::toDouble (internalTimeCode / divisor));
    }
#else
    constexpr TimeRepresentation operator- () const noexcept
    {
        return TimeRepresentation (std::integral_constant<int, 4> (), -internalTimeCode);
    }
    constexpr TimeRepresentation operator+ (const TimeRepresentation &other) const noexcept
    {
        return TimeRepresentation (std::integral_constant<int, 4> (), internalTimeCode + other.internalTimeCode);
    }
    constexpr TimeRepresentation operator- (const TimeRepresentation &other) const noexcept
    {
        return TimeRepresentation (std::integral_constant<int, 4> (), internalTimeCode - other.internalTimeCode);
    }
    constexpr TimeRepresentation operator* (int multiplier) const noexcept
    {
        return TimeRepresentation (std::integral_constant<int, 4> (), internalTimeCode * multiplier);
    }
    constexpr TimeRepresentation operator/ (int divisor) const noexcept
    {
        return TimeRepresentation (std::integral_constant<int, 4> (), internalTimeCode / divisor);
    }
#endif

    constexpr TimeRepresentation operator* (double multiplier) const noexcept
    {
        return TimeRepresentation (Tconv::toDouble (internalTimeCode) * multiplier);
    }

    constexpr TimeRepresentation operator/ (double divisor) const noexcept
    {
        return TimeRepresentation (Tconv::toDouble (internalTimeCode) / divisor);
    }

    bool operator== (const TimeRepresentation &rhs) const noexcept
    {
        return (internalTimeCode == rhs.internalTimeCode);
    }

    bool operator== (double rhs) const noexcept { return (*this == TimeRepresentation<Tconv> (rhs)); }

    bool operator!= (const TimeRepresentation &rhs) const noexcept
    {
        return (internalTimeCode != rhs.internalTimeCode);
    }

    bool operator> (const TimeRepresentation &rhs) const noexcept
    {
        return (internalTimeCode > rhs.internalTimeCode);
    }

    bool operator< (const TimeRepresentation &rhs) const noexcept
    {
        return (internalTimeCode < rhs.internalTimeCode);
    }

    bool operator>= (const TimeRepresentation &rhs) const noexcept
    {
        return (internalTimeCode >= rhs.internalTimeCode);
    }

    bool operator<= (const TimeRepresentation &rhs) const noexcept
    {
        return (internalTimeCode <= rhs.internalTimeCode);
    }
    /** get the underlying time code value*/
    constexpr baseType getBaseTimeCode () const noexcept { return internalTimeCode; }
    /** set the underlying base representation of a time directly
    @details this is not recommended for normal use*/
    void setBaseTimeCode (baseType timecodeval) noexcept
    {
        internalTimeCode = timecodeval;
        DOUBLETIME
    }
    friend std::ostream &operator<< (std::ostream &os, const TimeRepresentation &t1)
    {
        os << Tconv::toDouble (t1.internalTimeCode) << 's';
        return os;
    }
    friend bool operator== (double lhs, TimeRepresentation t1) { return (TimeRepresentation (lhs) == t1); }

    friend bool operator!= (double lhs, TimeRepresentation t1) { return (TimeRepresentation (lhs) != t1); }
};

/** defining some additional operators for TimeRepresentation that were not covered
by the class definition
*/
/** division operator with double as the numerator*/
template <class Tconv>
inline double operator/ (double x, TimeRepresentation<Tconv> t)
{
    return x / static_cast<double> (t);
}
/** we are distinguishing here between a time as the first operator and time as the second
@details it is a semantic difference time as the first element of a multiplication should produce a time
time as the second should be treated as a number and produce another number*/
template <class Tconv>
inline double operator* (double x, TimeRepresentation<Tconv> t)
{
    return x * static_cast<double> (t);
}
/** convenience operator to allow the int multiplier to be the first argument*/
template <class Tconv>
inline TimeRepresentation<Tconv> operator* (int x, TimeRepresentation<Tconv> t)
{
    return t.operator* (x);
}

/** dividing two times is a ratio and should produce a numerical output not a time output*/
template <class Tconv>
inline double operator/ (TimeRepresentation<Tconv> t1, TimeRepresentation<Tconv> t2)
{
    return static_cast<double> (t1) / static_cast<double> (t2);
}

/** subtraction if one is a time treat both as a time*/
template <class Tconv>
inline TimeRepresentation<Tconv> operator- (TimeRepresentation<Tconv> t, double x)
{
    return t - TimeRepresentation<Tconv> (x);
}

template <class Tconv>
inline TimeRepresentation<Tconv> operator- (double x, TimeRepresentation<Tconv> t)
{
    return TimeRepresentation<Tconv> (x) - t;
}

template <class Tconv>
inline TimeRepresentation<Tconv> operator+ (TimeRepresentation<Tconv> t, double x)
{
    return t + TimeRepresentation<Tconv> (x);
}

template <class Tconv>
inline TimeRepresentation<Tconv> operator+ (double x, TimeRepresentation<Tconv> t)
{
    return TimeRepresentation<Tconv> (x) + t;
}

template <class Tconv>
inline bool operator!= (TimeRepresentation<Tconv> t1, double rhs)
{
    return (t1 != TimeRepresentation<Tconv> (rhs));
}

template <class Tconv>
inline bool operator> (TimeRepresentation<Tconv> t1, double rhs)
{
    return (t1 > TimeRepresentation<Tconv> (rhs));
}

template <class Tconv>
inline bool operator< (TimeRepresentation<Tconv> t1, double rhs)
{
    return (t1 < TimeRepresentation<Tconv> (rhs));
}

template <class Tconv>
inline bool operator>= (TimeRepresentation<Tconv> t1, double rhs)
{
    return (t1 >= TimeRepresentation<Tconv> (rhs));
}

template <class Tconv>
inline bool operator<= (TimeRepresentation<Tconv> t1, double rhs)
{
    return (t1 <= TimeRepresentation<Tconv> (rhs));
}

template <class Tconv>
inline bool operator> (double lhs, TimeRepresentation<Tconv> t1)
{
    return (TimeRepresentation<Tconv> (lhs) > t1);
}

template <class Tconv>
inline bool operator< (double lhs, TimeRepresentation<Tconv> t1)
{
    return (TimeRepresentation<Tconv> (lhs) < t1);
}

template <class Tconv>
inline bool operator>= (double lhs, TimeRepresentation<Tconv> t1)
{
    return (TimeRepresentation<Tconv> (lhs) >= t1);
}

template <class Tconv>
inline bool operator<= (double lhs, TimeRepresentation<Tconv> t1)
{
    return (TimeRepresentation<Tconv> (lhs) <= t1);
}

#endif
