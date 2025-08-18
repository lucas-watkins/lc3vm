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
    template <decltype(COUNT + 0) opcode>
    void exec(std::uint16_t) {
        std::cout << "Invalid Opcode: " << opcode << '\n';
        std::abort();
    }

    template<>
    void exec<BR>(std::uint16_t);

    template <>
    void exec<ADD>(std::uint16_t);

    template <>
    void exec<LD>(std::uint16_t);

    template <>
    void exec<ST>(std::uint16_t);

    template <>
    void exec<JSR>(std::uint16_t);

    template <>
    void exec<AND>(std::uint16_t);

    template <>
    void exec<LDR>(std::uint16_t);

    template <>
    void exec<STR>(std::uint16_t);

    template <>
    void exec<NOT>(std::uint16_t);

    template <>
    void exec<LDI>(std::uint16_t);

    template <>
    void exec<STI>(std::uint16_t);

    template <>
    void exec<JMP>(std::uint16_t);

    template <>
    void exec<LEA>(std::uint16_t);

    template <>
    void exec<TRAP>(std::uint16_t);

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