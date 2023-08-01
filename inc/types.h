#pragma once

#include <cstdint>

using hword = uint32_t;
static constexpr uint64_t HIGH_32_MASK = 0x00'00'00'00'FF'FF'FF'FF;
static constexpr hword HW0 = 0x00'00'00'00;
static constexpr hword A0 = 0x01'23'45'67;
static constexpr hword B0 = 0x89'ab'cd'ef;
static constexpr hword C0 = 0xfe'dc'ba'98;
static constexpr hword D0 = 0x76'54'32'10;
