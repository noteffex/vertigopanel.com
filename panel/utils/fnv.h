#pragma once

#include <cstdint>

namespace fnv {
    constexpr uint32_t offset_basis = 0x811c9dc5;
    constexpr uint32_t prime = 0x1000193;

    constexpr uint32_t hash(const char* str, const uint32_t value = offset_basis) noexcept
    {
        return *str ? hash(str + 1, (value ^ *str) * static_cast<unsigned long long>(prime)) : value;
    }

    constexpr uint32_t hash_runtime(const char* str) noexcept
    {
        auto value = offset_basis;

        while (*str) {
            value ^= *str++;
            value *= prime;
        }
        return value;
    }
}