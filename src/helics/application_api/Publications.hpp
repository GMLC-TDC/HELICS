/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "../core/core-exceptions.hpp"
#include "Federate.hpp"
#include "HelicsPrimaryTypes.hpp"

#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace units {
class precise_unit;
}  // namespace units

namespace helics {
class ValueFederate;
/** define a publication object in the C++98 interface*/
class HELICS_CXX_EXPORT Publication: public Interface {
  protected:
    ValueFederate* fed{nullptr};  //!< the federate construct to interact with
  private:
    int referenceIndex{-1};  //!< an index used for callback lookup
    void* dataReference{nullptr};  //!< pointer to a piece of containing data
    double delta{-1.0};  //!< the minimum change to publish
  protected:
    DataType pubType{DataType::HELICS_ANY};  //!< the type of publication
    bool changeDetectionEnabled{false};  //!< the change detection is enabled
    bool disableAssign{false};  //!< disable assignment for the object
  private:
    size_t customTypeHash{
        0};  //!< a hash code for the custom type = 0; //!< store a hash code for a custom type
    mutable defV prevValue;  //!< the previous value of the publication
    std::string pubUnits;  //!< the defined units of the publication
    std::shared_ptr<units::precise_unit>
        pubUnitType;  //!< a unit representation of the publication unit Type;
  public:
    Publication() = default;
    /** constructor for a publication used by the valueFederateManager
    @param valueFed a pointer the link valueFederate
    @param id the InterfaceHandle from the core
    @param key the identifier for the publication
    @param type the type of the publication
    @param units an optional string defining the units*/
    Publication(ValueFederate* valueFed,
                InterfaceHandle id,
                std::string_view key,
                std::string_view type,
                std::string_view units);

    /** constructor for a publication
  @param valueFed a pointer the link valueFederate
  @param key the identifier for the publication
  @param type the type of the publication
  @param units an optional string defining the units*/
    Publication(ValueFederate* valueFed,
                std::string_view key,
                std::string_view type,
                std::string_view units = std::string_view{});

    /** base constructor for a publication
    @param valueFed a pointer of some kind to a value federate (any dereferenceable type with * and
    -> operator that results in a valueFederate object
    @param key the identifier for the publication
    @param type the type of the publication
    @param units an optional string defining the units*/
    template<class FedPtr>
    Publication(FedPtr valueFed,
                std::string_view key,
                std::string_view type = std::string_view(),
                std::string_view units = std::string_view()):
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
    Publication(InterfaceVisibility locality,
                ValueFederate* valueFed,
                std::string_view key,
                std::string_view type,
                std::string_view units = std::string_view());
    /** base constructor for a publication
    @param locality either GLOBAL or LOCAL, LOCAL prepends the federate name to create a global
    identifier
    @param valueFed a pointer of some kind to a value federate (any dereferenceable type with * and
    -> operator that results in a valueFederate object
    @param key the identifier for the publication
    @param type the type of the publication
    @param units an optional string defining the units*/
    template<class FedPtr>
    Publication(InterfaceVisibility locality,
                FedPtr& valueFed,
                std::string_view key,
                std::string_view type,
                std::string_view units = std::string_view()):
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
                std::string_view key,
                DataType type,
                std::string_view units = std::string_view()):
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
                std::string_view key,
                DataType type,
                std::string_view units = std::string_view()):
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
    Publication(InterfaceVisibility locality,
                ValueFederate* valueFed,
                std::string_view key,
                DataType type,
                std::string_view units = std::string_view()):
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
    Publication(InterfaceVisibility locality,
                FedPtr& valueFed,
                std::string_view key,
                DataType type,
                std::string_view units = std::string_view()):
        Publication(locality, valueFed, key, typeNameStringRef(type), units)
    {
    }

    /** get the type for the publication*/
    const std::string& getType() const { return getExtractionType(); }
    /** get the units of the publication*/
    const std::string& getUnits() const { return pubUnits; }
    /** add an input object to target*/
    void addTarget(std::string_view target) { addDestinationTarget(target); }
    /** add an input object to send the information*/
    void addInputTarget(std::string_view target) { addDestinationTarget(target); }
    /** close a input during an active simulation
    @details it is not necessary to call this function unless you are continuing the simulation
    after the close*/

    /** send a value for publication
    @param val the value to publish*/
    void publish(double val);
    void publish(const std::vector<std::string>& val);
    void publish(const std::vector<double>& val);
    void publish(const double* vals, int size);
    void publish(const std::vector<std::complex<double>>& val);
    /** interpret as a vector of alternating real and imag values*/
    void publishComplex(const double* vals, int size);
    void publish(std::complex<double> val);

    void publish(bool val);
    void publish(Time val);
    void publish(char val);
    void publish(const NamedPoint& np);
    void publish(std::string_view field, double val);
    /** secondary publish function to allow unit conversion before publication
    @param val the value to publish
    @param units  the units association with the publication
    */

    void publish(double val, const std::string& units);
    void publish(double val, const units::precise_unit& units);

    /** publish stringLike values */
    template<class X>
    std::enable_if_t<(std::is_constructible_v<std::string_view, X>), void> publish(const X& val)
    {
        publishString(val);
    }

    /** publish stringLike values */
    template<class X>
    std::enable_if_t<(std::is_same_v<defV, remove_cv_ref<X>>), void> publish(const X& val)
    {
        publishDefV(val);
    }

    /** publish integral values */
    template<class X>
    std::enable_if_t<(std::is_integral_v<X> && !std::is_same_v<remove_cv_ref<X>, char>), void>
        publish(X val)
    {
        publishInt(static_cast<int64_t>(val));
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

    virtual const std::string& getDisplayName() const override { return getName(); }

  private:
    /** implementation of the integer publications
    @details this is the same as the other publish function but is used in the template due to
    template overload resolution rules I wanted to be able to call this inside a template which took
    all Int types and without this it would be recursive
    */
    void publishInt(int64_t val);
    void publishString(std::string_view val);
    void publishDefV(const defV& val);
    friend class ValueFederateManager;
};

}  // namespace helics
