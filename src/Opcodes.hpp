//
// Created by Lucas Watkins on 8/6/25.
//

#ifndef LC3VM_EXECUTION_HPP
#define LC3VM_EXECUTION_HPP
#include <cstdint>
#include <iostream>
#include <array>
#include <functional>
#include "Registers.hpp"
#include "Memory.hpp"

namespace Opcodes {

    using opcode_func_t = std::function<void(std::uint16_t)>;

    std::uint16_t sign_extend(std::uint16_t, std::size_t);
    void update_cond(std::uint16_t);

    enum {
        BR,    /* Branch */
        ADD,   /* Add */
        LD,    /* Load */
        ST,    /* Store */
        JSR,   /* Jump register */
        AND,   /* Bitwise and */
        LDR,   /* Load register */
        STR,   /* Store register */
        RTI,   /* Unused */
        NOT,   /* Bitwise not */
        LDI,   /* Load indirect */
        STI,   /* Store indirect */
        JMP,   /* Jump */
        RES,   /* Reserved (unused) */
        LEA,   /* Load effective address */
        TRAP,  /* Execute trap */
        COUNT, /* Count of all Opcodes (16) */
    };

    /*
     * Default unspecialized implementation.
     * Panics as valid instructions are only specialized templates.
     */
    template <int opcode>
    void exec(std::uint16_t) {
        std::cout << "Invalid Opcode: " << opcode << '\n';
        std::abort();
    }

    /*
     * Encoding: https://www.jmeiners.com/lc3-vm/supplies/lc3-isa.pdf
     * Increments program counter with offset if any bits 9 through 11 are
     * set and that bit also matches the one that is in the cond register.
     */
    template<>
    inline void exec<BR>(const std::uint16_t op) {
        if (op >> 9 & 7 && Registers::vals[Registers::COND]) {
            Registers::vals[Registers::PC] += sign_extend(op & 0x1FF, 9);
        }
    }

    /* Encoding: https://www.jmeiners.com/lc3-vm/img/add_layout.gif
     * Adds two numbers together.
     */
    template <>
    inline void exec<ADD>(const std::uint16_t op) {
        const std::uint16_t dr ( op >> 9 & 7 ); // destination register
        const std::uint16_t sr1 ( op >> 6 & 7 ); // operand register

        // are we in imm5 mode? evaluates to true if we are.
        if (op >> 5 & 1) {
            Registers::vals[dr] = Registers::vals[sr1] + sign_extend(op & 0x1F, 5);
        } else {
            const std::uint16_t sr2 ( op & 7 );
            Registers::vals[dr] = Registers::vals[sr1] + Registers::vals[sr2];
        }

        update_cond(dr);
    }

    /*
     * Encoding: https://www.jmeiners.com/lc3-vm/supplies/lc3-isa.pdf
     * Loads value behind program counter + memory offset into direct register
     */
    template <>
    inline void exec<LD>(const std::uint16_t op) {
        const std::uint16_t mem_offset { sign_extend(op & 0x1FF, 9) };
        const std::uint16_t dr ( op >> 9 & 7 );

        Registers::vals[dr] = Memory::mem[Registers::vals[Registers::PC] + mem_offset];

        update_cond(dr);
    }

    /*
     * Encoding: https://www.jmeiners.com/lc3-vm/supplies/lc3-isa.pdf
     * Loads value from register sr into memory with the address specified
     * by adding the memory offset to the program counter.
     */
    template <>
    inline void exec<ST>(const std::uint16_t op) {
        const std::uint16_t sr ( op >> 9 & 7 );
        const std::uint16_t mem_offset { sign_extend(op & 0x1FF, 9) };

        Memory::mem[Registers::vals[Registers::PC] + mem_offset] = Registers::vals[sr];
    }

    /* Encoding: https://www.jmeiners.com/lc3-vm/supplies/lc3-isa.pdf
     * Sets the program counter to be equal to the value specified in offset
     * by default or value in the specified register. Program counter value
     * is first saved in register 7.
     */
    template <>
    inline void exec<JSR>(const std::uint16_t op) {
        Registers::vals[Registers::R7] = Registers::vals[Registers::PC];

        if (op >> 11 & 1 /* use offset or not (11th bit)*/) {
            Registers::vals[Registers::PC] += sign_extend(op & 0x7FF, 11);
        } else {
            const std::uint16_t base_r ( op >> 6 & 7);
            Registers::vals[Registers::PC] = Registers::vals[base_r];
        }
    }

