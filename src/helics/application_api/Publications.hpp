/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "../core/core-exceptions.hpp"
#include "HelicsPrimaryTypes.hpp"
#include "ValueFederate.hpp"

#include <memory>
#include <string>
#include <vector>

namespace units {
class precise_unit;
}  // namespace units

namespace helics {
/** define a publication object in the C++98 interface*/
class HELICS_CXX_EXPORT Publication {
  protected:
    ValueFederate* fed{nullptr};  //!< the federate construct to interact with
    interface_handle handle;  //!< the internal id of the publication
  private:
    int referenceIndex{-1};  //!< an index used for callback lookup
    void* dataReference{nullptr};  //!< pointer to a piece of containing data
    double delta{-1.0};  //!< the minimum change to publish
  protected:
    data_type pubType{data_type::helics_any};  //!< the type of publication
    bool changeDetectionEnabled{false};  //!< the change detection is enabled
    bool disableAssign{false};  //!< disable assignment for the object
  private:
    size_t customTypeHash{
        0};  //!< a hash code for the custom type = 0; //!< store a hash code for a custom type
    mutable defV prevValue;  //!< the previous value of the publication
    std::string pubKey;  //!< the name of the publication
    std::string pubUnits;  //!< the defined units of the publication
    std::shared_ptr<units::precise_unit>
        pubUnitType;  //!< a unit representation of the publication unit Type;
  public:
    Publication() = default;
    /** constructor for a publication used by the valueFederateManager
    @param valueFed a pointer the link valueFederate
    @param id the interface_handle from the core
    @param key the identifier for the publication
    @param type the type of the publication
    @param units an optional string defining the units*/
    Publication(ValueFederate* valueFed,
                interface_handle id,
                const std::string& key,
                const std::string& type,
                const std::string& units);

    /** constructor for a publication
  @param valueFed a pointer the link valueFederate
  @param key the identifier for the publication
  @param type the type of the publication
  @param units an optional string defining the units*/
    Publication(ValueFederate* valueFed,
                const std::string& key,
                const std::string& type,
                const std::string& units = std::string());

    /** base constructor for a publication
    @param valueFed a pointer of some kind to a value federate (any dereferenceable type with * and
    -> operator that results in a valueFederate object
    @param key the identifier for the publication
    @param type the type of the publication
    @param units an optional string defining the units*/
    template<class FedPtr>
    Publication(FedPtr valueFed,
                const std::string& key,
                const std::string& type = std::string(),
                const std::string& units = std::string()):
        Publication(std::addressof(*valueFed), key, type, units)
    {
        static_assert(
            std::is_base_of<ValueFederate, std::remove_reference_t<decltype(*valueFed)>>::value,
            "first argument must be a pointer to a ValueFederate");
    }
    /** base constructor for a publication
    @param locality either GLOBAL or LOCAL, LOCAL prepends the federate name to create a global
    identifier
    @param valueFed a pointer to a value federate
    @param key the identifier for the publication
    @param type the type of the publication
    @param units an optional string defining the units*/
    Publication(interface_visibility locality,
                ValueFederate* valueFed,
                const std::string& key,
                const std::string& type,
                const std::string& units = std::string());
    /** base constructor for a publication
    @param locality either GLOBAL or LOCAL, LOCAL prepends the federate name to create a global
    identifier
    @param valueFed a pointer of some kind to a value federate (any dereferenceable type with * and
    -> operator that results in a valueFederate object
    @param key the identifier for the publication
    @param type the type of the publication
    @param units an optional string defining the units*/
    template<class FedPtr>
    Publication(interface_visibility locality,
                FedPtr& valueFed,
                const std::string& key,
                const std::string& type,
                const std::string& units = std::string()):
        Publication(locality, std::addressof(*valueFed), key, type, units)
    {
        static_assert(
            std::is_base_of<ValueFederate, std::remove_reference_t<decltype(*valueFed)>>::value,
            "first argument must be a pointer to a ValueFederate");
    }

