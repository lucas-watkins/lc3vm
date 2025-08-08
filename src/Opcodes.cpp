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
std::uint16_t Opcodes::sign_extend(const std::uint16_t x, const std::size_t num_bits) {
    if (x >> (num_bits - 1) & 1) // if this statement is true, x represents a negative number
        return x | 0xFFFF << num_bits;
    return x; // otherwise just implicitly cast to std::uint16_t (which fills in 0s)
}

/* Updates register COND with the information about the sign of the previous calculation
 * @param std::uint16_t reg (the register that has the result of the previous calculation)
 */
void Opcodes::update_cond(const std::uint16_t reg) {
    if (Registers::vals[reg] == 0) {
        Registers::vals[Registers::COND] = CondFlags::ZERO;
    } else if (Registers::vals[reg] >> 15 == 0) {
        Registers::vals[Registers::COND] = CondFlags::POS;
    } else if (Registers::vals[reg] >> 15 == 1) {
        Registers::vals[Registers::COND] = CondFlags::NEG;
    }
}
