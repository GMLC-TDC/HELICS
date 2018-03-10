#ifndef TOM_COMMON_HPP__
#define TOM_COMMON_HPP__

#include <helics/application_api/ValueFederate.hpp>

template <typename T>
struct ValuePacket
{
    helics::Time time_;
    helics::publication_id_t id_;
    T value_;

    ValuePacket () = default;
    ValuePacket (helics::Time time, helics::publication_id_t id, T value) : time_{time}, id_{id}, value_{value} {}
};  // struct ValuePacket

#endif /* TOM_COMMON_HPP__ */

