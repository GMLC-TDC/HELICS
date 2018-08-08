/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once

#include "HelicsPrimaryTypes.hpp"
#include "ValueFederate.hpp"
#include "helicsTypes.hpp"
#include <algorithm>
#include <array>
namespace helics
{
/** base class for a subscription object*/
class InputBase
{
  protected:
    ValueFederate *fed = nullptr;  //!< reference to the value federate
    std::string name_;  //!< the name of the subscription
    std::string type_;  //!< the requested type of the subscription
    std::string units_;  //!< the defined units of the subscription
    input_id_t id;  //!< the id of the federate

  public:
    InputBase () = default;

    InputBase(ValueFederate *valueFed,
        const std::string &name,
        const std::string &type = "def",
        const std::string &units = std::string())
        : fed(valueFed), name_(name), type_(type), units_(units)
    {
        id = fed->registerInput(name_, type_, units_);
    }

    template <class FedPtr>
    InputBase (FedPtr & valueFed,
                      const std::string &name,
                      const std::string &type = "def",
                      const std::string &units = std::string ())
        : InputBase(std::addressof (*valueFed),name,type,units)
    {
        static_assert (std::is_base_of<ValueFederate, std::remove_reference_t<decltype (*valueFed)>>::value,
                       "first argument must be a pointer to a ValueFederate");
    }

   InputBase (ValueFederate *valueFed, int intIndex);
    virtual ~InputBase () = default;
    InputBase (InputBase &&base) = default;
    InputBase &operator= (InputBase &&base) = default;
    /** get the time of the last update
    @return the time of the last update
    */
    Time getLastUpdate () const { return fed->getLastUpdateTime (id); }
    /** check if the value has subscription has been updated*/
    virtual bool isUpdated () const { return fed->isUpdated (id); }
    input_id_t getID () const { return id; }

    /** register a callback for an update notification
    @details the callback is called in the just before the time request function returns
    @param[in] callback a function with signature void( Time time)
    time is the time the value was updated  This callback is a notification callback and doesn't return the value
    */
    void registerNotificationCallback (std::function<void(Time)> callback)
    {
        fed->registerInputNotificationCallback (id, [this,callback](input_id_t, Time time) { if (isUpdated()){callback (time);} });
    }
    /** get the Name for the subscription 
    @details the name is the local name if given, key is the full key name*/
    const std::string &getName () const { return name_; }
    /** get the type of the input*/
    std::string getType () const { return fed->getInputType (id); }
    /** get the units associated with a subscription*/
    const std::string &getUnits () const { return units_; }
    void addTarget (const std::string &newTarget) { fed->addTarget (id,newTarget);}
};

/** primary subscription object class
@details can convert between the helics primary base class types
*/
class Input : public InputBase
{
  private:
	//this needs to match the defV type
      mpark::variant<std::function<void(const double &, Time)>,
                   std::function<void(const int64_t &, Time)>,
                     std::function<void(const std::string &, Time)>,
                   std::function<void(const std::complex<double> &, Time)>,
                   std::function<void(const std::vector<double> &, Time)>, 
		std::function<void(const std::vector<std::complex<double>> &, Time)>,
		std::function<void(const named_point &, Time)>,
		std::function<void(const bool &, Time)> >
      value_callback;  //!< callback function for the federate

    mutable helics_type_t type = helics_type_t::helicsInvalid;  //!< the underlying type the publication is using
    bool changeDetectionEnabled = false;  //!< the change detection is enabled
    bool hasUpdate = false;  //!< the value has been updated
    defV lastValue;  //!< the last value updated
    double delta = -1.0;  //!< the minimum difference
  public:
    Input () = default;
    Input(ValueFederate *valueFed, const std::string &name, const std::string &units = std::string())
        : InputBase(valueFed, name, "def", units)
    {
    }

    template <class FedPtr>
    Input (FedPtr & valueFed, const std::string &key, const std::string &units = std::string ())
        : InputBase (valueFed, key, "def", units)
    {
    }

