//
// Created by Lucas Watkins on 8/15/25.
//

#include "Trap.hpp"
#include "Memory.hpp"
#include "Opcodes.hpp"
#include "Registers.hpp"
#include "PlatformSpecific.hpp"

/* Reads a character and returns it */
unsigned char read_char() {
    const unsigned char c ( std::cin.get() );

    // no std::cin.ignore() because we already disabled the buffer earlier

    return c;
}

template <>
void Trap::exec<Trap::PUTS>() {
    // String's starting address is in register 0
    std::uint16_t addr { Registers::vals[Registers::R0] };

    while (Memory::read(addr) != 0x0) {
        std::cout << static_cast<unsigned char>(Memory::read(addr++));
    }

    std::cout << std::flush;
}

/* Outputs a single character located in register 0 */
template <>
void Trap::exec<Trap::OUT>() {
    std::cout << static_cast<unsigned char>(Registers::vals[Registers::R0]) << std::flush;
}

template<>
void Trap::exec<Trap::IN>() {
    std::cout << "Enter a character: ";
    const unsigned char c ( read_char() );
    std::cout << c << std::flush;

    Registers::vals[Registers::R0] = c;
    Opcodes::update_cond(Registers::R0);
}

template<>
void Trap::exec<Trap::PUTSP>() {
    std::uint16_t addr { Registers::vals[Registers::R0] };

    while (Memory::read(addr) != 0x0) {
        const std::uint16_t chars { Memory::read(addr) };
        const unsigned char char1 ( chars & 0xFF );
        const unsigned char char2 ( chars >> 8 );

        std::cout << char1;
        if (char2) {
            std::cout << char2;
        }

        ++addr;
    }
    std::cout << std::flush;
}

template <>
void Trap::exec<Trap::GETC>() {
    Registers::vals[Registers::R0] = read_char();
    Opcodes::update_cond(Registers::R0);
}