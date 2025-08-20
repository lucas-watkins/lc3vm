//
// Created by Lucas Watkins on 8/15/25.
//

#include "Trap.hpp"
#include "Memory.hpp"
#include "Opcodes.hpp"
#include "Registers.hpp"
#include "PlatformSpecific.hpp"

/* Reads a character and copies it into register 0 */
void read_char() {
    Registers::vals[Registers::R0] = static_cast<unsigned char>( std::cin.get() );
    std::cin.ignore(std::numeric_limits<std::streamsize>::max());
}

template <>
void Trap::exec<Trap::PUTS>() {
    // String's starting address is in register 0
    std::uint16_t addr { Registers::vals[Registers::R0] };

    while (Memory::read(addr) != 0x0) {
        putc(static_cast<char>(Memory::read(addr++)), stdout);
    }

    fflush(stdout);
}

template <>
void Trap::exec<Trap::HALT>() {
    puts("\n** Program Halted **\n");
    fflush(stdout);
    //restore_input_buffering();
    std::exit(EXIT_SUCCESS);
}

/* Outputs a single character located in register 0 */
template <>
void Trap::exec<Trap::OUT>() {
    putc(static_cast<char>(Registers::vals[Registers::R0]), stdout);
    fflush(stdout);
}

template<>
void Trap::exec<Trap::IN>() {
    printf("Enter a character: ");
    const char c = getchar();
    putc(c, stdout);
    fflush(stdout);
    Registers::vals[Registers::R0] = static_cast<std::uint16_t>( c );
    Opcodes::update_cond(Registers::R0);
}

template<>
void Trap::exec<Trap::PUTSP>() {
    std::uint16_t addr { Registers::vals[Registers::R0] };

    while (Memory::read(addr) != 0x0) {
        const std::uint16_t chars { Memory::read(addr) };
        const unsigned char char1 ( chars & 0xFF );
        const unsigned char char2 ( chars >> 8 );

        putc(char1, stdout);
        if (char2) {
            putc(char2, stdout);
        }

        ++addr;
    }
    fflush(stdout);
}

template <>
void Trap::exec<Trap::GETC>() {
    Registers::vals[Registers::R0] = static_cast<std::uint16_t> ( getchar() );
    Opcodes::update_cond(Registers::R0);
}