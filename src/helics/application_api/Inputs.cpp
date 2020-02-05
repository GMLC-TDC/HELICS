/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "Inputs.hpp"

#include "../core/core-exceptions.hpp"
#include "units/units/units.hpp"

#include <algorithm>

namespace helics {
Input::Input(
    ValueFederate* valueFed,
    interface_handle id,
    const std::string& actName,
    const std::string& unitsOut):
    fed(valueFed),
    handle(id), actualName(actName)
{
    if (!unitsOut.empty()) {
        outputUnits = std::make_shared<units::precise_unit>(units::unit_from_string(unitsOut));
        if (!units::is_valid(*outputUnits)) {
            outputUnits.reset();
        }
    }
}

Input::Input(
    ValueFederate* valueFed,
    const std::string& key,
    const std::string& defaultType,
    const std::string& units)
{
    auto& inp = valueFed->getInput(key);
    if (inp.isValid()) {
        operator=(inp);
    } else {
        operator=(valueFed->registerInput(key, defaultType, units));
    }
}

Input::Input(
    interface_visibility locality,
    ValueFederate* valueFed,
    const std::string& key,
    const std::string& defaultType,
    const std::string& units)
{
    try {
        if (locality == interface_visibility::global) {
            operator=(valueFed->registerGlobalInput(key, defaultType, units));
        } else {
            operator=(valueFed->registerInput(key, defaultType, units));
        }
    }
    catch (const RegistrationFailure& e) {
        operator=(valueFed->getInput(key));
        if (!isValid()) {
            throw(e);
        }
    }
}

void Input::handleCallback(Time time)
{
    if (!isUpdated()) {
        return;
    }
    switch (value_callback.index()) {
        case double_loc: {
            auto val = getValue<double>();
            mpark::get<std::function<void(const double&, Time)>>(value_callback)(val, time);
        } break;
        case int_loc: {
            auto val = getValue<int64_t>();
            mpark::get<std::function<void(const int64_t&, Time)>>(value_callback)(val, time);
        } break;
        case string_loc:
        default: {
            auto val = getValue<std::string>();
            mpark::get<std::function<void(const std::string&, Time)>>(value_callback)(val, time);
        } break;
        case complex_loc: {
            auto val = getValue<std::complex<double>>();
            mpark::get<std::function<void(const std::complex<double>&, Time)>>(
                value_callback)(val, time);
        } break;
        case vector_loc: {
            auto val = getValue<std::vector<double>>();
            mpark::get<std::function<void(const std::vector<double>&, Time)>>(
                value_callback)(val, time);
        } break;
        case complex_vector_loc: {
            auto val = getValue<std::vector<std::complex<double>>>();
            mpark::get<std::function<void(const std::vector<std::complex<double>>&, Time)>>(
                value_callback)(val, time);
        } break;
        case named_point_loc: {
            auto val = getValue<NamedPoint>();
            mpark::get<std::function<void(const NamedPoint&, Time)>>(value_callback)(val, time);
        } break;
        case 7: // bool loc
        {
            auto val = getValue<bool>();
            mpark::get<std::function<void(const bool&, Time)>>(value_callback)(val, time);
        } break;
        case 8: // Time loc
        {
            auto val = getValue<Time>();
            mpark::get<std::function<void(const Time&, Time)>>(value_callback)(val, time);
        } break;
    }
}

bool Input::checkUpdate(bool assumeUpdate)
{
    if (changeDetectionEnabled) {
        if (assumeUpdate || fed->isUpdated(*this)) {
            auto dv = fed->getValueRaw(*this);
            if (type == data_type::helics_unknown) {
                loadSourceInformation();
            }
            auto visitor = [&, this](auto&& arg) {
                std::remove_reference_t<decltype(arg)> newVal;
                (void)arg; // suppress VS2015 warning
                if (type == helics::data_type::helics_double) {
                    defV val = doubleExtractAndConvert(dv, inputUnits, outputUnits);
                    valueExtract(val, newVal);
                } else if (type == helics::data_type::helics_int) {
                    defV val;
                    integerExtractAndConvert(val, dv, inputUnits, outputUnits);
                    valueExtract(val, newVal);
                } else {
                    valueExtract(dv, type, newVal);
                }

                if (changeDetected(lastValue, newVal, delta)) {
                    lastValue = newVal;
                    hasUpdate = true;
                }
            };
            mpark::visit(visitor, lastValue);
        }
    } else {
        hasUpdate = (hasUpdate || assumeUpdate || fed->isUpdated(*this));
    }
    return hasUpdate;
}

bool Input::isUpdated()
{
    if (hasUpdate) {
        return true;
    }
    return checkUpdate();
}

bool Input::isUpdated() const
{
    if (hasUpdate) {
        return true;
    }
    return fed->isUpdated(*this);
}

void Input::clearUpdate()
{
    hasUpdate = false;
    fed->clearUpdate(*this);
}

size_t Input::getRawSize()
{
    isUpdated();
    auto dv = fed->getValueRaw(*this);
    if (dv.empty()) {
        auto& out = getValueRef<std::string>();
        return out.size();
    }
    return dv.size();
}

data_view Input::getRawValue()
{
    hasUpdate = false;
    return fed->getValueRaw(*this);
}

size_t Input::getStringSize()
{
    isUpdated();
    if (hasUpdate && !changeDetectionEnabled) {
        if (lastValue.index() == named_point_loc) {
            auto& np = getValueRef<NamedPoint>();
            if (np.name.empty()) {
                return 30; //"#invalid" string +20
            }
            //+20 is just in case the converted string is actually being requested in which case the +20 is for
            // the string representation of a double
            return np.name.size() + 20;
        }
        auto& out = getValueRef<std::string>();
        return out.size();
    }

    if (lastValue.index() == string_loc) {
        return mpark::get<std::string>(lastValue).size();
    }
    if (lastValue.index() == named_point_loc) {
        const auto& np = mpark::get<NamedPoint>(lastValue);

        if (np.name.empty()) {
            return 30; //"~length of #invalid" string +20
        }
        //+20 is just in case the converted string is actually being requested in which case it the 20 accounts
        // for the string representation of a double
        return np.name.size() + 20;
    }
    auto& out = getValueRef<std::string>();
    return out.size();
}

size_t Input::getVectorSize()
{
    isUpdated();
    if (hasUpdate && !changeDetectionEnabled) {
        auto& out = getValueRef<std::vector<double>>();
        return out.size();
    }
    switch (lastValue.index()) {
        case double_loc:
        case int_loc:
            return 1;
        case complex_loc:
            return 2;
        case vector_loc:
            return mpark::get<std::vector<double>>(lastValue).size();
        case complex_vector_loc:
            return mpark::get<std::vector<std::complex<double>>>(lastValue).size() * 2;
        default:
            break;
    }
    auto& out = getValueRef<std::vector<double>>();
    return out.size();
}

void Input::loadSourceInformation()
{
    type = getTypeFromString(fed->getInjectionType(*this));
    auto& iunits = fed->getInjectionUnits(*this);
    if (!iunits.empty()) {
        inputUnits = std::make_shared<units::precise_unit>(units::unit_from_string(iunits));
        if (!units::is_valid(*inputUnits)) {
            inputUnits.reset();
        }
    }
}

double doubleExtractAndConvert(
    const data_view& dv,
    const std::shared_ptr<units::precise_unit>& inputUnits,
    const std::shared_ptr<units::precise_unit>& outputUnits)
{
    auto V = ValueConverter<double>::interpret(dv);
    if ((inputUnits) && (outputUnits)) {
        V = units::convert(V, *inputUnits, *outputUnits);
    }
    return V;
}

void integerExtractAndConvert(
    defV& store,
    const data_view& dv,
    const std::shared_ptr<units::precise_unit>& inputUnits,
    const std::shared_ptr<units::precise_unit>& outputUnits)
{
    auto V = ValueConverter<int64_t>::interpret(dv);
    if ((inputUnits) && (outputUnits)) {
        store = units::convert(static_cast<double>(V), *inputUnits, *outputUnits);
    } else {
        store = V;
    }
}

char Input::getValueChar()
{
    if (fed->isUpdated(*this) || (hasUpdate && !changeDetectionEnabled)) {
        auto dv = fed->getValueRaw(*this);
        if (type == data_type::helics_unknown) {
            type = getTypeFromString(fed->getInjectionType(*this));
        }

        if ((type == data_type::helics_string) || (type == data_type::helics_any) ||
            (type == data_type::helics_custom)) {
            std::string out;
            valueExtract(dv, type, out);
            if (changeDetectionEnabled) {
                if (changeDetected(lastValue, out, delta)) {
                    lastValue = out;
                }
            } else {
                lastValue = out;
            }
        } else {
            int64_t out = invalidValue<int64_t>();
            if (type == helics::data_type::helics_double) {
                out = static_cast<int64_t>(doubleExtractAndConvert(dv, inputUnits, outputUnits));
            } else {
                valueExtract(dv, type, out);
            }
            if (changeDetectionEnabled) {
                if (changeDetected(lastValue, out, delta)) {
                    lastValue = out;
                }
            } else {
                lastValue = out;
            }
        }
    }
    char V;
    valueExtract(lastValue, V);
    hasUpdate = false;
    return V;
}

int Input::getValue(double* data, int maxsize)
{
    auto V = getValueRef<std::vector<double>>();
    int length = 0;
    if (data != nullptr && maxsize > 0) {
        length = std::min(static_cast<int>(V.size()), maxsize);
        std::memmove(data, V.data(), length * sizeof(double));
    }

    hasUpdate = false;
    return length;
}

int Input::getValue(char* str, int maxsize)
{
    auto& S = getValueRef<std::string>();
    int length = 0;
    if (str != nullptr && maxsize > 0) {
        length = std::min(static_cast<int>(S.size()), maxsize);
        memcpy(str, S.data(), length);
        if (length == maxsize) {
            str[maxsize - 1] = '\0';
        } else {
            str[length] = '\0';
            ++length;
        }
    }
    hasUpdate = false;
    return length;
}

} // namespace helics
