//
// Created by Lucas Watkins on 8/6/25.
//

#ifndef LC3VM_EXECUTION_HPP
#define LC3VM_EXECUTION_HPP
#include <cstdint>
#include <array>
#include <functional>

namespace Opcodes {

    using opcode_func_t = std::function<void(std::uint16_t)>;

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

    /* Executes break instruction */
    void exec_BR(std::uint16_t);

    /* Executes add instruction */
    void exec_ADD(std::uint16_t);

    /* Executes load instruction */
    void exec_LD(std::uint16_t);

    /* Executes store instruction */
    void exec_ST(std::uint16_t);

    /* Executes jump register instruction */
    void exec_JSR(std::uint16_t);

    /* Executes bitwise and instruction */
    void exec_AND(std::uint16_t);

    /* Executes load direct register instruction */
    void exec_LDR(std::uint16_t);

    /* Executes store direct register instruction */
    void exec_STR(std::uint16_t);

    /* Executes bitwise not instruction */
    void exec_NOT(std::uint16_t);

    /* Executes load indirect instruction */
    void exec_LDI(std::uint16_t);

    /* Executes store indirect instruction */
    void exec_STI(std::uint16_t);

    /* Executes jump instruction */
    void exec_JMP(std::uint16_t);

    /* Executes load effective address instruction */
    void exec_LEA(std::uint16_t);

    /* Executes trap instruction */
    void exec_TRAP(std::uint16_t);

    /* Default catch all for forbidden instructions. Panics and shuts down VM */
    void exec_default(std::uint16_t);

    inline const std::array opcode_funcs {
        opcode_func_t(&exec_BR),
        opcode_func_t(&exec_ADD),
        opcode_func_t(&exec_LD),
        opcode_func_t(&exec_ST),
        opcode_func_t(&exec_JSR),
        opcode_func_t(&exec_AND),
        opcode_func_t(&exec_LDR),
        opcode_func_t(&exec_STR),
        opcode_func_t(&exec_default),
        opcode_func_t(&exec_NOT),
        opcode_func_t(&exec_LDI),
        opcode_func_t(&exec_STI),
        opcode_func_t(&exec_JMP),
        opcode_func_t(&exec_default),
        opcode_func_t(&exec_LEA),
        opcode_func_t(&exec_TRAP),
    };
}

#endif //LC3VM_EXECUTION_HPP