    /**constructor to build a publication object
  @param valueFed  the ValueFederate to use
  @param key the identifier for the publication
  @param type the defined type of the publication
  @param units the units associated with a Federate
  */
    Publication(ValueFederate* valueFed,
                const std::string& key,
                data_type type,
                const std::string& units = std::string()):
        Publication(valueFed, key, typeNameStringRef(type), units)
    {
    }
    /**constructor to build a publication object
    @param valueFed  the ValueFederate to use
    @param key the identifier for the publication
    @param type the defined type of the publication
    @param units the units associated with a Federate
    */
    template<class FedPtr>
    Publication(FedPtr& valueFed,
                const std::string& key,
                data_type type,
                const std::string& units = std::string()):
        Publication(valueFed, key, typeNameStringRef(type), units)
    {
    }
    /**constructor to build a publication object
    @param locality  set to global for a global publication or local for a local one
    @param valueFed  the ValueFederate to use
    @param key the identifier for the publication
    @param type the defined type of the publication
    @param units the units associated with a Federate
    */
    Publication(interface_visibility locality,
                ValueFederate* valueFed,
                const std::string& key,
                data_type type,
                const std::string& units = std::string()):
        Publication(locality, valueFed, key, typeNameStringRef(type), units)
    {
    }

    /**constructor to build a publication object
    @param locality  set to global for a global publication or local for a local one
    @param valueFed  the ValueFederate to use
    @param key the identifier for the publication
    @param type the defined type of the publication
    @param units the units associated with a Federate
    */
    template<class FedPtr>
    Publication(interface_visibility locality,
                FedPtr& valueFed,
                const std::string& key,
                data_type type,
                const std::string& units = std::string()):
        Publication(locality, valueFed, key, typeNameStringRef(type), units)
    {
    }

    /** get the publication id that can be used to make the function calls from a Value Federate
     */
    interface_handle getHandle() const { return handle; }
    /** implicit conversion operator for extracting the handle*/
    operator interface_handle() const { return handle; }

    /** check if the Publication links to a valid operation*/
    bool isValid() const { return handle.isValid(); }
    bool operator==(const Publication& pub) const { return (handle == pub.handle); }
    bool operator!=(const Publication& pub) const { return (handle != pub.handle); }
    bool operator<(const Publication& pub) const { return (handle < pub.handle); }

    /** get the key for the publication*/
    const std::string& getKey() const { return fed->getInterfaceName(*this); }
    /** get the key for the publication*/
    const std::string& getName() const { return pubKey; }
    /** get the type for the publication*/
    const std::string& getType() const { return fed->getExtractionType(*this); }
    /** get the units of the publication*/
    const std::string& getUnits() const { return pubUnits; }
    /** get the interface information field of the publication*/
    const std::string& getInfo() const { return fed->getInfo(handle); }
    /** set the interface information field of the publication*/
    void setInfo(const std::string& info) { fed->setInfo(handle, info); }
    /** set an option on the publication
    @param option the option to set
    @param value the value to set the option*/
    void setOption(int32_t option, int32_t value = 1)
    {
        fed->setInterfaceOption(handle, option, value);
    }

    /** get the current value of a flag for the handle*/
    int32_t getOption(int32_t option) const { return fed->getInterfaceOption(handle, option); }

    /** add a target to the publication*/
    void addTarget(const std::string& target) { fed->addTarget(*this, target); }
    /** remove a named input from sending data*/
    void removeTarget(const std::string& targetToRemove)
    {
        fed->removeTarget(*this, targetToRemove);
    }
    /** close a input during an active simulation
    @details it is not necessary to call this function unless you are continuing the simulation
    after the close*/
    void close() { fed->closeInterface(handle); }
    /** send a value for publication
    @param val the value to publish*/
    void publish(double val);
    void publish(const char* val);
    void publish(const std::string& val);
    void publish(const std::vector<double>& val);
    void publish(const std::vector<std::complex<double>>& val);
    void publish(const double* vals, int size);
    void publish(std::complex<double> val);
    void publish(const defV& val);
    void publish(bool val);
    void publish(Time val);
    void publish(char val);
    void publish(const NamedPoint& np);
    void publish(const std::string& name, double val);
    void publish(const char* name, double val);
    /** secondary publish function to allow unit conversion before publication
    @param val the value to publish
    @param units  the units association with the publication
    */

