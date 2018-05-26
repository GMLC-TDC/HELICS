/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#pragma once
#include <cstdint>

namespace helics
{
using identififier_base_type = int32_t;

/** class defining a federate_id_t
@details  the intent of this class is to limit the operations available on a federate identifier
to those that are a actually required and make sense, and make it as low impact as possible.
it also acts to limit any mistakes of a federate_id_t
*/
class federate_id_t
{

public:
    using base_type = identififier_base_type;
    /** default constructor*/
    constexpr federate_id_t()=default;
    
    constexpr explicit federate_id_t(base_type val) noexcept : _id(val) {};

    constexpr operator base_type() const { return _id; }
    /** equality operator*/
    bool operator== (federate_id_t id) const noexcept { return (_id == id._id); };
    /** inequality operator*/
    bool operator!= (federate_id_t id) const noexcept { return (_id != id._id); };
    /** less than operator for sorting*/
    bool operator< (federate_id_t id) const noexcept { return (_id < id._id); };
    bool isValid() const {
        return (_id != -2'000'000'000);
    }
private:
    base_type _id = -2'000'000'000;  //!< the underlying index value

};


/** a shift in the global federate id numbers to allow discrimination between local ids and global ones
this value allows 131072 federates to be available in each core
1,878,917,120 allowable federates in the system and
268,435,455 brokers allowed  if we need more than that this program has been phenomenally successful beyond
all wildest imaginations and we can probably afford to change these to 64 bit numbers to accommodate
*/
constexpr identififier_base_type global_federate_id_shift = 0x0002'0000;
/** a shift in the global id index to discriminate between global ids of brokers vs federates*/
constexpr identififier_base_type global_broker_id_shift = 0x7000'0000;

/** constant to use for indicating that a command is for the core itself from the Core Public API*/
constexpr federate_id_t local_core_id(-259);

class global_federate_id_t
{
public:
    using base_type = identififier_base_type;
    /** default constructor*/
    constexpr global_federate_id_t() = default;

    constexpr explicit global_federate_id_t(base_type val) noexcept : _id(val) {};
    /**implicit conversion to the basetype*/
    constexpr operator base_type() const { return _id; }
    /** equality operator*/
    bool operator== (global_federate_id_t id) const noexcept { return (_id == id._id); };
    /** inequality operator*/
    bool operator!= (global_federate_id_t id) const noexcept { return (_id != id._id); };
    /** less than operator for sorting*/
    bool operator< (global_federate_id_t id) const noexcept { return (_id < id._id); };
   
    bool isFederate() const {
        return ((_id >= global_federate_id_shift) && (_id < global_broker_id_shift));
    }
    bool isBroker() const {
        return (_id >= global_broker_id_shift);
    }
    bool isValid() const {
        return (isFederate());
    }
    constexpr base_type localIndex() const { return _id - global_federate_id_shift; }
private:
    base_type _id = -2'000'000'000;  //!< the underlying index value
    friend class global_broker_id_t;
};

class global_broker_id_t
{
public:
    using base_type = identififier_base_type;
    /** default constructor*/
    constexpr global_broker_id_t() = default;

    constexpr explicit global_broker_id_t(base_type val) noexcept : _id(val) {};

    constexpr operator base_type() const { return _id; }
    constexpr operator global_federate_id_t() const { return global_federate_id_t(_id); }
    /** equality operator*/
    bool operator== (global_broker_id_t id) const noexcept { return (_id == id._id); };
    /** inequality operator*/
    bool operator!= (global_broker_id_t id) const noexcept { return (_id != id._id); };
    /** less than operator for sorting*/
    bool operator< (global_broker_id_t id) const noexcept { return (_id < id._id); };
    
    /** equality operator*/
    bool operator== (global_federate_id_t id) const noexcept { return (_id == id._id); };
    /** inequality operator*/
    bool operator!= (global_federate_id_t id) const noexcept { return (_id != id._id); };
    /** less than operator for sorting*/
    bool operator< (global_federate_id_t id) const noexcept { return (_id < id._id); };

    bool isFederate() const {
        return ((_id >= global_federate_id_shift) && (_id < global_broker_id_shift));
    }
    bool isBroker() const {
        return (_id >= global_broker_id_shift);
    }
    bool isValid() const {
        return (isBroker());
    }
    base_type localIndex() const { return _id - global_broker_id_shift; }
private:
    base_type _id = -2'000'000'000;  //!< the underlying index value
    friend class global_federate_id_t;

};

/** constant to use for indicating that a command is for the core itself from the Core Public API*/
constexpr global_broker_id_t parent_broker_id(0);



} // namespace helics