    Input(ValueFederate *valueFed,
        const std::string &name,
        helics_type_t defType,
        const std::string &units = std::string())
        : InputBase(valueFed, name, typeNameStringRef(defType), units)
    {
    }

    template <class FedPtr>
    Input (FedPtr &valueFed,
                  const std::string &name,
                  helics_type_t defType,
                  const std::string &units = std::string ())
        : InputBase (valueFed, name, typeNameStringRef (defType), units)
    {
    }

    /** generate a subscription object from a preexisting subscription
    @param valueFed a pointer to the appropriate value Federate
    @param subIndex the index of the subscription
    */
   Input (ValueFederate *valueFed, int inpIndex) : InputBase (valueFed, inpIndex) {}
    /** check if the value has been updated*/
    virtual bool isUpdated () const override;
    /** check if the value has been updated and load the value into buffer*/
    bool getAndCheckForUpdate();

    /** register a callback for the update
    @details the callback is called in the just before the time request function returns
    @param[in] callback a function with signature void(X val, Time time)
    val is the new value and time is the time the value was updated
    */
    template <class X>
    void registerCallback (std::function<void(const X &, Time)> callback)
    {
        static_assert (helicsType<X> () != helics_type_t::helicsInvalid,
                       "callback type must be a primary helics type one of \"double, int64_t, named_point, bool, "
                       "std::vector<double>, std::vector<std::complex<double>>, std::complex<double>\"");
        value_callback = callback;
        fed->registerInputNotificationCallback (id, [this](input_id_t, Time time) {
            handleCallback (time);
        });
    }

    /** set the default value to use before any update has been published
     */
    template <class X>
    void setDefault (X &&val)
    {
        lastValue = make_valid (std::forward<X> (val));
    }

    /** set the minimum delta for change detection
    @param detltaV a double with the change in a value in order to register a different value
    */
    void setMinimumChange (double deltaV)
    {
        // this first check enables change detection if it was disabled via negative delta
        if (delta < 0.0)
        {
            changeDetectionEnabled = true;
        }
        delta = deltaV;
        // the second checks if we should disable from negative delta
        if (delta < 0.0)
        {
            changeDetectionEnabled = false;
        }
    }
    /** enable change detection
    @param enabled (optional) set to false to disable change detection true(default) to enable it
    */
    void enableChangeDetection (bool enabled = true) { changeDetectionEnabled = enabled; }

  private:
    /** deal with the callback from the application API*/
    void handleCallback (Time time);
    template <class X>
    void getValue_impl (std::true_type /*V*/, X &out)
    {
        if (fed->isUpdated (id)||(hasUpdate && !changeDetectionEnabled))
        {
            auto dv = fed->getValueRaw (id);
            if (type == helics_type_t::helicsInvalid)
            {
                type = getTypeFromString (fed->getPublicationType (id));
            }
            if (type != helics_type_t::helicsInvalid)
            {
                valueExtract (dv, type, out);
                if (changeDetectionEnabled)
                {
                    if (changeDetected (lastValue, out, delta))
                    {
                        lastValue = make_valid (out);
                    }
                    else
                    {
                        valueExtract (lastValue, out);
                    }
                }
                else
                {
                    lastValue = make_valid (out);
                }
            }
            else
            {
                out = invalidValue<X> ();
            }
        }
        else
        {
            valueExtract (lastValue, out);
        }
        hasUpdate = false;
    }
    template <class X>
    void getValue_impl (std::false_type /*V*/, X &out)
    {
        std::conditional<std::is_integral<X>::value, int64_t, double> gval;
        getValue_impl (std::true_type (), gval);
        out = static_cast<X> (gval);
    }
    template <class X>
    X getValue_impl (std::true_type /*V*/)
    {
        X val;
        getValue_impl (std::true_type (), val);
        return val;
    }
    template <class X>
    X getValue_impl (std::false_type /*V*/)
    {
        std::conditional<std::is_integral<X>::value, int64_t, double> gval;
        getValue_impl (std::true_type (), gval);
        return static_cast<X> (gval);
    }

