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

/*
 * Encoding: https://www.jmeiners.com/lc3-vm/supplies/lc3-isa.pdf
 * Increments program counter with offset if any bits 9 through 11 are
 * set and that bit also matches the one that is in the cond register.
 */
template<>
void Opcodes::exec<Opcodes::BR>(const std::uint16_t op) {
    if (op >> 9 & 0x7 && Registers::vals[Registers::COND]) {
        Registers::vals[Registers::PC] += sign_extend(op & 0x1FF, 9);
    }
}

/*
 * Encoding: https://www.jmeiners.com/lc3-vm/img/add_layout.gif
 * Adds two numbers together.
 */
template <>
void Opcodes::exec<Opcodes::ADD>(const std::uint16_t op) {
    const std::uint16_t dr ( op >> 9 & 0x7 ); // destination register
    const std::uint16_t sr1 ( op >> 6 & 0x7 ); // operand register

    // are we in imm5 mode? evaluates to true if we are.
    if (op >> 5 & 0x1) {
        Registers::vals[dr] = Registers::vals[sr1] + sign_extend(op & 0x1F, 5);
    } else {
        const std::uint16_t sr2 ( op & 0x7 );
        Registers::vals[dr] = Registers::vals[sr1] + Registers::vals[sr2];
    }

    update_cond(dr);
}

/*
 * Encoding: https://www.jmeiners.com/lc3-vm/supplies/lc3-isa.pdf
 * Loads value behind program counter + memory offset into direct register
 */
template <>
void Opcodes::exec<Opcodes::LD>(const std::uint16_t op) {
    const std::uint16_t mem_offset { sign_extend(op & 0x1FF, 9) };
    const std::uint16_t dr ( op >> 9 & 0x7 );

    Registers::vals[dr] = Memory::mem[Registers::vals[Registers::PC] + mem_offset];

    update_cond(dr);
}

/*
 * Encoding: https://www.jmeiners.com/lc3-vm/supplies/lc3-isa.pdf
 * Loads value from register sr into memory with the address specified
 * by adding the memory offset to the program counter.
 */
template <>
void Opcodes::exec<Opcodes::ST>(const std::uint16_t op) {
    const std::uint16_t sr ( op >> 9 & 0x7 );
    const std::uint16_t mem_offset { sign_extend(op & 0x1FF, 9) };

    Memory::mem[Registers::vals[Registers::PC] + mem_offset] = Registers::vals[sr];
}

/* Encoding: https://www.jmeiners.com/lc3-vm/supplies/lc3-isa.pdf
 * Sets the program counter to be equal to the value specified in offset
 * by default or value in the specified register. Program counter value
 * is first saved in register 7.
 */
template <>
void Opcodes::exec<Opcodes::JSR>(const std::uint16_t op) {
    Registers::vals[Registers::R7] = Registers::vals[Registers::PC];

    if (op >> 11 & 0x1 /* use offset or not (11th bit)*/) {
        Registers::vals[Registers::PC] += sign_extend(op & 0x7FF, 11);
    } else {
        const std::uint16_t base_r ( op >> 6 & 0x7);
        Registers::vals[Registers::PC] = Registers::vals[base_r];
    }
}

/* Encoding: https://www.jmeiners.com/lc3-vm/supplies/lc3-isa.pdf
 * When bit 5 is zero, bitwise and the value located in sr2
 * otherwise when in imm5 mode use the hardcoded value in that
 * opcode.
 */
template <>
void Opcodes::exec<Opcodes::AND>(const std::uint16_t op) {
    const std::uint16_t sr1 ( op >> 6 & 0x7 );
    const std::uint16_t dr ( op >> 9 & 0x7 );

    if (op >> 5 & 0x1) {
        Registers::vals[dr] = Registers::vals[sr1] & sign_extend(op & 0x1F, 5);
    } else {
        const std::uint16_t sr2 ( op & 0x7 );
        Registers::vals[dr] = Registers::vals[sr1] & Registers::vals[sr2];
    }
}

/*
     * Encoding: https://www.jmeiners.com/lc3-vm/supplies/lc3-isa.pdf
     * Loads the value at the memory address computed by getting the value
     * of the base register and adding the offset into the direct register.
     */
