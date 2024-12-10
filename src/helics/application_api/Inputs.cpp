/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "Inputs.hpp"

#include "../common/JsonProcessingFunctions.hpp"
#include "../core/core-exceptions.hpp"
#include "ValueFederate.hpp"
#include "units/units.hpp"

#include <algorithm>
#include <limits>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace helics {
Input::Input(ValueFederate* valueFed,
             InterfaceHandle id,
             std::string_view actName,
             std::string_view unitsOut): Interface(valueFed, id, actName), fed(valueFed)
{
    if (!unitsOut.empty()) {
        outputUnits =
            std::make_shared<units::precise_unit>(units::unit_from_string(std::string{unitsOut}));
        if (!units::is_valid(*outputUnits)) {
            outputUnits.reset();
        }
    }
}

Input::Input(ValueFederate* valueFed,
             std::string_view key,
             std::string_view defaultType,
             std::string_view units)
{
    auto& inp = valueFed->getInput(key);
    if (inp.isValid()) {
        operator=(inp);
    } else {
        operator=(valueFed->registerInput(key, defaultType, units));
    }
}

Input::Input(InterfaceVisibility locality,
             ValueFederate* valueFed,
             std::string_view key,
             std::string_view defaultType,
             std::string_view units)
{
    try {
        if (locality == InterfaceVisibility::GLOBAL) {
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

void Input::setDefaultBytes(data_view val)
{
    fed->setDefaultValue(*this, std::move(val));
}

void Input::handleCallback(Time time)
{
    if (!isUpdated()) {
        return;
    }
    switch (value_callback.index()) {
        case double_loc: {
            auto val = getValue<double>();
            std::get<std::function<void(const double&, Time)>>(value_callback)(val, time);
        } break;
        case int_loc: {
            auto val = getValue<int64_t>();
            std::get<std::function<void(const int64_t&, Time)>>(value_callback)(val, time);
        } break;
        case string_loc:
        default: {
            auto val = getValue<std::string>();
            std::get<std::function<void(const std::string&, Time)>>(value_callback)(val, time);
        } break;
        case complex_loc: {
            auto val = getValue<std::complex<double>>();
            std::get<std::function<void(const std::complex<double>&, Time)>>(value_callback)(val,
                                                                                             time);
        } break;
        case vector_loc: {
            auto val = getValue<std::vector<double>>();
            std::get<std::function<void(const std::vector<double>&, Time)>>(value_callback)(val,
                                                                                            time);
        } break;
        case complex_vector_loc: {
            auto val = getValue<std::vector<std::complex<double>>>();
            std::get<std::function<void(const std::vector<std::complex<double>>&, Time)>>(
                value_callback)(val, time);
        } break;
        case named_point_loc: {
            auto val = getValue<NamedPoint>();
            std::get<std::function<void(const NamedPoint&, Time)>>(value_callback)(val, time);
        } break;
        case 7:  // bool loc
        {
            auto val = getValue<bool>();
            std::get<std::function<void(const bool&, Time)>>(value_callback)(val, time);
        } break;
        case 8:  // Time loc
        {
            auto val = getValue<Time>();
            std::get<std::function<void(const Time&, Time)>>(value_callback)(val, time);
        } break;
    }
}

template<class X>
X varMax(const std::vector<defV>& vals)
{
    X dmax = std::get<X>(vals.front());
    for (const auto& dval : vals) {
        if (std::get<X>(dval) > dmax) {
            dmax = std::get<X>(dval);
        }
    }
    return dmax;
}

template<class X, class Y, typename OP>
Y varDiff(const std::vector<defV>& vals, const OP& op)
{
    Y val = op(std::get<X>(vals.front()));
    for (size_t ii = 1; ii < vals.size(); ++ii) {
        val = val - op(std::get<X>(vals[ii]));
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
        auto val = op(std::get<X>(dval));
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
    X dmin = std::get<X>(vals.front());
    for (const auto& dval : vals) {
        if (std::get<X>(dval) < dmin) {
            dmin = std::get<X>(dval);
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
        auto val = op(std::get<X>(dval));
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
            const auto& val = std::get<std::string>(vals.front());
            for (size_t ii = 1; ii < vals.size(); ++ii) {
                if (std::get<std::string>(vals[ii]) != val) {
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
                const auto& v = std::get<std::vector<double>>(val);
                res.insert(res.end(), v.begin(), v.end());
            }
            return res;
        }
        case string_loc: {
            if (vals.empty()) {
                return std::string{};
            }
            nlohmann::json svect = nlohmann::json::array();
            for (const auto& val : vals) {
                svect.push_back(std::get<std::string>(val));
            }

            return fileops::generateJsonString(svect);
        }
        case complex_vector_loc: {
            if (vals.empty()) {
                return std::vector<std::complex<double>>();
            }
            std::vector<std::complex<double>> res;
            for (const auto& val : vals) {
                const auto& v = std::get<std::vector<std::complex<double>>>(val);
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
        const auto& vect = std::get<std::vector<double>>(v);
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
        const auto& vect = std::get<std::vector<double>>(v);
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
        const auto& vect = std::get<std::vector<double>>(v);
        for (const auto& el : vect) {
            if (start != invalidDouble) {
                X.push_back(start - el);
            }
            start = el;
        }
    }
    return X;
}

static defV sumOperation(const std::vector<defV>& vals)
{
    if (vals.empty()) {
        return std::string{};
    }
    if (vals.front().index() == string_loc) {
        std::string output;
        for (const auto& v : vals) {
            std::string valOutput;
            valueExtract(v, valOutput);
            output.append(valOutput);
        }
        return output;
    } else {
        return vectorSum(vals);
    }
}

static bool changeDetected(const defV& prevValue, const defV& newVal, double deltaV)
{
    auto visitor = [&](const auto& arg) { return changeDetected(prevValue, arg, deltaV); };
    return std::visit(visitor, newVal);
}

bool Input::vectorDataProcess(const std::vector<std::shared_ptr<const SmallBuffer>>& dataV)
{
    if (injectionType == DataType::HELICS_UNKNOWN ||
        static_cast<int32_t>(dataV.size()) != prevInputCount) {
        loadSourceInformation();
        prevInputCount = static_cast<int32_t>(dataV.size());
    }
    std::vector<defV> res;
    res.reserve(dataV.size());
    for (size_t ii = 0; ii < dataV.size(); ++ii) {
        if (dataV[ii]) {
            auto localTargetType = (injectionType == helics::DataType::HELICS_MULTI) ?
                sourceTypes[ii].first :
                injectionType;

            const auto& localUnits = (multiUnits) ? sourceTypes[ii].second : inputUnits;
            if (localTargetType == helics::DataType::HELICS_DOUBLE) {
                res.emplace_back(doubleExtractAndConvert(*dataV[ii], localUnits, outputUnits));
            } else if (localTargetType == helics::DataType::HELICS_INT) {
                res.emplace_back();
                integerExtractAndConvert(res.back(), *dataV[ii], localUnits, outputUnits);
            } else {
                res.emplace_back();
                valueExtract(*dataV[ii], localTargetType, res.back());
            }
        }
    }
    DataType type = DataType::HELICS_MULTI;
    switch (inputVectorOp) {
        case MultiInputHandlingMethod::AND_OPERATION:
        case MultiInputHandlingMethod::OR_OPERATION:
            type = DataType::HELICS_BOOL;
            break;
        case MultiInputHandlingMethod::AVERAGE_OPERATION:
            type = DataType::HELICS_VECTOR;
            break;
        case MultiInputHandlingMethod::SUM_OPERATION:
            switch (targetType) {
                case DataType::HELICS_STRING:
                case DataType::HELICS_CHAR:
                    type = DataType::HELICS_STRING;
                    break;
                default:
                    type = DataType::HELICS_VECTOR;
                    break;
            }
            break;
        case MultiInputHandlingMethod::VECTORIZE_OPERATION:
            switch (targetType) {
                case DataType::HELICS_STRING:
                case DataType::HELICS_CHAR:
                    type = DataType::HELICS_STRING;
                    break;
                case DataType::HELICS_COMPLEX:
                case DataType::HELICS_COMPLEX_VECTOR:
                    type = DataType::HELICS_COMPLEX_VECTOR;
                    break;
                default:
                    type = DataType::HELICS_VECTOR;
                    break;
            }
            break;
        default:
            type = (targetType == DataType::HELICS_UNKNOWN) ? DataType::HELICS_DOUBLE : targetType;
            break;
    }
    // convert everything to a uniform type
    for (auto& ival : res) {
        valueConvert(ival, type);
    }
    defV result;
    switch (inputVectorOp) {
        case MultiInputHandlingMethod::MAX_OPERATION:
            result = maxOperation(res);
            break;
        case MultiInputHandlingMethod::MIN_OPERATION:
            result = minOperation(res);
            break;
        case MultiInputHandlingMethod::AND_OPERATION:
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
        case MultiInputHandlingMethod::OR_OPERATION:
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
        case MultiInputHandlingMethod::SUM_OPERATION:
            result = sumOperation(res);
            break;
        case MultiInputHandlingMethod::AVERAGE_OPERATION:
            result = vectorAvg(res);
            break;
        case MultiInputHandlingMethod::DIFF_OPERATION:
            if (type == DataType::HELICS_VECTOR) {
                result = vectorDiff(res);
            } else {
                result = diffOperation(res);
            }
            break;
        case MultiInputHandlingMethod::VECTORIZE_OPERATION:
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
            auto dv = fed->getBytes(*this);
            if (injectionType == DataType::HELICS_UNKNOWN) {
                loadSourceInformation();
            }
            auto visitor = [&, this](auto&& arg) {
                std::remove_reference_t<decltype(arg)> newVal;
                (void)arg;  // suppress VS2015 warning
                if (injectionType == helics::DataType::HELICS_DOUBLE) {
                    defV val = doubleExtractAndConvert(dv, inputUnits, outputUnits);
                    valueExtract(val, newVal);
                } else if (injectionType == helics::DataType::HELICS_INT) {
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
            std::visit(visitor, lastValue);
        }
    } else {
        hasUpdate = (hasUpdate || assumeUpdate || fed->isUpdated(*this));
    }
    return hasUpdate;
}

void Input::setOption(int32_t option, int32_t value)
{
    if (option == HELICS_HANDLE_OPTION_MULTI_INPUT_HANDLING_METHOD) {
        inputVectorOp = static_cast<MultiInputHandlingMethod>(value);
    } else {
        Interface::setOption(option, value);
    }
}

/** get the current value of a flag for the handle*/
int32_t Input::getOption(int32_t option) const
{
    if (option == HELICS_HANDLE_OPTION_MULTI_INPUT_HANDLING_METHOD) {
        return static_cast<int32_t>(inputVectorOp);
    }
    return Interface::getOption(option);
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

Time Input::getLastUpdate() const
{
    return fed->getLastUpdateTime(*this);
}

void Input::registerNotificationCallback(std::function<void(Time)> callback)
{
    fed->setInputNotificationCallback(*this,
                                      [this, callback = std::move(callback)](const Input& /*inp*/,
                                                                             Time time) {
                                          if (isUpdated()) {
                                              callback(time);
                                          }
                                      });
}

void Input::registerCallback()
{
    fed->setInputNotificationCallback(*this, [this](Input& /*unused*/, Time time) {
        handleCallback(time);
    });
}
size_t Input::getByteCount()
{
    isUpdated();
    auto dv = fed->getBytes(*this);
    if (dv.empty()) {
        const auto& out = getValueRef<std::string>();
        return out.size();
    }
    return dv.size();
}

void Input::addPublication(std::string_view target)
{
    if (givenTarget.empty()) {
        givenTarget = target;
    }
    fed->addTarget(*this, target);
}

data_view Input::getBytes()
{
    hasUpdate = false;
    return fed->getBytes(*this);
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
        return std::get<std::string>(lastValue).size();
    }
    if (lastValue.index() == named_point_loc) {
        const auto& np = std::get<NamedPoint>(lastValue);

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
        case vector_loc:
            return std::get<std::vector<double>>(lastValue).size();
        case complex_vector_loc:
            return std::get<std::vector<std::complex<double>>>(lastValue).size();
        default:
            break;
    }
    const auto& out = getValueRef<std::vector<double>>();
    return out.size();
}

void Input::loadSourceInformation()
{
    if (targetType == DataType::HELICS_UNKNOWN) {
        targetType = getTypeFromString(getExtractionType());
    }
    multiUnits = false;
    const auto& iType = getInjectionType();
    const auto& iUnits = getInjectionUnits();
    injectionType = getTypeFromString(iType);
    if ((injectionType == DataType::HELICS_MULTI) || (!iUnits.empty() && iUnits.front() == '[')) {
        sourceTypes.clear();
        if (injectionType == DataType::HELICS_MULTI) {
            auto jvalue = fileops::loadJsonStr(iType);
            for (auto& res : jvalue) {
                sourceTypes.emplace_back(getTypeFromString(res.get<std::string>()), nullptr);
            }
        } else {
            auto iValue = fileops::loadJsonStr(iUnits);
            sourceTypes.resize(iValue.size(), {injectionType, nullptr});
        }
        if (!iUnits.empty()) {
            if (iUnits.front() == '[') {
                multiUnits = true;
                auto iValue = fileops::loadJsonStr(iUnits);
                int ii{0};
                for (auto& res : iValue) {
                    auto str = res.get<std::string>();
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

data_view Input::checkAndGetFedUpdate()
{
    return ((fed != nullptr) && (fed->isUpdated(*this) || allowDirectFederateUpdate())) ?
        (fed->getBytes(*this)) :
        data_view{};
}

void Input::forceCoreDataUpdate()
{
    if (fed == nullptr) {
        return;
    }
    auto dv = fed->getBytes(*this);
    if (!dv.empty()) {
        valueExtract(dv, injectionType, lastValue);
    } else if (getMultiInputMode() != MultiInputHandlingMethod::NO_OP) {
        fed->forceCoreUpdate(*this);
    }
}

bool checkForNeededCoreRetrieval(std::size_t currentIndex,
                                 DataType injectionType,
                                 DataType conversionType)
{
    static constexpr std::array<DataType, 7> locType{{DataType::HELICS_DOUBLE,
                                                      DataType::HELICS_INT,
                                                      DataType::HELICS_STRING,
                                                      DataType::HELICS_COMPLEX,
                                                      DataType::HELICS_VECTOR,
                                                      DataType::HELICS_COMPLEX_VECTOR,
                                                      DataType::HELICS_NAMED_POINT}};

    if (locType[currentIndex] == injectionType || locType[currentIndex] == conversionType) {
        return false;
    }

    return !(currentIndex != int_loc && conversionType == DataType::HELICS_DOUBLE);
}

char Input::getValueChar()
{
    auto dv = checkAndGetFedUpdate();
    if (!dv.empty()) {
        if (injectionType == DataType::HELICS_UNKNOWN) {
            loadSourceInformation();
        }

        if ((injectionType == DataType::HELICS_STRING) || (injectionType == DataType::HELICS_ANY) ||
            (injectionType == DataType::HELICS_CHAR) ||
            (injectionType == DataType::HELICS_CUSTOM)) {
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
            if (injectionType == helics::DataType::HELICS_DOUBLE) {
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
    const auto& V = getValueRef<std::vector<double>>();
    int length = 0;
    if (data != nullptr && maxsize > 0) {
        length = std::min(static_cast<int>(V.size()), maxsize);
        std::memmove(data, V.data(), length * sizeof(double));
    }

    hasUpdate = false;
    return length;
}

#if defined(__GNUC__)
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wstrict-aliasing"
// std::complex is explicitly allowed to alias like this in the standard
#endif

int Input::getComplexValue(double* data, int maxsize)
{
    const auto& CV = getValueRef<std::vector<std::complex<double>>>();
    int length = 0;
    if (data != nullptr && maxsize > 0) {
        length = std::min(static_cast<int>(CV.size()), maxsize);
        std::memmove(data, reinterpret_cast<const double*>(CV.data()), length * sizeof(double) * 2);
    }

    hasUpdate = false;
    return length;
}

#if defined(__GNUC__)
#    pragma GCC diagnostic pop
#endif

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
