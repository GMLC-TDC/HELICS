/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#pragma once
#include "../common/MapTraits.hpp"
#include <cstdint>
#include <functional>
#include <iosfwd>

namespace helics
{
using identififier_base_type = int32_t;

/** class defining a local_federate_id
@details  the intent of this class is to limit the operations available on a federate identifier
to those that are a actually required and make sense, and make it as low impact as possible.
it also acts to limit any mistakes of a local_federate_id
*/
class local_federate_id
{
  public:
    using base_type = identififier_base_type;
    /** default constructor*/
    constexpr local_federate_id () = default;

    constexpr explicit local_federate_id (base_type val) noexcept : fid (val){};

    constexpr base_type baseValue () const { return fid; }
    /** equality operator*/
    bool operator== (local_federate_id id) const noexcept { return (fid == id.fid); };
    /** inequality operator*/
    bool operator!= (local_federate_id id) const noexcept { return (fid != id.fid); };
    /** less than operator for sorting*/
    bool operator< (local_federate_id id) const noexcept { return (fid < id.fid); };
    bool isValid () const { return (fid != -2'000'000'000); }

  private:
    base_type fid = -2'000'000'000;  //!< the underlying index value
};

/** stream operator for a federate_id
 */
std::ostream &operator<< (std::ostream &os, local_federate_id fid);

/** class defining a local_federate_id
@details  the intent of this class is to limit the operations available on a federate identifier
to those that are a actually required and make sense, and make it as low impact as possible.
it also acts to limit any mistakes of a local_federate_id
*/
class interface_handle
{
  public:
    using base_type = identififier_base_type;
    /** default constructor*/
    constexpr interface_handle () = default;

    constexpr explicit interface_handle (base_type val) noexcept : hid (val){};

    constexpr base_type baseValue () const { return hid; }
    /** equality operator*/
    bool operator== (interface_handle id) const noexcept { return (hid == id.hid); };
    /** inequality operator*/
    bool operator!= (interface_handle id) const noexcept { return (hid != id.hid); };
    /** comparison operator for sorting*/
    bool operator< (interface_handle id) const noexcept { return (hid < id.hid); };
    bool operator> (interface_handle id) const noexcept { return (hid > id.hid); };
    bool isValid () const { return (hid != -1'700'000'000); }

  private:
    base_type hid = -1'700'000'000;  //!< the underlying index value
};

/** stream operator for a federate_id
 */
std::ostream &operator<< (std::ostream &os, interface_handle handle);

constexpr interface_handle direct_send_handle =
  interface_handle (-1'745'234);  //!< this special handle can be used to directly send a message in a core

/** a shift in the global federate id numbers to allow discrimination between local ids and global ones
this value allows 131072 federates to be available in each core
1,878,917,120 allowable federates in the system and
268,435,455 brokers allowed  if we need more than that this, program has been phenomenally successful beyond
all wildest imaginations and we can probably afford to change these to 64 bit numbers to accommodate
*/
constexpr identififier_base_type global_federate_id_shift = 0x0002'0000;
/** a shift in the global id index to discriminate between global ids of brokers vs federates*/
constexpr identififier_base_type global_broker_id_shift = 0x7000'0000;

/** constant to use for indicating that a command is for the core itself from the Core Public API*/
constexpr local_federate_id local_core_id (-259);

class global_broker_id
{
  public:
    using base_type = identififier_base_type;
    /** default constructor*/
    constexpr global_broker_id () = default;

    constexpr explicit global_broker_id (base_type val) noexcept : gid (val){};

    constexpr base_type baseValue () const { return gid; }
    /** equality operator*/
    bool operator== (global_broker_id id) const noexcept { return (gid == id.gid); };
    /** inequality operator*/
    bool operator!= (global_broker_id id) const noexcept { return (gid != id.gid); };
    /** less than operator for sorting*/
    bool operator< (global_broker_id id) const noexcept { return (gid < id.gid); };

    bool isFederate () const { return ((gid >= global_federate_id_shift) && (gid < global_broker_id_shift)); }
    bool isBroker () const { return (gid >= global_broker_id_shift); }
    bool isValid () const { return (gid != -2'010'000'000); }
    base_type localIndex () const { return gid - global_broker_id_shift; }

  private:
    base_type gid = -2'010'000'000;  //!< the underlying index value
    friend class global_federate_id;  // for allowing comparison operators to work well
};

/** constant to use for indicating that a command is for the core itself from the Core Public API*/
constexpr global_broker_id parent_broker_id (0);

/** stream operator for a federate_id
 */
std::ostream &operator<< (std::ostream &os, global_broker_id id);

class global_federate_id
{
  public:
    using base_type = identififier_base_type;
    /** default constructor*/
    constexpr global_federate_id () = default;

    constexpr explicit global_federate_id (base_type val) noexcept : gid (val){};
    /** implicit conversion from global_id*/
    constexpr global_federate_id (global_broker_id id) noexcept : gid (id.gid){};

    constexpr operator global_broker_id () const noexcept { return global_broker_id (gid); };
    /** conversion to the base_type*/
    constexpr base_type baseValue () const { return gid; };
    /** equality operator*/
    bool operator== (global_federate_id id) const noexcept { return (gid == id.gid); };
    /** inequality operator*/
    bool operator!= (global_federate_id id) const noexcept { return (gid != id.gid); };
    /** less than operator for sorting*/
    bool operator< (global_federate_id id) const noexcept { return (gid < id.gid); };
    /** greater than operator for sorting*/
    bool operator> (global_federate_id id) const noexcept { return (gid > id.gid); };
    /** equality operator*/
    bool operator== (global_broker_id id) const noexcept { return (gid == id.gid); };
    /** inequality operator*/
    bool operator!= (global_broker_id id) const noexcept { return (gid != id.gid); };
    /** less than operator for sorting*/
    bool operator< (global_broker_id id) const noexcept { return (gid < id.gid); };
    /** greater than operator for sorting*/
    bool operator> (global_broker_id id) const noexcept { return (gid > id.gid); };
    bool isFederate () const { return ((gid >= global_federate_id_shift) && (gid < global_broker_id_shift)); }
    bool isBroker () const { return (gid >= global_broker_id_shift); }
    bool isValid () const { return (gid != -2'010'000'000); }
    constexpr base_type localIndex () const { return gid - global_federate_id_shift; }

  private:
    base_type gid = -2'010'000'000;  //!< the underlying index value
};

/** stream operator for a federate_id
 */
std::ostream &operator<< (std::ostream &os, global_federate_id id);

/** class merging a global id and handle together */
class global_handle
{
  public:
    global_federate_id fed_id = global_federate_id{};
    interface_handle handle = interface_handle{};
    constexpr global_handle () = default;
    constexpr global_handle (global_federate_id fed, interface_handle hand) : fed_id (fed), handle (hand){};
    explicit operator uint64_t () const
    {
        auto key = static_cast<uint64_t> (fed_id.baseValue ()) << 32u;
        key += static_cast<uint64_t> (handle.baseValue ()) & (0x0000'0000'FFFF'FFFF);
        return key;
    }
    bool operator== (global_handle id) const noexcept { return ((fed_id == id.fed_id) && (handle == id.handle)); };
    /** inequality operator*/
    bool operator!= (global_handle id) const noexcept { return ((fed_id != id.fed_id) || (handle != id.handle)); };
    /** less than operator for sorting*/
    bool operator< (global_handle id) const noexcept
    {
        return (fed_id < id.fed_id) ? true : ((fed_id != id.fed_id) ? false : (handle < id.handle));
    };
    /** check if the handle points to a validly specified interface*/
    bool isValid () const { return fed_id.isValid () && handle.isValid (); };
};

/** stream operator for a federate_id
 */
std::ostream &operator<< (std::ostream &os, global_handle id);

class route_id
{
  public:
    using base_type = identififier_base_type;
    /** default constructor*/
    constexpr route_id () = default;

    constexpr explicit route_id (base_type val) noexcept : rid (val){};

    constexpr base_type baseValue () const { return rid; }
    /** equality operator*/
    bool operator== (route_id id) const noexcept { return (rid == id.rid); };
    /** inequality operator*/
    bool operator!= (route_id id) const noexcept { return (rid != id.rid); };
    /** less than operator for sorting*/
    bool operator< (route_id id) const noexcept { return (rid < id.rid); };
    bool isValid () const { return (rid != -1'295'148'000); }

  private:
    base_type rid = -1'295'148'000;  //!< the underlying index value
};

constexpr route_id parent_route_id (0);

/** stream operator for a route_id
 */
std::ostream &operator<< (std::ostream &os, route_id id);

}  // namespace helics

namespace std
{
template <>
struct hash<helics::local_federate_id>
{
    using argument_type = helics::local_federate_id;
    using result_type = hash<helics::local_federate_id::base_type>::result_type;
    result_type operator() (argument_type const &key) const noexcept
    {
        return std::hash<helics::local_federate_id::base_type>{}(key.baseValue ());
    }
};

template <>
struct hash<helics::interface_handle>
{
    using argument_type = helics::interface_handle;
    using result_type = hash<helics::interface_handle::base_type>::result_type;
    result_type operator() (argument_type const &key) const noexcept
    {
        return std::hash<helics::interface_handle::base_type>{}(key.baseValue ());
    }
};

template <>
struct hash<helics::global_federate_id>
{
    using argument_type = helics::global_federate_id;
    using result_type = std::size_t;
    result_type operator() (argument_type const &key) const noexcept
    {
        return std::hash<helics::global_federate_id::base_type>{}(key.baseValue ());
    }
};

template <>
struct hash<helics::global_broker_id>
{
    using argument_type = helics::global_broker_id;
    using result_type = std::size_t;
    result_type operator() (argument_type const &key) const noexcept
    {
        return std::hash<helics::global_broker_id::base_type>{}(key.baseValue ());
    }
};

template <>
struct hash<helics::route_id>
{
    using argument_type = helics::route_id;
    using result_type = std::size_t;
    result_type operator() (argument_type const &key) const noexcept
    {
        return std::hash<helics::route_id::base_type>{}(key.baseValue ());
    }
};

template <>
struct hash<helics::global_handle>
{
    using argument_type = helics::global_handle;
    using result_type = std::size_t;
    result_type operator() (argument_type const &key) const noexcept
    {
        return std::hash<uint64_t>{}(static_cast<uint64_t> (key));
    }
};

}  // namespace std

template <>
struct is_easily_hashable<helics::global_federate_id>
{
    static constexpr bool value = true;
};

template <>
struct is_easily_hashable<helics::local_federate_id>
{
    static constexpr bool value = true;
};

template <>
struct is_easily_hashable<helics::global_broker_id>
{
    static constexpr bool value = true;
};

template <>
struct is_easily_hashable<helics::route_id>
{
    static constexpr bool value = true;
};

template <>
struct is_easily_hashable<helics::global_handle>
{
    static constexpr bool value = true;
};
