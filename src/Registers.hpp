//
// Created by Lucas Watkins on 8/5/25.
//
#ifndef LC3VM_REGISTERS_HPP
#define LC3VM_REGISTERS_HPP

#include <cstdint>
#include <array>

/* Condition flags used for results of logical comparisons */
namespace CondFlags {
    enum {
        POS  = 1 << 0,
        ZERO  = 1 << 1,
        NEG = 1 << 2,
    };
}

namespace Registers {

    enum {
        R0, /* Registers for data */
        R1,
        R2,
        R3,
        R4,
        R5,
        R6,
        R7,
        PC, /* Program Counter (pointer to current instruction) */
        COND, /* Holds information about previous calculation */
        COUNT, /* Total count of the registers (10) */
    };

    constexpr std::uint16_t pc_start {0x3000}; /* default starting position for the program counter */

    inline std::array<std::uint16_t, COUNT> vals {}; /* values of all registers */

}

#endif //LC3VM_REGISTERS_HPP