/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "Inputs.hpp"

#include "../common/JsonProcessingFunctions.hpp"
#include "../core/core-exceptions.hpp"
#include "units/units/units.hpp"

#include <algorithm>
#include <limits>
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
    for (const auto& dval : vals) {
        if (mpark::get<X>(dval) > dmax) {
            dmax = mpark::get<X>(dval);
        }
    }
    return dmax;
}

template<class X, class Y, typename OP>
Y varDiff(const std::vector<defV>& vals, const OP& op)
{
    Y val = op(mpark::get<X>(vals.front()));
    for (size_t ii = 1; ii < vals.size(); ++ii) {
        val = val - op(mpark::get<X>(vals[ii]));
    }
    return val;
}

template<class X>
size_t varMaxIndex(const std::vector<defV>& vals, std::function<double(const X&)> op)
{
    double dmax = -std::numeric_limits<double>::max();
    size_t index{0};
    size_t mxIndex{0};
    for (const auto& dval : vals) {
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
    for (const auto& dval : vals) {
        if (mpark::get<X>(dval) < dmin) {
            dmin = mpark::get<X>(dval);
        }
    }
    return dmin;
}

template<class X>
size_t varMinIndex(const std::vector<defV>& vals, std::function<double(const X&)> op)
{
    double dmin = std::numeric_limits<double>::max();
    size_t index{0};
    size_t mnIndex{0};
    for (const auto& dval : vals) {
        auto val = op(mpark::get<X>(dval));
        if (val < dmin) {
            dmin = val;
            mnIndex = index;
        }
        ++index;
    }
    return mnIndex;
}

static defV maxOperation(const std::vector<defV>& vals)
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
            auto index =
                varMaxIndex<std::complex<double>>(vals, [](const auto& v) { return std::abs(v); });
            return vals[index];
        }
        case vector_loc: {
            auto index =
                varMaxIndex<std::vector<double>>(vals, [](const auto& v) { return vectorNorm(v); });
            return vals[index];
        };
        case complex_vector_loc: {
            auto index = varMaxIndex<std::vector<std::complex<double>>>(vals, [](const auto& v) {
                return vectorNorm(v);
            });
            return vals[index];
        } break;
        case named_point_loc: {
            auto index = varMaxIndex<NamedPoint>(vals, [](const auto& v) { return v.value; });
            return vals[index];
        } break;
    }
}

static defV diffOperation(const std::vector<defV>& vals)
{
    if (vals.empty()) {
        return invalidDouble;
    }
    switch (vals.front().index()) {
        case double_loc:
        default:
            return varDiff<double, double>(vals, [](const double& x) { return x; });
        case int_loc:
            return varDiff<int64_t, int64_t>(vals, [](const int64_t& x) { return x; });
        case string_loc: {
            const auto& val = mpark::get<std::string>(vals.front());
            for (size_t ii = 1; ii < vals.size(); ++ii) {
                if (mpark::get<std::string>(vals[ii]) != val) {
                    return "1";
                }
            }
            return "0";
        }
        case complex_loc: {
            using C = std::complex<double>;
            return varDiff<C, C>(vals, [](const C& x) { return x; });
        }
        case vector_loc: {
            using C = std::vector<double>;
            return varDiff<C, double>(vals, [](const auto& v) { return vectorNorm(v); });
        }
        case complex_vector_loc: {
            using C = std::vector<std::complex<double>>;
            return varDiff<C, double>(vals, [](const auto& v) { return vectorNorm(v); });
        }
        case named_point_loc:
            return varDiff<NamedPoint, double>(vals, [](const NamedPoint& x) { return x.value; });
    }
}