    void publish(double val, const std::string& units);
    void publish(double val, const units::precise_unit& units);

    /** publish integral values */
    template<class X>
    std::enable_if_t<(std::is_integral<X>::value && !std::is_same<remove_cv_ref<X>, char>::value),
                     void>
        publish(X val)
    {
        publishInt(static_cast<int64_t>(val));
    }

    /** publish anything not previously covered*/
    template<class X>
    std::enable_if_t<((typeCategory<X>::value == nonConvertibleType) &&
                      (!std::is_convertible<X, std::string>::value) &&
                      (!std::is_same<X, defV>::value) && (!std::is_convertible<X, Time>::value)),
                     void>
        publish(const X& val)
    {
        if ((pubType == data_type::helics_custom) || (pubType == data_type::helics_any)) {
            fed->publishRaw(*this, ValueConverter<X>::convert(val));
        }
    }
    /** set the level by which a value must have changed to actually publish the value
     */
    void setMinimumChange(double deltaV) noexcept
    {
        if (delta < 0.0) {
            changeDetectionEnabled = true;
        }
        delta = deltaV;
        if (delta < 0.0) {
            changeDetectionEnabled = false;
        }
    }
    /** if set to false, the change detection mechanisms are not enabled
    if set to true the values will be published if there is sufficient change as specified in
    the call to setMinimumChange
    */
    void enableChangeDetection(bool enabled = true) noexcept { changeDetectionEnabled = enabled; }