  public:
    int getValue (double *data, int maxsize);
    int getValue (char *str, int maxsize);
    /** get the latest value for the subscription
    @param[out] out the location to store the value
    */
    template <class X>
    void getValue (X &out)
    {
        static_assert (((helicsType<X> () != helics_type_t::helicsInvalid) || (isConvertableType<X> ())),
                       "requested types must be one of the primary helics types or convertible to one");
        getValue_impl<X> (std::conditional_t<(helicsType<X> () != helics_type_t::helicsInvalid), std::true_type,
                                             std::false_type> (),
                          out);
    }
    /** get the most recent value
    @return the value*/
    template <class X>
    X getValue ()
    {
        static_assert (((helicsType<X> () != helics_type_t::helicsInvalid) || (isConvertableType<X> ())),
                       "requested types must be one of the primary helics types or convertible to one");
        return getValue_impl<X> (std::conditional_t<(helicsType<X> () != helics_type_t::helicsInvalid),
                                                    std::true_type, std::false_type> ());
    }

    /** get the size of the raw data*/
    size_t getRawSize();
    /** get the size of the data if it were a string*/
    size_t getStringSize();
    /** get the number of elements in the data if it were a vector*/
    size_t getVectorSize();

	//TODO:: add a getValueByReference function that gets the data by reference but may force a copy and will only work on the primary types

};

/** class to handle a subscription
@tparam X the class of the value associated with a subscription*/
template <class X>
class InputT : public InputBase
{
  public:
  private:
    std::function<void(X, Time)> value_callback;  //!< callback function for the federate
    std::function<double(const X &v1, const X &v2)>
      changeDetectionCallback;  //!< callback function for change detection
    double delta = 0.0;  // 1< the minimum difference to trigger an update
    bool changeDetectionEnabled = false;  //!< flag indicating if change detection is enabled or not
  public:
    InputT () = default;
    /**constructor to build a subscription object
    @param[in] valueFed  the ValueFederate to use
    @param[in] name the name of the input
    @param[in] units the units associated with a Federate
    */
    InputT(ValueFederate *valueFed, const std::string &name, const std::string &units = std::string())
        : InputBase(valueFed, name, ValueConverter<X>::type(), units)
    {
    }
    /**constructor to build a subscription object
    @param[in] valueFed  the ValueFederate to use
    @param[in] name the name of the input
    @param[in] units the units associated with a Federate
    */
    template <class FedPtr>
    InputT (FedPtr &valueFed, const std::string &name, const std::string &units = std::string ())
        : InputBase (valueFed, name, ValueConverter<X>::type (), units)
    {
    }
   
    /** get the most recent value
    @return the value*/
    X getValue () const { return fed->getValue<X> (id); }
    /** store the value in the given variable
    @param[out] out the location to store the value
    */
    void getValue (X &out) const { fed->getValue (id, out); }

    /** register a callback for the update
    @details the callback is called in the just before the time request function returns
    @param[in] callback a function with signature void(X val, Time time)
    val is the new value and time is the time the value was updated
    */
    void registerCallback (std::function<void(X, Time)> callback)
    {
        value_callback = callback;
        fed->registerInputNotificationCallback (id, [=](input_id_t, Time time) {
            handleCallback (time);
        });
    }
    /** set a default value
    @param val the value to set as the default
    */
    void setDefault (const X &val) { fed->setDefaultValue (id, val); }
    /** set a minimum change value*/
    void setMinimumChange (double deltaV)
    {
        if (delta < 0.0)
        {
            changeDetectionEnabled = true;
        }
        delta = deltaV;
        if (delta < 0.0)
        {
            changeDetectionEnabled = false;
        }
    }
    void enableChangeDetection (bool enabled = true) { changeDetectionEnabled = enabled; }

    /** store the value in the given variable
    @param[out] out the location to store the value
    */
    const X &operator[] (int index) const { return fed->getValue<X> (id, index); }
  private:
    void handleCallback (Time time)
    {
        X out;
        fed->getValue (id, out);
        value_callback (out, time);
    }
};

}  // namespace helics
