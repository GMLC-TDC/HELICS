/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "../core/core-data.hpp"
#include "../core/helicsTime.hpp"
#include "helics/helics-config.h"
#include "helicsTypes.hpp"

#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace helics {
/** class containing a constant view of data block*/
class data_view {
  private:
    std::string_view dblock;  //!< using a string_view to represent the data
    std::shared_ptr<const SmallBuffer>
        ref;  //!< need to capture a reference to the data being viewed if it is from a shared_ptr
  public:
    /** default constructor*/
    data_view() = default;
    /** destructor*/
    ~data_view() = default;
    /** construct from a shared_ptr to a data_block*/
    data_view(std::shared_ptr<const SmallBuffer> dt):
        dblock(dt->to_string()), ref(std::move(dt)) {}  // NOLINT
    /** construct from a regular data_block*/
    data_view(const SmallBuffer& dt) noexcept: dblock(dt.to_string()) {}  // NOLINT
    /** copy constructor*/
    data_view(const data_view& dt) noexcept = default;
    /** move constructor*/
    data_view(data_view&& dv) noexcept: dblock(dv.dblock), ref(std::move(dv.ref)) {}
    template<typename U,
             typename T = std::enable_if_t<std::is_constructible_v<std::string_view, U>>>
    data_view(U&& u) noexcept: dblock(std::forward<U>(u))  // NOLINT
    {
    }
    /** construct from a char Pointer and length*/
    data_view(const char* dt, size_t len) noexcept: dblock(dt, len) {}
    /** construct from a rValue to a string*/
    data_view(SmallBuffer&& sb):
        data_view(std::make_shared<SmallBuffer>(std::move(sb))) {}  // NOLINT
    /** construct from a char vector*/
    data_view(const std::vector<char>& dvec) noexcept:
        dblock(dvec.data(), dvec.size()) {}  // NOLINT

    data_view(const std::vector<double>& dvec) noexcept:
        dblock(reinterpret_cast<const char*>(dvec.data()), dvec.size() * sizeof(double))
    {
    }  // NOLINT
    /** assignment operator from another ata_view*/
    data_view& operator=(const data_view& dv) noexcept = default;

    data_view& operator=(data_view&& dv) noexcept
    {
        dblock = dv.dblock;
        ref = std::move(dv.ref);
        return *this;
    }

    /** assignment from a data_block shared_ptr*/
    data_view& operator=(std::shared_ptr<const SmallBuffer> dt) noexcept
    {
        dblock = dt->to_string();
        ref = std::move(dt);
        return *this;
    }
    /** assignment from a data_block*/
    data_view& operator=(const SmallBuffer& dt) noexcept
    {
        dblock = dt.to_string();
        ref = nullptr;
        return *this;
    }
    /** assignment from a string_view*/
    data_view& operator=(const std::string_view& str) noexcept
    {
        dblock = str;
        ref = nullptr;
        return *this;
    }
    /** assignment from a const char * */
    data_view& operator=(const char* s) noexcept
    {
        dblock = s;
        ref = nullptr;
        return *this;
    }
    /** create a new data_block from the data*/
    SmallBuffer to_buffer() const { return SmallBuffer(dblock); }
    /** swap function */

    void swap(data_view& dv2) noexcept
    {
        dblock.swap(dv2.dblock);
        ref.swap(dv2.ref);
    }
    /** get the data block*/
    const char* data() const noexcept { return dblock.data(); }
    /** get the data as a std::byte array*/
    const std::byte* bytes() const noexcept
    {
        return reinterpret_cast<const std::byte*>(dblock.data());
    }
    /** get the length*/
    size_t size() const noexcept { return dblock.length(); }
    /** check if the view is empty*/
    bool empty() const noexcept { return dblock.empty(); }
    /** return a string of the data
    @details this actually does a copy to a new string
    */
    std::string string() const { return std::string(dblock); }
    /** get a string_view object*/
    std::string_view string_view() const { return dblock; }
    /** random access operator*/
    char operator[](int index) const { return dblock[index]; }
    /** begin iterator*/
    auto begin() { return dblock.begin(); }
    /** end iterator*/
    auto end() { return dblock.end(); }
    /** begin const iterator*/
    auto cbegin() const { return dblock.cbegin(); }
    /** end const iterator*/
    auto cend() const { return dblock.cend(); }
};

constexpr auto bvecstr = "block_vector";

template<>
inline const char* typeNameString<std::vector<SmallBuffer>>()
{
    return bvecstr;
}

}  // namespace helics

namespace std {
template<>
inline void swap(helics::data_view& db1, helics::data_view& db2) noexcept
{
    db1.swap(db2);
}
}  // namespace std
