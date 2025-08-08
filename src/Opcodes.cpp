//
// Created by Lucas Watkins on 8/6/25.
//

#include "Opcodes.hpp"
#include "Registers.hpp"

/* Extends number into std::uint16_t. Fills in 0s for positive numbers
 * and 1s for negative numbers. The leftmost bit is the sign bit
 * (0 -> positive, 1 -> negative).
 * @param std::uint16_t x (the actual number being extended)
 * @param std::size_t num_bits (the bits that x originally had)
 */
std::uint16_t sign_extend(const std::uint16_t x, const std::size_t num_bits) {
    if (x >> (num_bits - 1) & 1) // if this statement is true, x represents a negative number
        return x | 0xFFFF << num_bits;
    return x; // otherwise just implicitly cast to std::uint16_t (which fills in 0s)
}

/* Updates register COND with the information about the sign of the previous calculation
 * @param std::uint16_t reg (the register that has the result of the previous calculation)
 */
void update_cond(const std::uint16_t reg) {
    if (Registers::vals[reg] == 0) {
        Registers::vals[Registers::COND] = CondFlags::ZERO;
    } else if (Registers::vals[reg] >> 15 == 0) {
        Registers::vals[Registers::COND] = CondFlags::POS;
    } else if (Registers::vals[reg] >> 15 == 1) {
        Registers::vals[Registers::COND] = CondFlags::NEG;
    }
}

void Opcodes::exec_BR(const std::uint16_t) {

}

// Encoding: https://www.jmeiners.com/lc3-vm/img/add_layout.gif
void Opcodes::exec_ADD(const std::uint16_t op) {
    const std::uint16_t dr ( op >> 9 & 7 ); // destination register
    const std::uint16_t sr1 ( op >> 6 & 7 ); // operand register

    // are we in imm5 mode? evaluates to true if we are.
    if (op >> 5 & 1) {
        Registers::vals[dr] = Registers::vals[sr1] + sign_extend(op & 0x1F, 5);
    } else {
        Registers::vals[dr] = Registers::vals[sr1] + Registers::vals[op & 7 /* second operand register */];
    }

    update_cond(dr);
}

void Opcodes::exec_LD(const std::uint16_t) {

}

void Opcodes::exec_ST(const std::uint16_t) {

}

void Opcodes::exec_JSR(const std::uint16_t) {

}

void Opcodes::exec_AND(const std::uint16_t) {

}

void Opcodes::exec_LDR(const std::uint16_t) {

}

void Opcodes::exec_STR(const std::uint16_t) {

}

void Opcodes::exec_NOT(const std::uint16_t) {

}

void Opcodes::exec_LDI(const std::uint16_t) {

}

void Opcodes::exec_STI(const std::uint16_t) {

}

void Opcodes::exec_JMP(const std::uint16_t) {

}

void Opcodes::exec_LEA(const std::uint16_t) {

}

void Opcodes::exec_TRAP(const std::uint16_t) {

}

void Opcodes::exec_default(const std::uint16_t) {

}
