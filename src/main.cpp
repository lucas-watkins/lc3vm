#include "Registers.hpp"
#include "Opcodes.hpp"
#include <bit>
#include <cstdint>
#include <csignal>
#include "Trap.hpp"
#include <fstream>

/* Reads a binary containing the instructions to execute into memory */
bool read_image(const char *const filename) {
    std::ifstream file_stream {filename, std::ios::binary};

    if (file_stream.bad() || !file_stream.is_open()) {
        return false;
    }

    // where to place the program in memory
    std::uint16_t start_addr {};
    file_stream.read(reinterpret_cast<char *>(&start_addr), sizeof(std::uint16_t));

    if (file_stream.bad())
        return false;

    // Because all lc3 programs are in big endian, we must swap to little if we are little endian
    if constexpr (std::endian::native == std::endian::little) {
        start_addr = start_addr << 8 | start_addr >> 8;
    }


    // Amount of memory to read in std::uint16_t(s)
    const auto to_read { static_cast<std::streamsize>(Memory::mem_amt - start_addr) };

    // Read rest of the program
    file_stream.read(
        reinterpret_cast<char *>(&Memory::mem[start_addr]),
        static_cast<std::streamsize>(to_read * sizeof(std::uint16_t)) // convert to bytes
    );

    if (file_stream.bad()) {
        return false;
    }

    // Swap rest of memory to little endian if needed
    if constexpr (std::endian::native == std::endian::little) {
        for ( std::size_t i { start_addr }; i <= start_addr + to_read; ++i ) {
            Memory::mem[i] = Memory::mem[i] << 8 | Memory::mem[i] >> 8;
        }
    }

    file_stream.close();

    return true;
}

void handle_interrupt(const int _) {
    restore_input_buffering();
    std::cout << "\n** Program Terminated **\n";
    std::exit(-2);
}

int main(int argc, const char *const argv[]) {

    if (!read_image("2048")) {
        return -1;
    }

    std::signal(SIGINT, handle_interrupt);
    disable_input_buffering();

    /* The default value in the COND register is 0 */
    Registers::write(Registers::COND, CondFlags::ZERO);

    /* Program counter needs to be in the starting position */
    Registers::write(Registers::PC, Registers::pc_start);

    bool running { true };
    while (running) {
        /*
         * No Registers::read() because the value is copied when using read, and we
         * need to modify it, but doing another read and write after every instruction is painful.
         */
        const std::uint16_t instr { Memory::read(Registers::vals[Registers::PC]++) };

        const std::uint16_t opcode ( instr >> 12 );

        if (opcode == Opcodes::TRAP && (instr & 0xFF) == Trap::HALT) {
            running = false;
            std::cout << "\n** Program Halted **\n" << std::flush;
        } else {
            Opcodes::opcode_funcs[opcode](instr);
        }
    }

    restore_input_buffering();
    return 0;
}