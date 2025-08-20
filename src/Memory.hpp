//
// Created by Lucas Watkins on 8/5/25.
//

#ifndef LC3VM_MEMORY_HPP
#define LC3VM_MEMORY_HPP
#include <array>
#include "PlatformSpecific.hpp"

namespace Memory {

    /* Memory mapped registers */
    enum {
        KBSR = 0xFE00, /* Keyboard Status Register */
        KBDR = 0xFE02, /* Keyboard Data Register */
    };

    /* Amount of memory that the VM has access to (128 KiB, 65536 locations each 16 bits wide) */
    constexpr std::size_t mem_amt { 1 << 16 };

    /* The memory as an array */
    inline std::array<std::uint16_t, mem_amt> mem;

    inline void write(const std::uint16_t addr, const std::uint16_t val) {
        mem[addr] = val;
    }

    inline std::uint16_t read(const std::uint16_t addr) {
        if (addr == KBSR) {
            if (false) {
                mem[KBSR] = 1 << 15;
                mem[KBDR] = getchar();
            } else {
                mem[KBSR] = 0;
            }
        }

        return mem[addr];
    }

}

#endif //LC3VM_MEMORY_HPP