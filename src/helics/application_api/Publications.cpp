/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "Publications.hpp"

#include "../core/core-exceptions.hpp"
#include "ValueFederate.hpp"
#include "units/units.hpp"

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace helics {
Publication::Publication(ValueFederate* valueFed,
                         InterfaceHandle id,
                         std::string_view key,
                         std::string_view type,
                         std::string_view units):
    Interface(valueFed, id, key), fed(valueFed), pubUnits(units)
{
    pubType = getTypeFromString(type);
    if (!units.empty()) {
        pubUnitType = std::make_shared<units::precise_unit>(units::unit_from_string(pubUnits));
        if (!units::is_valid(*pubUnitType)) {
            pubUnitType.reset();
        }
    }
}

Publication::Publication(ValueFederate* valueFed,
                         std::string_view key,
                         std::string_view type,
                         std::string_view units)
{
    auto& pub = valueFed->getPublication(key);
    if (pub.isValid()) {
        operator=(pub);
    } else {
        operator=(valueFed->registerPublication(key, type, units));
    }
}

Publication::Publication(InterfaceVisibility locality,
                         ValueFederate* valueFed,
                         std::string_view key,
                         std::string_view type,
                         std::string_view units)
{
    try {
        if (locality == InterfaceVisibility::GLOBAL) {
            operator=(valueFed->registerGlobalPublication(key, type, units));
        } else {
            operator=(valueFed->registerPublication(key, type, units));
        }
    }
    catch (const RegistrationFailure&) {
        operator=(valueFed->getPublication(key));
        if (!isValid()) {
            throw;
        }
    }
}

void Publication::publish(double val)
{
    bool doPublish = (fed != nullptr);
    if (changeDetectionEnabled) {
        if (changeDetected(prevValue, val, delta)) {
            prevValue = val;
        } else {
            doPublish = false;
        }
    }
    if (doPublish) {
        auto db = typeConvert(pubType, val);
        fed->publishBytes(*this, db);
    }
}

void Publication::publishInt(int64_t val)
{
    bool doPublish = (fed != nullptr);
    if (changeDetectionEnabled) {
        if (changeDetected(prevValue, val, delta)) {
            prevValue = val;
        } else {
            doPublish = false;
        }
    }
    if (doPublish) {
        auto db = typeConvert(pubType, val);
        fed->publishBytes(*this, db);
    }
}

void Publication::publish(char val)
{
    switch (pubType) {
        case DataType::HELICS_BOOL:
            publish(!((val == '0') || (val == 'f') || (val == 0) || (val == 'F') || (val == '-')));
            break;
        case DataType::HELICS_STRING:
        case DataType::HELICS_NAMED_POINT:
        case DataType::HELICS_CHAR:
            publishString({&val, 1});
            break;
        default:
            publishInt(static_cast<int64_t>(val));
    }
}

void Publication::publish(Time val)
{
    bool doPublish = (fed != nullptr);
    if (changeDetectionEnabled) {
        if (changeDetected(prevValue, val, delta)) {
            prevValue = val.getBaseTimeCode();
        } else {
            doPublish = false;
        }
    }
    if (doPublish) {
        auto db = typeConvert(pubType, val.getBaseTimeCode());
        fed->publishBytes(*this, db);
    }
}

void Publication::publish(bool val)
{
    bool doPublish = (fed != nullptr);
    std::string_view bstring = val ? "1" : "0";
    if (changeDetectionEnabled) {
        if (changeDetected(prevValue, bstring, delta)) {
            prevValue = std::string(bstring);
        } else {
            doPublish = false;
        }
    }
    if (doPublish) {
        auto db = typeConvert(pubType, bstring);
        fed->publishBytes(*this, db);
    }
}

void Publication::publishString(std::string_view val)
{
    bool doPublish = (fed != nullptr);
    if (changeDetectionEnabled) {
        if (changeDetected(prevValue, val, delta)) {
            prevValue = std::string(val);
        } else {
            doPublish = false;
        }
    }
    if (doPublish) {
        auto db = typeConvert(pubType, val);
        fed->publishBytes(*this, db);
    }
}

void Publication::publish(const std::vector<std::string>& val)
{
    auto buffer = ValueConverter<std::vector<std::string>>::convert(val);
    auto str = ValueConverter<std::string_view>::interpret(buffer);
    bool doPublish = (fed != nullptr);
    if (changeDetectionEnabled) {
        if (changeDetected(prevValue, str, delta)) {
            prevValue = std::string(str);
        } else {
            doPublish = false;
        }
    }
    if (doPublish) {
        fed->publishBytes(*this, buffer);
    }
}

void Publication::publish(const std::vector<double>& val)
{
    bool doPublish = (fed != nullptr);
    if (changeDetectionEnabled) {
        if (changeDetected(prevValue, val, delta)) {
            prevValue = val;
        } else {
            doPublish = false;
        }
    }
    if (doPublish) {
        auto db = typeConvert(pubType, val);
        fed->publishBytes(*this, db);
    }
}

void Publication::publish(const std::vector<std::complex<double>>& val)
{
    bool doPublish = (fed != nullptr);
    if (changeDetectionEnabled) {
        if (changeDetected(prevValue, val, delta)) {
            prevValue = val;
        } else {
            doPublish = false;
        }
    }
    if (doPublish) {
        auto db = typeConvert(pubType, val);
        fed->publishBytes(*this, db);
    }
}

void Publication::publish(const double* vals, int size)
{
    bool doPublish = (fed != nullptr);
    if (changeDetectionEnabled) {
        if (changeDetected(prevValue, vals, size, delta)) {
            prevValue = std::vector<double>(vals, vals + size);
        } else {
            doPublish = false;
        }
    }
    if (doPublish) {
        auto db = typeConvert(pubType, vals, size);
        fed->publishBytes(*this, db);
    }
}

void Publication::publishComplex(const double* vals, int size)
{
    if (changeDetectionEnabled) {
        std::vector<std::complex<double>> CV;
        CV.reserve(size);
        for (int ii = 0; ii < size; ++ii) {
            CV.emplace_back(vals[2 * ii], vals[2 * ii + 1]);
        }
        if (changeDetected(prevValue, CV, delta)) {
            prevValue = CV;
            auto db = typeConvert(pubType, CV);
            fed->publishBytes(*this, db);
        }
    } else {
        auto db = typeConvertComplex(pubType, vals, size);
        fed->publishBytes(*this, db);
    }
}

void Publication::publish(std::complex<double> val)
{
    bool doPublish = (fed != nullptr);
    if (changeDetectionEnabled) {
        if (changeDetected(prevValue, val, delta)) {
            prevValue = val;
        } else {
            doPublish = false;
        }
    }
    if (doPublish) {
        auto db = typeConvert(pubType, val);
        fed->publishBytes(*this, db);
    }
}

void Publication::publish(const NamedPoint& np)
{
    bool doPublish = (fed != nullptr);
    if (changeDetectionEnabled) {
        if (changeDetected(prevValue, np, delta)) {
            prevValue = np;
        } else {
            doPublish = false;
        }
    }
    if (doPublish) {
        auto db = typeConvert(pubType, np);
        fed->publishBytes(*this, db);
    }
}

void Publication::publish(std::string_view field, double val)
{
    bool doPublish = (fed != nullptr);
    if (changeDetectionEnabled) {
        NamedPoint np(field, val);
        if (changeDetected(prevValue, np, delta)) {
            prevValue = std::move(np);
        } else {
            doPublish = false;
        }
    }
    if (doPublish) {
        auto db = typeConvert(pubType, field, val);
        fed->publishBytes(*this, db);
    }
}

void Publication::publish(double val, const std::string& units)
{
    if (units == pubUnits) {
        publish(val);
    }
    auto punit = units::unit_from_string(units);
    if (units::is_valid(punit)) {
        publish(val, punit);
    } else {
        throw(InvalidConversion{});
    }
}
void Publication::publish(double val, const units::precise_unit& units)
{
    if (pubUnitType) {
        publish(units::convert(val, units, *pubUnitType));
    } else {
        publish(val);
    }
}

void Publication::publishDefV(const defV& val)
{
    bool doPublish = (fed != nullptr);
    if (changeDetectionEnabled) {
        if (prevValue != val) {
            prevValue = val;
        } else {
            doPublish = false;
        }
    }
    if (doPublish) {
        auto db = typeConvertDefV(pubType, val);
        fed->publishBytes(*this, db);
    }
}
}  // namespace helics
