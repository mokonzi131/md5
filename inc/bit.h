#pragma once

#include <concepts>

// NOTE: C++23 has this function in the standard library under
// the <bit> header. Since we are using C++20, here is an implementation
// ref: https://en.cppreference.com/w/cpp/numeric/byteswap

template<std::integral T>
constexpr T byteswap(T value) noexcept
{
    static_assert(std::has_unique_object_representations_v<T>, 
                  "T may not have padding bits");
    auto value_representation = std::bit_cast<std::array<std::byte, sizeof(T)>>(value);
    std::ranges::reverse(value_representation);
    return std::bit_cast<T>(value_representation);
}
