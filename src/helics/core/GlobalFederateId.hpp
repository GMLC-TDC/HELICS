/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once
#include "LocalFederateId.hpp"

namespace helics {
/** a shift in the global federate id numbers to allow discrimination between local ids and global
   ones this value allows 131072 federates to be available in each core 1,878,917,120 allowable
   federates in the system and 268,435,455 brokers allowed  if we need more than that this, HELICS
   as a program has been phenomenally successful beyond all wildest imaginations and we can probably
   afford to change these to 64 bit numbers to accommodate.  Of the available federates there are 2
   federate numbers that can be defined per core/broker for various purposes.  These are the upper
   number of federate id's so 536,870,910 reserved federate id's.  An ID of 1 is reserved for the
   root broker

    */
constexpr IdentifierBaseType gGlobalFederateIdShift{0x0002'0000};
/** a shift in the global id index to discriminate between global ids of brokers vs federates*/
constexpr IdentifierBaseType gGlobalBrokerIdShift{0x7000'0000};
/** the block size for global priority levels*/
constexpr IdentifierBaseType gGlobalPriorityBlockSize{100'000'000};

/** class holding a globally unique identifier for brokers
    @details the class is fully compatible with GlobalFederateId*/
class GlobalBrokerId {
  public:
    using BaseType = IdentifierBaseType;
    /** default constructor*/
    constexpr GlobalBrokerId() = default;

    constexpr explicit GlobalBrokerId(BaseType val) noexcept: gid(val) {}
    /** extract the base value of the id code*/
    constexpr BaseType baseValue() const { return gid; }
    /** equality operator*/
    constexpr bool operator==(GlobalBrokerId id) const noexcept { return (gid == id.gid); }
    /** inequality operator*/
    constexpr bool operator!=(GlobalBrokerId id) const noexcept { return (gid != id.gid); }
    /** less than operator for sorting*/
    constexpr bool operator<(GlobalBrokerId id) const noexcept { return (gid < id.gid); }

    constexpr bool isFederate() const
    {
        return ((gid >= gGlobalFederateIdShift) && (gid < gGlobalBrokerIdShift));
    }
    constexpr bool isBroker() const { return (gid >= gGlobalBrokerIdShift || gid == 1); }
    constexpr bool isValid() const
    {
        return (gid != invalid_global_broker_id && gid != detail::gInvalidInterfaceHandle);
    }
    BaseType localIndex() const
    {
        return (gid >= gGlobalBrokerIdShift) ? (gid - gGlobalBrokerIdShift) : gid;
    }

  private:
    BaseType gid = invalid_global_broker_id;  //!< the underlying index value
    friend class GlobalFederateId;  // for allowing comparison operators to work well
    static constexpr BaseType invalid_global_broker_id{-2'010'000'000};
};

/** constant to use for indicating that a command is for the parent object*/
constexpr GlobalBrokerId parent_broker_id{0};
/** constant to use for indicating the id of the root broker*/
constexpr GlobalBrokerId gRootBrokerID{1};

/** stream operator for a federate_id
 */
std::ostream& operator<<(std::ostream& os, GlobalBrokerId id);
/** class holder a globally unique identifier for federates*/
class GlobalFederateId {
  public:
    using BaseType = IdentifierBaseType;
    /** default constructor*/
    constexpr GlobalFederateId() = default;

    constexpr explicit GlobalFederateId(BaseType val) noexcept: gid(val) {}
    /** implicit conversion from global_id*/
    constexpr GlobalFederateId(GlobalBrokerId id) noexcept: gid(id.gid) {}  // NOLINT

    constexpr operator GlobalBrokerId() const noexcept { return GlobalBrokerId{gid}; }  // NOLINT
    /** conversion to the BaseType*/
    constexpr BaseType baseValue() const { return gid; }
    /** equality operator*/
    constexpr bool operator==(GlobalFederateId id) const noexcept { return (gid == id.gid); }
    /** inequality operator*/
    constexpr bool operator!=(GlobalFederateId id) const noexcept { return (gid != id.gid); }
    /** less than operator for sorting*/
    constexpr bool operator<(GlobalFederateId id) const noexcept { return (gid < id.gid); }
    /** greater than operator for sorting*/
    constexpr bool operator>(GlobalFederateId id) const noexcept { return (gid > id.gid); }
    /** equality operator*/
    constexpr bool operator==(GlobalBrokerId id) const noexcept { return (gid == id.gid); }
    /** inequality operator*/
    constexpr bool operator!=(GlobalBrokerId id) const noexcept { return (gid != id.gid); }
    /** less than operator for sorting*/
    constexpr bool operator<(GlobalBrokerId id) const noexcept { return (gid < id.gid); }
    /** greater than operator for sorting*/
    constexpr bool operator>(GlobalBrokerId id) const noexcept { return (gid > id.gid); }
    /** return true if the broker_id is a valid federate id code*/
    constexpr bool isFederate() const
    {
        return ((gid >= gGlobalFederateIdShift) && (gid < gGlobalBrokerIdShift));
    }
    /** return true if the broker_id is a valid broker id code*/
    constexpr bool isBroker() const { return (gid >= gGlobalBrokerIdShift || gid == 1); }
    /** return true if the broker_id is a valid broker id code*/
    constexpr bool isValid() const
    {
        return (gid != invalid_global_fed_id && gid != detail::gInvalidInterfaceHandle);
    }
    /** generate a local offset index
        @details the global_id is shifted by a certain amount*/
    constexpr BaseType localIndex() const
    {
        return (gid >= gGlobalFederateIdShift) ? (gid - gGlobalFederateIdShift) : gid;
    }

    /** get a pointer to the index value type for copying from memory*/
    BaseType* getBaseTypePointer() { return &gid; }

  private:
    static constexpr BaseType invalid_global_fed_id{-2'010'000'000};
    BaseType gid{invalid_global_fed_id};  //!< the underlying index value
};
/** identifier to target the local core even if the global id hasn't been assigned yet*/
constexpr GlobalFederateId gDirectCoreId{-235262};

/** generate a special federate id for a broker/core */
constexpr GlobalFederateId getSpecialFederateId(GlobalBrokerId broker,
                                                GlobalBrokerId::BaseType index)
{
    // allow 3 special federates per subbroker/core + 3 special ones for the root broker
    return (broker != gRootBrokerID) ?
        (GlobalFederateId(gGlobalBrokerIdShift -
                          3 * (broker.baseValue() - gGlobalBrokerIdShift + 2) + index)) :
        GlobalFederateId(gGlobalBrokerIdShift - index - 1);
}
/** stream operator for a federate_id*/
std::ostream& operator<<(std::ostream& os, GlobalFederateId id);

/** class merging a global id and handle together */
class GlobalHandle {
  public:
    GlobalFederateId fed_id = GlobalFederateId{};  //!< the federate id component
    InterfaceHandle handle = InterfaceHandle{};  //!< the interface handle component
    /** default constructor*/
    constexpr GlobalHandle() = default;
    /** construct directly from a federate_id and InterfaceHandle*/
    constexpr GlobalHandle(GlobalFederateId fed, InterfaceHandle hand): fed_id(fed), handle(hand) {}
    /** convert to a uint64_t
        @details for use in maps and other things*/
    explicit operator uint64_t() const
    {
        auto key = static_cast<uint64_t>(fed_id.baseValue()) << 32U;
        key += static_cast<uint64_t>(handle.baseValue()) & (0x0000'0000'FFFF'FFFF);
        return key;
    }
    /** equality operator*/
    constexpr bool operator==(GlobalHandle id) const noexcept
    {
        return ((fed_id == id.fed_id) && (handle == id.handle));
    }
    /** inequality operator*/
    constexpr bool operator!=(GlobalHandle id) const noexcept
    {
        return ((fed_id != id.fed_id) || (handle != id.handle));
    }
    /** less than operator for sorting*/
    constexpr bool operator<(GlobalHandle id) const noexcept
    {
        return (fed_id < id.fed_id) ? true : ((fed_id != id.fed_id) ? false : (handle < id.handle));
    }
    /** check if the handle points to a validly specified interface*/
    constexpr bool isValid() const { return fed_id.isValid() && handle.isValid(); }
};

/** stream operator for a federate_id
 */
std::ostream& operator<<(std::ostream& os, GlobalHandle id);

/** class defining a specific type for labeling a route*/
class route_id {
  public:
    using BaseType = IdentifierBaseType;
    /** default constructor*/
    constexpr route_id() = default;
    /** construct from a BaseType value*/
    constexpr explicit route_id(BaseType val) noexcept: rid(val) {}
    /** explicitly convert to a base value */
    constexpr BaseType baseValue() const { return rid; }
    /** equality operator*/
    constexpr bool operator==(route_id id) const noexcept { return (rid == id.rid); }
    /** inequality operator*/
    constexpr bool operator!=(route_id id) const noexcept { return (rid != id.rid); }
    /** less than operator for sorting*/
    constexpr bool operator<(route_id id) const noexcept { return (rid < id.rid); }
    /** check if the route is valid*/
    constexpr bool isValid() const { return (rid != invalid_route_id); }

  private:
    static constexpr BaseType invalid_route_id{-1'295'148'000};
    BaseType rid{invalid_route_id};  //!< the underlying index value
};

constexpr route_id parent_route_id{0};
constexpr route_id control_route{-1};

constexpr route_id generateRouteId(int32_t route_type_code, int32_t index)
{
    return route_id(route_type_code * 256 * 256 * 256 + index);
}

constexpr int32_t getRouteTypeCode(route_id rid)
{
    return (rid.baseValue() >> 24);
}

constexpr int32_t normal_route_code{0};
constexpr int32_t json_route_code{10};
/** stream operator for a route_id
 */
std::ostream& operator<<(std::ostream& os, route_id id);

}  // namespace helics

namespace std {
/** define a template specialization for hash function for GlobalFederateId so it can be used in
 * unordered_map*/
template<>
struct hash<helics::GlobalFederateId> {
    using argument_type = helics::GlobalFederateId;  //!< typedef for input type of hash
    using result_type = std::size_t;  //!< typedef for output result
    /** hash operator*/
    result_type operator()(argument_type const& key) const noexcept
    {
        return std::hash<helics::GlobalFederateId::BaseType>{}(key.baseValue());
    }
};

/** define a template specialization for hash function for GlobalBrokerId so it can be used in
 * unordered_map*/
template<>
struct hash<helics::GlobalBrokerId> {
    using argument_type = helics::GlobalBrokerId;  //!< typedef for input type of hash
    using result_type = std::size_t;  //!< typedef for output result
    /** hash operator*/
    result_type operator()(argument_type const& key) const noexcept
    {
        return std::hash<helics::GlobalBrokerId::BaseType>{}(key.baseValue());
    }
};

/** define a template specialization for hash function for route_id so it can be used in
 * unordered_map*/
template<>
struct hash<helics::route_id> {
    using argument_type = helics::route_id;  //!< typedef for input type of hash
    using result_type = std::size_t;  //!< typedef for output result
    /** hash operator*/
    result_type operator()(argument_type const& key) const noexcept
    {
        return std::hash<helics::route_id::BaseType>{}(key.baseValue());
    }
};

/** define a template specialization for hash function for GlobalHandle so it can be used in
unordered_map
@details based on hash of std::uint64_t*/
template<>
struct hash<helics::GlobalHandle> {
    using argument_type = helics::GlobalHandle;  //!< typedef for input type of hash
    using result_type = std::size_t;  //!< typedef for output result
    /** hash operator*/
    result_type operator()(argument_type const& key) const noexcept
    {
        return std::hash<uint64_t>{}(static_cast<uint64_t>(key));
    }
};

}  // namespace std
