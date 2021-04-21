/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "../core/core-data.hpp"
#include "../core/helics-time.hpp"
#include "helics/external/string_view.hpp"
#include "helics/helics-config.h"
#include "helicsTypes.hpp"

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace helics {
/** class containing a constant view of data block*/
class data_view {
  private:
    stx::string_view dblock;  //!< using a string_view to represent the data
    std::shared_ptr<const data_block>
        ref;  //!< need to capture a reference to the data being viewed if it is from a shared_ptr
  public:
    /** default constructor*/
    data_view() = default;
    /** destructor*/
    ~data_view() = default;
    /** construct from a shared_ptr to a data_block*/
    data_view(std::shared_ptr<const data_block> dt):
        dblock(dt->m_data), ref(std::move(dt)) {}  // NOLINT
    /** construct from a regular data_block*/
    data_view(const data_block& dt) noexcept: dblock(dt.m_data) {}  // NOLINT
    /** copy constructor*/
    data_view(const data_view& dt) noexcept = default;
    /** move constructor*/
    data_view(data_view&& dv) noexcept: dblock(dv.dblock), ref(std::move(dv.ref)) {}

    /** construct from a string*/
    data_view(const char* dt) noexcept: dblock(dt) {}  // NOLINT
    /** construct from a char Pointer and length*/
    data_view(const char* dt, size_t len) noexcept: dblock(dt, len) {}
    /** construct from a string*/
    data_view(const std::string& str) noexcept: dblock(str) {}  // NOLINT
    /** construct from a rValue to a string*/
    data_view(std::string&& str):
        data_view(std::make_shared<data_block>(std::move(str))) {}  // NOLINT
    /** construct from a char vector*/
    data_view(const std::vector<char>& dvec) noexcept:
        dblock(dvec.data(), dvec.size()) {}  // NOLINT
    /** construct from a string_view*/
    data_view(const stx::string_view& sview) noexcept:
        dblock(sview){};  // NOLINT (intended implicit)
    /** assignment operator from another ata_view*/
    data_view& operator=(const data_view& dv) noexcept = default;

    data_view& operator=(data_view&& dv) noexcept
    {
        dblock = dv.dblock;
        ref = std::move(dv.ref);
        return *this;
    }

    /** assignment from a data_block shared_ptr*/
    data_view& operator=(std::shared_ptr<const data_block> dt) noexcept
    {
        dblock = dt->m_data;
        ref = std::move(dt);
        return *this;
    }
    /** assignment from a data_block*/
    data_view& operator=(const data_block& dt) noexcept
    {
        dblock = dt.m_data;
        ref = nullptr;
        return *this;
    }
    /** assignment from a string_view*/
    data_view& operator=(const stx::string_view& str) noexcept
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
    data_block to_data_block() const { return data_block(dblock.data(), dblock.length()); }
    /** swap function */

    void swap(data_view& dv2) noexcept
    {
        dblock.swap(dv2.dblock);
        ref.swap(dv2.ref);
    }
    /** get the data block*/
    const char* data() const noexcept { return dblock.data(); }
    /** get the length*/
    size_t size() const noexcept { return dblock.length(); }
    /** check if the view is empty*/
    bool empty() const noexcept { return dblock.empty(); }
    /** return a string of the data
    @details this actually does a copy to a new string
    */
    std::string string() const { return dblock.to_string(); }
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
inline const char* typeNameString<std::vector<data_block>>()
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
