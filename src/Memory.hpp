//
// Created by Lucas Watkins on 8/5/25.
//

#ifndef LC3VM_MEMORY_HPP
#define LC3VM_MEMORY_HPP
#include <array>

#if defined(__unix__) || defined(__linux__) || defined(__APPLE__)
#include <sys/termios.h>
#include <sys/select.h>
#elif defined(_WIN32)
#include <Windows.h>
#include <conio.h>
#endif
namespace Memory {

    /* Memory mapped registers */
    enum {
        KBSR = 0xFE00, /* Keyboard Status Register */
        KBDR = 0xFE02, /* Keyboard Data Register */
    };

    /* Amount of memory that the VM has access to (128 KiB, 65536 locations each 16 bits wide) */
    constexpr std::size_t mem_amt { 1 << 16 };

    /* The memory as an array */
    inline std::array<std::uint16_t, mem_amt> mem {};

    inline bool check_keyboard_available() {
#if defined(__unix__) || defined(__linux__) || defined(__APPLE__)
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(fileno(stdin), &fds);

        static timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 0;

        return select(1, &fds, nullptr, nullptr, &timeout) != 0;
#elif defined(_WIN32)
        return WaitForSingleObject(hStdin, 1000) == WAIT_OBJECT_0 && _kbhit();
#else
        static_assert(false, "Unrecognized OS");
#endif
    }

    inline void write(const std::uint16_t addr, const std::uint16_t val) {
        mem[addr] = val;
    }

    inline std::uint16_t read(const std::uint16_t addr) {
        if (addr == KBSR) {

            if (check_keyboard_available()) {
                write(KBSR, 1 << 15);
                write(KBDR, static_cast<unsigned char>(std::cin.get()));

                std::cin.ignore(std::numeric_limits<std::streamsize>::max());
            } else {
                write(KBSR, 0);
            }

        }

        return mem[addr];
    }

}

#endif //LC3VM_MEMORY_HPP