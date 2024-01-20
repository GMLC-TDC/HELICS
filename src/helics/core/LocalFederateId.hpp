/*
Copyright (c) 2017-2024,
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
using IdentifierBaseType = int32_t;

/** class defining a LocalFederateId
@details  the intent of this class is to limit the operations available on a federate identifier
to those that are a actually required and make sense, and make it as low impact as possible.
it also acts to limit any mistakes of a LocalFederateId
*/
class LocalFederateId {
  public:
    using BaseType = IdentifierBaseType;
    /** default constructor*/
    constexpr LocalFederateId() = default;
    /** convert a base type value into a local_federate id*/
    constexpr explicit LocalFederateId(BaseType val) noexcept: fid(val) {}
    /** extract the base value of the id code*/
    constexpr BaseType baseValue() const { return fid; }
    /** equality operator*/
    bool operator==(LocalFederateId id) const noexcept { return (fid == id.fid); }
    /** inequality operator*/
    bool operator!=(LocalFederateId id) const noexcept { return (fid != id.fid); }
    /** less than operator for sorting*/
    bool operator<(LocalFederateId id) const noexcept { return (fid < id.fid); }
    /** check if the operator is valid
    @details valid operators have been set to something other than the default value*/
    bool isValid() const { return (fid != invalid_fid); }

  private:
    static constexpr BaseType invalid_fid{-2'000'000'000};  //!< defined invalid handle
    BaseType fid{invalid_fid};  //!< the underlying index value
};

#if defined HELICS_STATIC_CORE_LIBRARY && !defined HELICS_SHARED_LIBRARY
/** stream operator for a federate_id
 */
std::ostream& operator<<(std::ostream& os, LocalFederateId fid);

#endif
/** constant to use for indicating that a command is for the core itself from the Core Public API*/
constexpr LocalFederateId gLocalCoreId(-259);

namespace detail {
    /** constant numerical value for invalid handle identification*/
    constexpr IdentifierBaseType gInvalidInterfaceHandle{-1'700'000'000};
}  // namespace detail

/** class defining a LocalFederateId
 @details  the intent of this class is to limit the operations available on a federate identifier
 to those that are actually required and make sense, and make it as low impact as possible.
 it also acts to limit any mistakes of a LocalFederateId
 */
class InterfaceHandle {
  public:
    using BaseType = IdentifierBaseType;
    /** default constructor*/
    constexpr InterfaceHandle() = default;

    constexpr explicit InterfaceHandle(BaseType val) noexcept: hid(val) {}
    /** extract the base value of the id code*/
    constexpr BaseType baseValue() const { return hid; }
    /** equality operator*/
    bool operator==(InterfaceHandle id) const noexcept { return (hid == id.hid); }
    /** inequality operator*/
    bool operator!=(InterfaceHandle id) const noexcept { return (hid != id.hid); }
    /** comparison operator for sorting*/
    bool operator<(InterfaceHandle id) const noexcept { return (hid < id.hid); }
    bool operator>(InterfaceHandle id) const noexcept { return (hid > id.hid); }
    bool isValid() const { return (hid != mInvalidHandle); }
    /** get a pointer to the index value type for copying from memory*/
    BaseType* getBaseTypePointer() { return &hid; }

  private:
    static constexpr BaseType mInvalidHandle{detail::gInvalidInterfaceHandle};
    BaseType hid{mInvalidHandle};  //!< the underlying index value
};

#if defined HELICS_STATIC_CORE_LIBRARY && !defined HELICS_SHARED_LIBRARY
/** stream operator for a interface handle
 */
std::ostream& operator<<(std::ostream& os, InterfaceHandle handle);
#endif

constexpr InterfaceHandle gDirectSendHandle{
    -1'745'234};  //!< this special handle can be used to directly send a message in a core

}  // namespace helics

namespace std {
/** define a hash function for LocalFederateId*/
template<>
struct hash<helics::LocalFederateId> {
    /** actual hash operator*/
    std::size_t operator()(helics::LocalFederateId const& key) const noexcept
    {
        return std::hash<helics::LocalFederateId::BaseType>{}(key.baseValue());
    }
};

/** define a template specialization for hash function for InterfaceHandle so it can be used in
 * unordered_map*/
template<>
struct hash<helics::InterfaceHandle> {
    /** actual hash operator*/
    std::size_t operator()(helics::InterfaceHandle const& key) const noexcept
    {
        return std::hash<helics::InterfaceHandle::BaseType>{}(key.baseValue());
    }
};

}  // namespace std
