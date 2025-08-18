#include "Registers.hpp"
#include "Opcodes.hpp"
#include <bit>
#include <cstdint>
#include <csignal>
#include <fstream>

/* Reads a binary containing the instructions to execute into memory */
bool read_image(const char *const filename) {
    std::ifstream file_stream {filename, std::ios::binary};

    if (file_stream.bad() || !file_stream.is_open()) {
        return false;
    }

    /* where to place the program in memory */
    std::uint16_t start_addr {};
    file_stream.read(reinterpret_cast<char *>(&start_addr), sizeof(std::uint16_t));

    if (file_stream.bad())
        return false;

    /* Because all lc3 programs are in big endian, we must swap to little if we are little endian */
    if constexpr (std::endian::native == std::endian::little) {
        start_addr = start_addr << 8 | start_addr >> 8;
    }


    /* Amount of memory to read in std::uint16_t(s) */
    const auto to_read { static_cast<std::streamsize>(Memory::mem_amt - start_addr) };

    /* Read rest of the program */
    file_stream.read(
        reinterpret_cast<char *>(&Memory::mem[start_addr]),
        static_cast<std::streamsize>(to_read * sizeof(std::uint16_t)) /* convert to bytes */
    );

    if (file_stream.bad()) {
        return false;
    }

    /* Swap rest of memory to little endian if needed */
    if constexpr (std::endian::native == std::endian::little) {
        for ( std::size_t i { start_addr }; i <= start_addr + to_read; ++i ) {
            Memory::mem[i] = Memory::mem[i] << 8 | Memory::mem[i] >> 8;
        }
    }

    file_stream.close();

    return true;
}

void set_input_buffering(const bool buffer) {
#if defined(__unix__) || defined(__linux__) || defined(__APPLE__)
    static termios term;
    if (!buffer) {
        tcgetattr(fileno(stdin), &term);
        termios new_term { term };
        new_term.c_lflag &= ~ECHO & ~ICANON;
        tcsetattr(fileno(stdin), TCSANOW, &new_term);
    } else {
        tcsetattr(fileno(stdin), TCSANOW, &term);
    }
#elif defined(_WIN32)
    static HANDLE hStdin { INVALID_HANDLE_VALUE };
    static DWORD fdwMode, fdwOldMode;

    if (!buffer) {
        hStdin = GetStdHandle(STD_INPUT_HANDLE);
        GetConsoleMode(hStdin, &fdwOldMode); /* save old mode */
        fdwMode = fdwOldMode
                ^ ENABLE_ECHO_INPUT  /* no input echo */
                ^ ENABLE_LINE_INPUT; /* return when one or
                                        more characters are available */
        SetConsoleMode(hStdin, fdwMode); /* set new mode */
        FlushConsoleInputBuffer(hStdin); /* clear buffer */
    } else {
        SetConsoleMode(hStdin, fdwOldMode);
    }
#else
    static_assert(false, "Unrecognized OS");
#endif
}

void handle_interrupt(const int signal) {
    set_input_buffering(true);
    std::cout << '\n';
    std::exit(-2);
}

int main(int argc, const char *const argv[]) {

    std::signal(SIGINT, handle_interrupt);
    set_input_buffering(false);

    /* Program counter needs to be in the starting position */
    Registers::vals[Registers::PC] = Registers::pc_start;

    if (read_image("hello")) {
        while (true) {
            const std::uint16_t curr_inst { Memory::read(Registers::vals[Registers::PC]++) };

            const std::uint16_t opcode ( curr_inst >> 12 & 0xF);

            Opcodes::opcode_funcs[opcode](curr_inst);
        }
    } else {
        std::cout << "Failed to read object" << '\n';
    }

    set_input_buffering(true);
    return 0;
}