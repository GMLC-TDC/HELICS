/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "Inputs.hpp"

#include "../core/core-exceptions.hpp"
#include "units/units/units.hpp"

#include <algorithm>
#include <string>
#include <vector>

namespace helics {
Input::Input(ValueFederate* valueFed,
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

Input::Input(ValueFederate* valueFed,
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

Input::Input(interface_visibility locality,
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
    catch (const RegistrationFailure&) {
        operator=(valueFed->getInput(key));
        if (!isValid()) {
            throw;
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
            mpark::get<std::function<void(const std::vector<double>&, Time)>>(value_callback)(val,
                                                                                              time);
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
        case 7:  // bool loc
        {
            auto val = getValue<bool>();
            mpark::get<std::function<void(const bool&, Time)>>(value_callback)(val, time);
        } break;
        case 8:  // Time loc
        {
            auto val = getValue<Time>();
            mpark::get<std::function<void(const Time&, Time)>>(value_callback)(val, time);
        } break;
    }
}

template<class X>
    X varMax(const std::vector<defV>& vals)
    {
    X dmax = mpark::get<X>(vals.front());
    for (auto& dval : vals) {
        if (mpark::get<X>(dval) > dmax) {
            dmax = mpark::get<X>(dval);
        }
    }
    return dmax;
    }

template<class X>
    size_t varMaxIndex(const std::vector<defV>& vals,std::function<double(const X &)> op)
    {
        double dmax = invalidDouble;
        size_t index{0};
        size_t mxIndex{0};
        for (auto& dval : vals) {
            auto val = op(mpark::get<X>(dval));
            if (val > dmax) {
                dmax = val;
                mxIndex = index;
            }
            ++index;
        }
        return mxIndex;
    }

    template<class X>
    X varMin(const std::vector<defV>& vals)
    {
        X dmin = mpark::get<X>(vals.front());
        for (auto& dval : vals) {
            if (mpark::get<X>(dval) < dmin) {
                dmin = mpark::get<X>(dval);
            }
        }
        return dmin;
    }

    template<class X>
    size_t varMinIndex(const std::vector<defV>& vals, std::function<double(const X&)> op)
    {
        double dmin = -invalidDouble;
        size_t index{0};
        size_t mnIndex{0};
        for (auto& dval : vals) {
            auto val = op(mpark::get<X>(dval));
            if (val < dmin) {
                dmin = val;
                mnIndex = index;
            }
            ++index;
        }
        return mnIndex;
    }

static defV maxOperation(const std::vector<defV> & vals)
{
    if (vals.empty()) {
        return invalidDouble;
    }
    switch (vals.front().index()) {
        case double_loc:
        default:
            return varMax<double>(vals);
        case int_loc:
            return varMax<int64_t>(vals);
        case string_loc:
            return varMax<std::string>(vals);
        case complex_loc: {
            auto index = varMaxIndex<std::complex<double>>(vals, [](const auto &v){ return std::abs(v); });
            return vals[index];
        }
        case vector_loc: {
            auto index =
                varMaxIndex<std::vector<double>>(vals, [](const auto& v) { return vectorNorm(v); });
            return vals[index];
        };
        case complex_vector_loc: {
            auto index =
                varMaxIndex<std::vector<std::complex<double>>>(vals, [](const auto& v) { return vectorNorm(v); });
            return vals[index];
        } break;
        case named_point_loc: {
            auto index = varMaxIndex<NamedPoint>(vals, [](const auto& v) {
                return v.value;
            });
            return vals[index];
        } break;
    }
}

static defV minOperation(const std::vector<defV>& vals)
{
    if (vals.empty()) {
        return invalidDouble;
    }
    switch (vals.front().index()) {
        case double_loc:
        default:
            return varMin<double>(vals);
        case int_loc:
            return varMin<int64_t>(vals);
        case string_loc:
            return varMin<std::string>(vals);
        case complex_loc: {
            auto index =
                varMinIndex<std::complex<double>>(vals, [](const auto& v) { return std::abs(v); });
            return vals[index];
        }
        case vector_loc: {
            auto index =
                varMinIndex<std::vector<double>>(vals, [](const auto& v) { return vectorNorm(v); });
            return vals[index];
        };
        case complex_vector_loc: {
            auto index = varMinIndex<std::vector<std::complex<double>>>(vals, [](const auto& v) {
                return vectorNorm(v);
            });
            return vals[index];
        } break;
        case named_point_loc: {
            auto index = varMinIndex<NamedPoint>(vals, [](const auto& v) { return v.value; });
            return vals[index];
        } break;
    }
}

static defV vectorSum(const std::vector<defV>& vals)
{
    double result{0.0};
    for (const auto &v : vals)
    {
        const auto& vect = mpark::get<std::vector<double>>(v);
        for (auto &el : vect)
        {
            result += el;
        }
    }
    return result;
}

static defV vectorAvg(const std::vector<defV>& vals)
{
    double result{0.0};
    int N{0};
    for (const auto& v : vals) {
        const auto& vect = mpark::get<std::vector<double>>(v);
        for (auto& el : vect) {
            result += el;
            ++N;
        }
    }
    return result/static_cast<double>(N);
}

bool Input::vectorDataProcess(
    const std::vector<std::shared_ptr<const data_block>>&
    dataV)
{
    if (type == data_type::helics_unknown) {
        loadSourceInformation();
    }
    std::vector<defV> res;
    res.reserve(dataV.size());
    
    for (size_t ii = 0; ii < dataV.size(); ++ii)
    {
        if (dataV[ii])
        {
            if (type == helics::data_type::helics_double) {
                res.push_back(doubleExtractAndConvert(*dataV[ii], inputUnits, outputUnits));
            } else if (type == helics::data_type::helics_int) {
                res.emplace_back();
                integerExtractAndConvert(res.back(), *dataV[ii], inputUnits, outputUnits);
            } else {
                res.emplace_back();
                valueExtract(*dataV[ii], type, res.back());
            }
        }
    }
    data_type targetType = data_type::helics_multi;
    switch (inputVectorOp) {
        case multi_input_mode::and_operation:
        case multi_input_mode::or_operation:
            targetType = data_type::helics_bool;
            break;
        case multi_input_mode::sum_operation:
        case multi_input_mode::average_operation:
        case multi_input_mode::diff_operation:
            targetType = data_type::helics_vector;
            break;
        default:
            break;
    }
    if (targetType == data_type::helics_multi)
    {
        targetType = data_type::helics_double;
    }
    for (auto &ival : res)
    {
        valueConvert(ival, targetType);
    }
    defV result;
    switch (inputVectorOp)
    {
        case multi_input_mode::max_operation:
            result = maxOperation(res);
            break;
        case multi_input_mode::min_operation:
            result = minOperation(res);
            break;
        case multi_input_mode::and_operation:
            result = std::all_of(res.begin(),res.end(),[](auto &val){
                bool res;
                valueExtract(val, res);
                return res;
                                 }) ?
                "1" :
                "0";
            break;
        case multi_input_mode::or_operation:
            result = std::any_of(res.begin(),
                                 res.end(),
                                 [](auto& val) {
                                     bool res;
                                     valueExtract(val, res);
                                     return res;
                                 }) ?
                "1" :
                "0";
            break;
        case multi_input_mode::sum_operation:
            result = vectorSum(res);
            break;
        case multi_input_mode::average_operation:
            result = vectorAvg(res);
            break;
        default:
            break;
    }
    if (changeDetectionEnabled) {
  //  if (changeDetected(lastValue, result, delta)) {
        lastValue = result;
        hasUpdate = true;
 //   }
    }
    else
    {
        lastValue = result;
        hasUpdate = true;
    }
    return hasUpdate;
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
                (void)arg;  // suppress VS2015 warning
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

void Input::setOption(int32_t option, int32_t value)
{
    if (option == helics_handle_option_multi_input_handling_method)
    {
        inputVectorOp = static_cast<multi_input_mode>(value);
    }
    else
    {
        fed->setInterfaceOption(handle, option, value);
    }
    
}

/** get the current value of a flag for the handle*/
int32_t Input::getOption(int32_t option) const
{
    if (option == helics_handle_option_multi_input_handling_method) {
        return static_cast<int32_t>(inputVectorOp);
    }
    else
    {
        return fed->getInterfaceOption(handle, option);
    }
    
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
        const auto& out = getValueRef<std::string>();
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
            const auto& np = getValueRef<NamedPoint>();
            if (np.name.empty()) {
                return 30;  //"#invalid" string +20
            }
            // +20 is just in case the converted string is actually being requested in which case
            // the +20 is for the string representation of a double
            return np.name.size() + 20;
        }
        const auto& out = getValueRef<std::string>();
        return out.size();
    }

    if (lastValue.index() == string_loc) {
        return mpark::get<std::string>(lastValue).size();
    }
    if (lastValue.index() == named_point_loc) {
        const auto& np = mpark::get<NamedPoint>(lastValue);

        if (np.name.empty()) {
            return 30;  //"~length of #invalid" string +20
        }
        // +20 is just in case the converted string is actually being requested in which case the
        // +20 accounts for the string representation of a double
        return np.name.size() + 20;
    }
    const auto& out = getValueRef<std::string>();
    return out.size();
}

size_t Input::getVectorSize()
{
    isUpdated();
    if (hasUpdate && !changeDetectionEnabled) {
        const auto& out = getValueRef<std::vector<double>>();
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
    const auto& out = getValueRef<std::vector<double>>();
    return out.size();
}

void Input::loadSourceInformation()
{
    type = getTypeFromString(fed->getInjectionType(*this));
    if (type == data_type::helics_multi)
    {

    }
    else
    {
        const auto& iunits = fed->getInjectionUnits(*this);
        if (!iunits.empty()) {
            inputUnits = std::make_shared<units::precise_unit>(units::unit_from_string(iunits));
            if (!units::is_valid(*inputUnits)) {
                inputUnits.reset();
            }
        }
    }
    
}

double doubleExtractAndConvert(const data_view& dv,
                               const std::shared_ptr<units::precise_unit>& inputUnits,
                               const std::shared_ptr<units::precise_unit>& outputUnits)
{
    auto V = ValueConverter<double>::interpret(dv);
    if ((inputUnits) && (outputUnits)) {
        V = units::convert(V, *inputUnits, *outputUnits);
    }
    return V;
}

void integerExtractAndConvert(defV& store,
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
    const auto& S = getValueRef<std::string>();
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

}  // namespace helics