template <>
void Opcodes::exec<Opcodes::LDR>(const std::uint16_t op) {
    const std::uint16_t dr ( op >> 9 & 0x7);
    const std::uint16_t base_r ( op >> 6 & 0x7 );
    const std::uint16_t mem_offset { sign_extend(op & 0x3F, 6) };

    Registers::vals[dr] = Memory::mem[Registers::vals[base_r] + mem_offset];

    update_cond(Registers::vals[dr]);
}

/*
 * Encoding: https://www.jmeiners.com/lc3-vm/supplies/lc3-isa.pdf
 * The value of sr is stored in the memory address computed by
 * the value base register + the offset
 */
template <>
void Opcodes::exec<Opcodes::STR>(const std::uint16_t op) {
    const std::uint16_t sr ( op >> 9 & 0x7 );
    const std::uint16_t base_r ( op >> 6 & 0x7 );
    const std::uint16_t mem_offset { sign_extend(op & 0x3F, 6) };

    Memory::mem[Registers::vals[base_r] + mem_offset] = Registers::vals[sr];
}

/*
 * Encoding: https://www.jmeiners.com/lc3-vm/supplies/lc3-isa.pdf
 * Takes the value of the input register, performs a bitwise not
 * then saves it to the direct register.
 */
template <>
void Opcodes::exec<Opcodes::NOT>(const std::uint16_t op) {
    const std::uint16_t dr ( op >> 9 & 0x7 );
    const std::uint16_t sr1 ( op >> 6 & 0x7 );

    Registers::vals[dr] = ~Registers::vals[sr1];

    update_cond(dr);
}

/*
 * Encoding: https://www.jmeiners.com/lc3-vm/img/ldi_layout.gif
 * Finds value behind address of memory offset + program counter then
 * finds value behind that value and loads it into the direct register
 */
template <>
void Opcodes::exec<Opcodes::LDI>(const std::uint16_t op) {
    const std::uint16_t mem_offset { sign_extend(op & 0x1FF, 9) }; /* Offset of memory to access from PC */
    const std::uint16_t dr ( op >> 9 & 0x7 );

    Registers::vals[dr] = Memory::mem[Memory::mem[mem_offset + Registers::vals[Registers::PC]]];

    update_cond(dr);
}

/*
 * Encoding: https://www.jmeiners.com/lc3-vm/supplies/lc3-isa.pdf
 * The value of the program counter + the offset is used as a memory address which
 * gets a value which is then treated as another memory address and finally the value of SR
 * is stored into that final memory address.
 */
template <>
void Opcodes::exec<Opcodes::STI>(const std::uint16_t op) {
    const std::uint16_t sr ( op >> 9 & 0x7 );
    const std::uint16_t mem_offset { sign_extend(op & 0x1FF, 9) };

    Memory::mem[Memory::mem[Registers::vals[Registers::PC] + mem_offset]] = Registers::vals[sr];
}

/*
 * Encoding: https://www.jmeiners.com/lc3-vm/supplies/lc3-isa.pdf
 * Jumps the program counter unconditionally to the value held in the base register
 */
template <>
void Opcodes::exec<Opcodes::JMP>(const std::uint16_t op) {
    const std::uint16_t base_r ( op >> 6 & 0x7 );

    Registers::vals[Registers::PC] = Registers::vals[base_r];
}

/*
     * Encoding: https://www.jmeiners.com/lc3-vm/supplies/lc3-isa.pdf
     * The value of the program counter + the offset is loaded into the direct register
     */
template <>
void Opcodes::exec<Opcodes::LEA>(const std::uint16_t op) {
    const std::uint16_t dr ( op >> 9 & 0x7 );
    Registers::vals[dr] = Registers::vals[Registers::PC] + sign_extend(op & 0x1FF, 9);

    update_cond(dr);
}

/*
 * Encoding: https://www.jmeiners.com/lc3-vm/supplies/lc3-isa.pdf
 */
template <>
void Opcodes::exec<Opcodes::TRAP>(const std::uint16_t op) {

}
