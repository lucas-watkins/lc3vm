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

    template <int opcode>
    void exec(std::uint16_t) {
        /* Default unspecialized implementation. Panics as valid instructions are only specialized templates */
        std::cout << "Invalid Opcode: " << opcode << '\n';
        std::abort();
    }

    // Encoding: https://www.jmeiners.com/lc3-vm/img/add_layout.gif
    template <>
    inline void exec<ADD>(const std::uint16_t op) {
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

    template <>
    inline void exec<LD>(const std::uint16_t) {

    }

    template <>
    inline void exec<ST>(const std::uint16_t) {

    }

    template <>
    inline void exec<JSR>(const std::uint16_t) {

    }

    template <>
    inline void exec<AND>(const std::uint16_t) {

    }

    template <>
    inline void exec<LDR>(const std::uint16_t) {

    }

    template <>
    inline void exec<STR>(const std::uint16_t) {

    }

    template <>
    inline void exec<NOT>(const std::uint16_t) {

    }

    template <>
    inline void exec<LDI>(const std::uint16_t) {

    }

    template <>
    inline void exec<STI>(const std::uint16_t) {

    }

    template <>
    inline void exec<JMP>(const std::uint16_t) {

    }

    template <>
    inline void exec<LEA>(const std::uint16_t) {

    }

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