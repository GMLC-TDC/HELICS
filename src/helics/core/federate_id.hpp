/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once
#include <cstdint>
#include <functional>
#include <iosfwd>

namespace helics {
/** base date type for id types*/
using identififier_base_type = int32_t;

/** class defining a local_federate_id
@details  the intent of this class is to limit the operations available on a federate identifier
to those that are a actually required and make sense, and make it as low impact as possible.
it also acts to limit any mistakes of a local_federate_id
*/
class local_federate_id {
  public:
    using base_type = identififier_base_type;
    /** default constructor*/
    constexpr local_federate_id() = default;
    /** convert a base type value into a local_federate id*/
    constexpr explicit local_federate_id(base_type val) noexcept: fid(val) {}
    /** extract the base value of the id code*/
    constexpr base_type baseValue() const { return fid; }
    /** equality operator*/
    bool operator==(local_federate_id id) const noexcept { return (fid == id.fid); }
    /** inequality operator*/
    bool operator!=(local_federate_id id) const noexcept { return (fid != id.fid); }
    /** less than operator for sorting*/
    bool operator<(local_federate_id id) const noexcept { return (fid < id.fid); }
    /** check if the operator is valid
    @details valid operators have been set to something other than the default value*/
    bool isValid() const { return (fid != invalid_fid); }

  private:
    static constexpr base_type invalid_fid{-2'000'000'000};  //!< defined invalid handle
    base_type fid{invalid_fid};  //!< the underlying index value
};

#if defined HELICS_STATIC_CORE_LIBRARY && !defined HELICS_SHARED_LIBRARY
/** stream operator for a federate_id
 */
std::ostream& operator<<(std::ostream& os, local_federate_id fid);

#endif
/** constant to use for indicating that a command is for the core itself from the Core Public API*/
constexpr local_federate_id local_core_id(-259);

namespace detail {
    /** constant numerical value for invalid handle identification*/
    constexpr identififier_base_type invalid_interface_handle{-1'700'000'000};
}  // namespace detail

/** class defining a local_federate_id
@details  the intent of this class is to limit the operations available on a federate identifier
to those that are actually required and make sense, and make it as low impact as possible.
it also acts to limit any mistakes of a local_federate_id
*/
class interface_handle {
  public:
    using base_type = identififier_base_type;
    /** default constructor*/
    constexpr interface_handle() = default;

    constexpr explicit interface_handle(base_type val) noexcept: hid(val) {}
    /** extract the base value of the id code*/
    constexpr base_type baseValue() const { return hid; }
    /** equality operator*/
    bool operator==(interface_handle id) const noexcept { return (hid == id.hid); }
    /** inequality operator*/
    bool operator!=(interface_handle id) const noexcept { return (hid != id.hid); }
    /** comparison operator for sorting*/
    bool operator<(interface_handle id) const noexcept { return (hid < id.hid); }
    bool operator>(interface_handle id) const noexcept { return (hid > id.hid); }
    bool isValid() const { return (hid != invalid_handle); }

  private:
    static constexpr base_type invalid_handle{detail::invalid_interface_handle};
    base_type hid{invalid_handle};  //!< the underlying index value
};

#if defined HELICS_STATIC_CORE_LIBRARY && !defined HELICS_SHARED_LIBRARY
/** stream operator for a interface handle
 */
std::ostream& operator<<(std::ostream& os, interface_handle handle);
#endif

/// this special handle can be used to directly send a message in a core
constexpr interface_handle direct_send_handle{-1'745'234};

}  // namespace helics

namespace std {
/** define a hash function for local_federate_id*/
template<>
struct hash<helics::local_federate_id> {
    using argument_type = helics::local_federate_id;  //!< type of object to hash
    using result_type =
        hash<helics::local_federate_id::base_type>::result_type;  //!< the type of the return result
    /** actual hash operator*/
    result_type operator()(argument_type const& key) const noexcept
    {
        return std::hash<helics::local_federate_id::base_type>{}(key.baseValue());
    }
};

/** define a template specialization for hash function for interface_handle so it can be used in
 * unordered_map*/
template<>
struct hash<helics::interface_handle> {
    using argument_type = helics::interface_handle;  //!< type of object to hash
    using result_type =
        hash<helics::interface_handle::base_type>::result_type;  //!< the type of the return result
    /** actual hash operator*/
    result_type operator()(argument_type const& key) const noexcept
    {
        return std::hash<helics::interface_handle::base_type>{}(key.baseValue());
    }
};

}  // namespace std
