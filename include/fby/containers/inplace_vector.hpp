#pragma once

#include <cstddef>

namespace fby
{
template<typename T, std::size_t Capacity>
class inplace_vector
{
public:
    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = T&;

public:
    inplace_vector() = default;

private:
};
}