//
// Created by Lucas Watkins on 8/15/25.
//

#ifndef LC3VM_TRAP_HPP
#define LC3VM_TRAP_HPP
#include <iostream>

namespace Trap {

    enum /* Trap Codes */ {
        GETC  = 0x20,  /* Get character from keyboard (no echo onto terminal) */
        OUT   = 0x21,   /* Output a character */
        PUTS  = 0x22,  /* Outputs a word string */
        IN    = 0x23,    /* Get character from keyboard (echoed onto terminal) */
        PUTSP = 0x24, /* Outputs a byte string */
        HALT  = 0x25,  /* Halts the entire program */
        COUNT = 0x6, /* Count of all Trap Codes */
    };

    template <decltype(COUNT + 0) trap_code>
    void exec() {
        std::cout << "Invalid Trap Code: " << trap_code << '\n';
        std::abort();
    };

    template <>
    void exec<GETC>();

    template <>
    void exec<OUT>();

    template <>
    void exec<PUTS>();

    template <>
    void exec<IN>();

    template <>
    void exec<PUTSP>();

    template <>
    void exec<HALT>();

}

#endif //LC3VM_TRAP_HPP