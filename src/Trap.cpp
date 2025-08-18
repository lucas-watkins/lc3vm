//
// Created by Lucas Watkins on 8/15/25.
//

#include "Trap.hpp"
#include "Memory.hpp"
#include "Registers.hpp"

#if  defined(__unix__) || defined(__APPLE__) || defined(__linux__)
#include <sys/termios.h>
#elif defined(_WIN32)
#include <Windows.h>
#endif

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
        std::cout << static_cast<unsigned char>(Memory::read(addr++));
    }
}

template <>
void Trap::exec<Trap::HALT>() {
    std::cout << "\n** Program Halted **\n";
    std::exit(EXIT_SUCCESS);
}

/* Outputs a single character located in register 0 */
template <>
void Trap::exec<Trap::OUT>() {
    std::cout << static_cast<unsigned char>(Registers::vals[Registers::R0] & 0xFF);
}

template<>
void Trap::exec<Trap::IN>() {
    read_char();
}

template<>
void Trap::exec<Trap::PUTSP>() {
    std::uint16_t addr { Memory::read(Registers::vals[Registers::R0]) };

    while (Memory::read(addr) != 0x0) {
        for ( int shift { 0 }; shift <= 8; shift += 8) {
            if (const unsigned char c = Memory::read(addr) >> shift & 0xFF ) {
                std::cout << c;
            }
        }
        ++addr;
    }
}

template <>
void Trap::exec<Trap::GETC>() {
#if defined(__unix__) || defined(__APPLE__) || defined(__linux__)
    static termios term {};
    tcgetattr(fileno(stdin), &term);

    term.c_lflag &= ~ECHO;
    tcsetattr(fileno(stdin), TCSANOW, &term);

    read_char();

    term.c_lflag &= ECHO;
    tcsetattr(fileno(stdin), TCSANOW, &term);
#elif defined(_WIN32)
    HANDLE stdin_handle { GetStdHandle(STD_INPUT_HANDLE) };
    static DWORD console_mode {};
    GetConsoleMode(stdin_handle, &console_mode);
    SetConsoleMode(stdin_handle, console_mode & ~ENABLE_ECHO_INPUT);

    read_char();

    SetConsoleMode(stdin_handle, console_mode & ENABLE_ECHO_INPUT);
#else
    static_assert(false, "Unrecognized OS");
#endif
}