static defV vectorizeOperation(const std::vector<defV>& vals)
{
    switch (vals.front().index()) {
        case vector_loc: {
            if (vals.empty()) {
                return std::vector<double>();
            }
            std::vector<double> res;
            for (const auto& val : vals) {
                const auto& v = mpark::get<std::vector<double>>(val);
                res.insert(res.end(), v.begin(), v.end());
            }
            return res;
        }
        case string_loc: {
            if (vals.empty()) {
                return std::string{};
            }
            Json::Value svect = Json::arrayValue;
            for (const auto& val : vals) {
                svect.append(mpark::get<std::string>(val));
            }

            return generateJsonString(svect);
        }
        case complex_vector_loc: {
            if (vals.empty()) {
                return std::vector<std::complex<double>>();
            }
            std::vector<std::complex<double>> res;
            for (const auto& val : vals) {
                const auto& v = mpark::get<std::vector<std::complex<double>>>(val);
                res.insert(res.end(), v.begin(), v.end());
            }
            return res;
        }
        default:
            return std::vector<double>();
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
    for (const auto& v : vals) {
        const auto& vect = mpark::get<std::vector<double>>(v);
        for (const auto& el : vect) {
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
        for (const auto& el : vect) {
            result += el;
            ++N;
        }
    }
    return result / static_cast<double>(N);
}

static defV vectorDiff(const std::vector<defV>& vals)
{
    std::vector<double> X;
    double start{invalidDouble};
    for (const auto& v : vals) {
        const auto& vect = mpark::get<std::vector<double>>(v);
        for (const auto& el : vect) {
            if (start != invalidDouble) {
                X.push_back(start - el);
            }
            start = el;
        }
    }
    return X;
}

static bool changeDetected(const defV& prevValue, const defV& newVal, double deltaV)
{
    auto visitor = [&](const auto& arg) { return changeDetected(prevValue, arg, deltaV); };
    return mpark::visit(visitor, newVal);
}

bool Input::vectorDataProcess(const std::vector<std::shared_ptr<const data_block>>& dataV)
{
    if (injectionType == data_type::helics_unknown ||
        static_cast<int32_t>(dataV.size()) != prevInputCount) {
        loadSourceInformation();
        prevInputCount = static_cast<int32_t>(dataV.size());
    }
    std::vector<defV> res;
    res.reserve(dataV.size());
    for (size_t ii = 0; ii < dataV.size(); ++ii) {
        if (dataV[ii]) {
            auto localTargetType = (injectionType == helics::data_type::helics_multi) ?
                sourceTypes[ii].first :
                injectionType;

            const auto& localUnits = (multiUnits) ? sourceTypes[ii].second : inputUnits;
            if (localTargetType == helics::data_type::helics_double) {
                res.emplace_back(doubleExtractAndConvert(*dataV[ii], localUnits, outputUnits));
            } else if (localTargetType == helics::data_type::helics_int) {
                res.emplace_back();
                integerExtractAndConvert(res.back(), *dataV[ii], localUnits, outputUnits);
            } else {
                res.emplace_back();
                valueExtract(*dataV[ii], localTargetType, res.back());
            }
        }
    }
    data_type type = data_type::helics_multi;
    switch (inputVectorOp) {
        case multi_input_handling_method::and_operation:
        case multi_input_handling_method::or_operation:
            type = data_type::helics_bool;
            break;
        case multi_input_handling_method::sum_operation:
        case multi_input_handling_method::average_operation:
            type = data_type::helics_vector;
            break;
        case multi_input_handling_method::vectorize_operation:
            switch (targetType) {
                case data_type::helics_string:
                    type = targetType;
                    break;
                case data_type::helics_complex:
                case data_type::helics_complex_vector:
                    type = data_type::helics_complex_vector;
                    break;
                default:
                    type = data_type::helics_vector;
                    break;
            }
            break;
        default:
            type =
                (targetType == data_type::helics_unknown) ? data_type::helics_double : targetType;
            break;
    }
    // convert everything to a uniform type
    for (auto& ival : res) {
        valueConvert(ival, type);
    }
    defV result;
    switch (inputVectorOp) {
        case multi_input_handling_method::max_operation:
            result = maxOperation(res);
            break;
        case multi_input_handling_method::min_operation:
            result = minOperation(res);
            break;
        case multi_input_handling_method::and_operation:
            result = std::all_of(res.begin(),
                                 res.end(),
                                 [](auto& val) {
                                     bool boolResult;
                                     valueExtract(val, boolResult);
                                     return boolResult;
                                 }) ?
                "1" :
                "0";
            break;
        case multi_input_handling_method::or_operation:
            result = std::any_of(res.begin(),
                                 res.end(),
                                 [](auto& val) {
                                     bool boolResult;
                                     valueExtract(val, boolResult);
                                     return boolResult;
                                 }) ?
                "1" :
                "0";
            break;
        case multi_input_handling_method::sum_operation:
            result = vectorSum(res);
            break;
        case multi_input_handling_method::average_operation:
            result = vectorAvg(res);
            break;
        case multi_input_handling_method::diff_operation:
            if (type == data_type::helics_vector) {
                result = vectorDiff(res);
            } else {
                result = diffOperation(res);
            }
            break;
        case multi_input_handling_method::vectorize_operation:
            result = vectorizeOperation(res);
            break;
        default:
            break;
    }
    if (changeDetectionEnabled) {
        if (changeDetected(lastValue, result, delta)) {
            lastValue = result;
            hasUpdate = true;
        } else {
            hasUpdate = false;
        }
    } else {
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
            if (injectionType == data_type::helics_unknown) {
                loadSourceInformation();
            }
            auto visitor = [&, this](auto&& arg) {
                std::remove_reference_t<decltype(arg)> newVal;
                (void)arg;  // suppress VS2015 warning
                if (injectionType == helics::data_type::helics_double) {
                    defV val = doubleExtractAndConvert(dv, inputUnits, outputUnits);
                    valueExtract(val, newVal);
                } else if (injectionType == helics::data_type::helics_int) {
                    defV val;
                    integerExtractAndConvert(val, dv, inputUnits, outputUnits);
                    valueExtract(val, newVal);
                } else {
                    valueExtract(dv, injectionType, newVal);
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
    if (option == helics_handle_option_multi_input_handling_method) {
        inputVectorOp = static_cast<multi_input_handling_method>(value);
    } else {
        fed->setInterfaceOption(handle, option, value);
    }
}

/** get the current value of a flag for the handle*/
int32_t Input::getOption(int32_t option) const
{
    if (option == helics_handle_option_multi_input_handling_method) {
        return static_cast<int32_t>(inputVectorOp);
    }
    return fed->getInterfaceOption(handle, option);
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
    if (allowDirectFederateUpdate()) {
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
    if (allowDirectFederateUpdate()) {
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
    if (targetType == data_type::helics_unknown) {
        targetType = getTypeFromString(fed->getExtractionType(*this));
    }
    multiUnits = false;
    const auto& iType = fed->getInjectionType(*this);
    const auto& iUnits = fed->getInjectionUnits(*this);
    injectionType = getTypeFromString(iType);
    if ((injectionType == data_type::helics_multi) || (!iUnits.empty() && iUnits.front() == '[')) {
        sourceTypes.clear();
        if (injectionType == data_type::helics_multi) {
            auto jvalue = loadJsonStr(iType);
            for (auto& res : jvalue) {
                sourceTypes.emplace_back(getTypeFromString(res.asCString()), nullptr);
            }
        } else {
            auto iValue = loadJsonStr(iUnits);
            sourceTypes.resize(iValue.size(), {injectionType, nullptr});
        }
        if (!iUnits.empty()) {
            if (iUnits.front() == '[') {
                multiUnits = true;
                auto iValue = loadJsonStr(iUnits);
                int ii{0};
                for (auto& res : iValue) {
                    auto str = res.asString();
                    if (!str.empty()) {
                        auto U =
                            std::make_shared<units::precise_unit>(units::unit_from_string(str));
                        if (units::is_valid(*U)) {
                            sourceTypes[ii].second = std::move(U);
                        }
                    }
                    ++ii;
                }
            } else {
                inputUnits = std::make_shared<units::precise_unit>(units::unit_from_string(iUnits));
                if (!units::is_valid(*inputUnits)) {
                    inputUnits.reset();
                } else {
                    for (auto& src : sourceTypes) {
                        src.second = inputUnits;
                    }
                }
            }
        }

    } else {
        if (!iUnits.empty()) {
            inputUnits = std::make_shared<units::precise_unit>(units::unit_from_string(iUnits));
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
    if (fed->isUpdated(*this) || allowDirectFederateUpdate()) {
        auto dv = fed->getValueRaw(*this);
        if (injectionType == data_type::helics_unknown) {
            loadSourceInformation();
        }

        if ((injectionType == data_type::helics_string) ||
            (injectionType == data_type::helics_any) ||
            (injectionType == data_type::helics_custom)) {
            std::string out;
            valueExtract(dv, injectionType, out);
            if (changeDetectionEnabled) {
                if (changeDetected(lastValue, out, delta)) {
                    lastValue = out;
                }
            } else {
                lastValue = out;
            }
        } else {
            int64_t out = invalidValue<int64_t>();
            if (injectionType == helics::data_type::helics_double) {
                out = static_cast<int64_t>(doubleExtractAndConvert(dv, inputUnits, outputUnits));
            } else {
                valueExtract(dv, injectionType, out);
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
