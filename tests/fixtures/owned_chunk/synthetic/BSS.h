// SPDX-License-Identifier: MIT
#pragma once
#include <stdint.h>
// Synthetic codec location only; not an original game field offset.
struct __attribute__((packed)) BssLayout { uint8_t zero_layout[680644]; };
alignas(4) extern BssLayout _bss;
#define dword_5A179C (reinterpret_cast<int8_t *>(&_bss.zero_layout[0]))