    /* Encoding: https://www.jmeiners.com/lc3-vm/supplies/lc3-isa.pdf
     * When bit 5 is zero, bitwise and the value located in sr2
     * otherwise when in imm5 mode use the hardcoded value in that
     * opcode.
     */
    template <>
    inline void exec<AND>(const std::uint16_t op) {
        const std::uint16_t sr1 ( op >> 6 & 7 );
        const std::uint16_t dr ( op >> 9 & 7 );

        if (op >> 5 & 1) {
            Registers::vals[dr] = Registers::vals[sr1] & sign_extend(op & 0x1F, 5);
        } else {
            const std::uint16_t sr2 ( op & 7 );
            Registers::vals[dr] = Registers::vals[sr1] & Registers::vals[sr2];
        }
    }

    /*
     * Encoding: https://www.jmeiners.com/lc3-vm/supplies/lc3-isa.pdf
     * Loads the value at the memory address computed by getting the value
     * of the base register and adding the offset into the direct register.
     */
    template <>
    inline void exec<LDR>(const std::uint16_t op) {
        const std::uint16_t dr ( op >> 9 & 7);
        const std::uint16_t base_r ( op >> 6 & 7 );
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
    inline void exec<STR>(const std::uint16_t op) {
        const std::uint16_t sr ( op >> 9 & 7 );
        const std::uint16_t base_r ( op >> 6 & 7 );
        const std::uint16_t mem_offset { sign_extend(op & 0x3F, 6) };

        Memory::mem[Registers::vals[base_r] + mem_offset] = Registers::vals[sr];
    }

    /*
     * Encoding: https://www.jmeiners.com/lc3-vm/supplies/lc3-isa.pdf
     * Takes the value of the input register, performs a bitwise not
     * then saves it to the direct register.
     */
    template <>
    inline void exec<NOT>(const std::uint16_t op) {
        const std::uint16_t dr ( op >> 9 & 7 );
        const std::uint16_t sr1 ( op >> 6 & 7 );

        Registers::vals[dr] = ~Registers::vals[sr1];

        update_cond(dr);
    }

    /*
     * Encoding: https://www.jmeiners.com/lc3-vm/img/ldi_layout.gif
     * Finds value behind address of memory offset + program counter then
     * finds value behind that value and loads it into the direct register
     */
    template <>
    inline void exec<LDI>(const std::uint16_t op) {
        const std::uint16_t mem_offset { sign_extend(op & 0x1FF, 9) }; /* Offset of memory to access from PC */
        const std::uint16_t dr ( op >> 9 & 7 );

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
    inline void exec<STI>(const std::uint16_t op) {
        const std::uint16_t sr ( op >> 9 & 7 );
        const std::uint16_t mem_offset { sign_extend(op & 0x1FF, 9) };

        Memory::mem[Memory::mem[Registers::vals[Registers::PC] + mem_offset]] = Registers::vals[sr];
    }

    /*
     * Encoding: https://www.jmeiners.com/lc3-vm/supplies/lc3-isa.pdf
     * Jumps the program counter unconditionally to the value held in the base register
     */
    template <>
    inline void exec<JMP>(const std::uint16_t op) {
        const std::uint16_t base_r ( op >> 6 & 7 );

        Registers::vals[Registers::PC] = Registers::vals[base_r];
    }

    /*
     * Encoding: https://www.jmeiners.com/lc3-vm/supplies/lc3-isa.pdf
     * The value of the program counter + the offset is loaded into the direct register
     */
    template <>
    inline void exec<LEA>(const std::uint16_t op) {
        const std::uint16_t dr ( op >> 9 & 7 );
        Registers::vals[dr] = Registers::vals[Registers::PC] + sign_extend(op & 0x1FF, 9);

        update_cond(dr);
    }

    /*
     * Encoding: https://www.jmeiners.com/lc3-vm/supplies/lc3-isa.pdf
     */
    template <>
    inline void exec<TRAP>(const std::uint16_t) {

    }

    inline const std::array opcode_funcs {
        opcode_func_t(&exec<BR>),
        opcode_func_t(&exec<ADD>),
        opcode_func_t(&exec<LD>),
        opcode_func_t(&exec<ST>),
        opcode_func_t(&exec<JSR>),
        opcode_func_t(&exec<AND>),
        opcode_func_t(&exec<LDR>),
        opcode_func_t(&exec<STR>),
        opcode_func_t(&exec<RES> /* Panics if executed */),
        opcode_func_t(&exec<NOT>),
        opcode_func_t(&exec<LDI>),
        opcode_func_t(&exec<STI>),
        opcode_func_t(&exec<JMP>),
        opcode_func_t(&exec<RES> /* Panics if executed */),
        opcode_func_t(&exec<LEA>),
        opcode_func_t(&exec<TRAP>),
    };
}

#endif //LC3VM_EXECUTION_HPP