  private:
    /** implementation of the integer publications
    @details this is the same as the other publish function but is used in the template due to
    template overload resolution rules I wanted to be able to call this inside a template which took
    all Int types and without this it would be recursive
    */
    void publishInt(int64_t val);
    friend class ValueFederateManager;
};

/** create a pointer to a publication
@tparam X is the type of the publication
@param valueFed pointer to a valid federate
@param key the identifier for the publication
@param units optional units for the publication
*/
template<class X>
typename std::enable_if_t<helicsType<X>() != data_type::helics_custom, std::unique_ptr<Publication>>
    make_publication(ValueFederate* valueFed,
                     const std::string& key,
                     const std::string& units = std::string())
{
    return std::make_unique<Publication>(valueFed, helicsType<X>(), key, units);
}

/** create a pointer to a publication
@tparam X is the type of the publication
@tparam FedPtr a pointer a value Federate
@param valueFed pointer to a valid federate
@param key the identifier for the publication
@param units optional units for the publication
*/
template<class X, class FedPtr>
typename std::enable_if_t<helicsType<X>() != data_type::helics_custom, std::unique_ptr<Publication>>
    make_publication(FedPtr& valueFed,
                     const std::string& key,
                     const std::string& units = std::string())
{
    return std::make_unique<Publication>(valueFed, helicsType<X>(), key, units);
}

/** create a pointer to a publication
@tparam X is the type of the publication
@param locality either LOCAL or GLOBAL defining whether the federate name is prepended or not
@param valueFed pointer to a valid federate
@param key the identifier for the publication
@param units optional units for the publication
*/
template<class X>
typename std::enable_if_t<helicsType<X>() != data_type::helics_custom, std::unique_ptr<Publication>>
    make_publication(interface_visibility locality,
                     ValueFederate* valueFed,
                     const std::string& key,
                     const std::string& units = std::string())
{
    return std::make_unique<Publication>(locality, valueFed, key, helicsType<X>(), units);
}

/** create a pointer to a publication
@tparam X is the type of the publication
@tparam FedPtr a pointer a value Federate
@param locality either LOCAL or GLOBAL defining whether the federate name is prepended or not
@param valueFed pointer to a valid federate
@param key the identifier for the publication
@param units optional units for the publication
*/
template<class X, class FedPtr>
typename std::enable_if_t<helicsType<X>() != data_type::helics_custom, std::unique_ptr<Publication>>
    make_publication(interface_visibility locality,
                     FedPtr& valueFed,
                     const std::string& key,
                     const std::string& units = std::string())
{
    return std::make_unique<Publication>(locality, valueFed, key, helicsType<X>(), units);
}

/** class to handle a publication of an arbitrary type*/
template<class X>
class PublicationT: public Publication {
  public:
    PublicationT() = default;
    /**constructor to build a publication object
    @param valueFed  the ValueFederate to use
    @param key the identifier for the publication
    @param units the units associated with a Federate
    */
    PublicationT(ValueFederate* valueFed,
                 const std::string& key,
                 const std::string& units = std::string()):
        Publication(valueFed, key, typeNameString<X>(), units)
    {
    }
    /**constructor to build a publication object
    @param valueFed  the ValueFederate to use
    @param key the identifier for the publication
    @param units the units associated with a Federate
    */
    template<class FedPtr>
    PublicationT(FedPtr& valueFed,
                 const std::string& key,
                 const std::string& units = std::string()):
        Publication(valueFed, key, typeNameString<X>(), units)
    {
    }
    /**constructor to build a publication object
    @param locality the visibility of the publication either global or local
    @param valueFed  the ValueFederate to use
    @param key the identifier for the publication
    @param units the units associated with a Federate
    */
    PublicationT(interface_visibility locality,
                 ValueFederate* valueFed,
                 const std::string& key,
                 const std::string& units = std::string()):
        Publication(locality, valueFed, key, typeNameString<X>(), units)
    {
    }
    /**constructor to build a publication object
     @param locality  define the visibility of the publication (local or global)
    @param valueFed  the ValueFederate to use
    @param key the identifier for the publication
    @param units the units associated with a Federate
    */
    template<class FedPtr>
    PublicationT(interface_visibility locality,
                 FedPtr& valueFed,
                 const std::string& key,
                 const std::string& units = std::string()):
        Publication(locality, valueFed, key, typeNameString<X>(), units)
    {
    }
    /** send a value for publication
    @param val the value to publish*/
    void publish(const X& val) { Publication::publish(val); }
    /** secondary publish function to allow unit conversion before publication
    @param val the value to publish
    @param units  the units association with the publication
    */
    void publish(const X& val, const std::string& units)
    {
        // TODO(PT): figure out units
        (void)(units);
        Publication::publish(val);
    }
};

/** class to handle a publication on change for an arbitrary type
but the value is only published in the change is greater than a certain level*/
template<class X>
class PublicationOnChange: public PublicationT<X> {
  private:
    X publishDelta;  //!< the delta on which to publish a value
    mutable X prev;  //!< the previous value
  public:
    PublicationOnChange() = default;
    /**constructor to build a publishOnChange object
    @param valueFed  the ValueFederate to use
    @param key the identifier for the publication
    @param minChange  the minimum change required to actually publish the value
    @param units the units associated with a Federate
    */
    PublicationOnChange(ValueFederate* valueFed,
                        const std::string& key,
                        const X& minChange,
                        const std::string& units = std::string()):
        PublicationT<X>(valueFed, key, units),
        publishDelta(minChange), prev(X())
    {
    }
    /**constructor to build a publishOnChange object
    @param valueFed  the ValueFederate to use
    @param key the identifier for the publication
    @param minChange  the minimum change required to actually publish the value
    @param units the units associated with a Federate
    */
    template<class FedPtr>
    PublicationOnChange(FedPtr& valueFed,
                        const std::string& key,
                        const X& minChange,
                        const std::string& units = std::string()):
        PublicationT<X>(valueFed, key, units),
        publishDelta(minChange), prev(X())
    {
    }
    /** send a value for publication
    @details the value is only published if it exceeds the specified level
    @param val the value to publish*/
    virtual void publish(const X& val) const override
    {
        if (std::abs(val - prev) >= publishDelta) {
            prev = val;
            PublicationT<X>::publish(val);
        }
    }
};

/** publish directly from the publication key name
@details this is a convenience function to publish directly from the publication key
this function should not be used as the primary means of publications as it does involve an
additional map find operation vs the member publish calls
@param fed a reference to a valueFederate
@param pubKey  the name of the publication
@param pargs any combination of arguments that go into the other publish commands
*/
template<class... Us>
void publish(ValueFederate& fed, const std::string& pubKey, Us... pargs)
{
    fed.getPublication(pubKey).publish(pargs...);
}
}  // namespace helics
