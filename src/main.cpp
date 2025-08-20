#include "Registers.hpp"
#include "Opcodes.hpp"
#include <bit>
#include <cstdint>
#include <csignal>
#include "Trap.hpp"
#include <fstream>

/* Reads a binary containing the instructions to execute into memory */
/*bool read_image(const char *const filename) {
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
}*/

void read_image_file(FILE* file)
{
    /* the origin tells us where in memory to place the image */
    uint16_t origin;
    fread(&origin, sizeof(origin), 1, file);
    origin = origin << 8 | origin >> 8;

    /* we know the maximum file size so we only need one fread */
    uint16_t max_read = Memory::mem_amt - origin;
    uint16_t* p = &Memory::mem[0] + origin;
    size_t read = fread(p, sizeof(uint16_t), max_read, file);

    /* swap to little endian */
    while (read-- > 0)
    {
        *p = *p << 8 | *p >> 8;
        ++p;
    }
}
int read_image(const char* image_path)
{
    FILE* file = fopen(image_path, "rb");
    if (!file) { return 0; };
    read_image_file(file);
    fclose(file);
    return 1;
}

termios original_tio;

void disable_input_buffering() {
    tcgetattr(STDIN_FILENO, &original_tio);
    struct termios new_tio = original_tio;
    new_tio.c_lflag &= ~ICANON & ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &new_tio);
}

void restore_input_buffering() {
    tcsetattr(STDIN_FILENO, TCSANOW, &original_tio);
}

void handle_interrupt(const int signal) {
    restore_input_buffering();
    printf("\n** Program Terminated **\n");
    std::exit(-2);
}


uint16_t check_key() {
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
    return select(1, &readfds, NULL, NULL, &timeout) != 0;
}

uint16_t mem_read(uint16_t address)
{
    if (address == Memory::KBSR)
    {
        if (check_key())
        {
            Memory::mem[Memory::KBSR] = (1 << 15);
            Memory::mem[Memory::KBDR] = getchar();
        }
        else
        {
            Memory::mem[Memory::KBSR] = 0;
        }
    }
    return Memory::mem[address];
}


int main(int argc, const char *argv[]) {

    if (!read_image("2048")) {
        return -1;
    }

    std::signal(SIGINT, handle_interrupt);
    disable_input_buffering();

    /* The default value in the COND register is 0 */
    Registers::vals[Registers::COND] = CondFlags::ZERO;

    /* Program counter needs to be in the starting position */
    Registers::vals[Registers::PC] = Registers::pc_start;



    bool running { true };
    while (running) {
        const std::uint16_t instr { mem_read(Registers::vals[Registers::PC]++) };

        const std::uint16_t opcode ( instr >> 12 );

        switch (opcode)
        {
            case Opcodes::ADD:
            {
                /* destination register (DR) */
                uint16_t r0 = (instr >> 9) & 0x7;
                /* first operand (SR1) */
                uint16_t r1 = (instr >> 6) & 0x7;
                /* whether we are in immediate mode */
                uint16_t imm_flag = (instr >> 5) & 0x1;

                if (imm_flag)
                {
                    uint16_t imm5 = Opcodes::sign_extend(instr & 0x1F, 5);
                    Registers::vals[r0] = Registers::vals[r1] + imm5;
                }
                else
                {
                    uint16_t r2 = instr & 0x7;
                    Registers::vals[r0] = Registers::vals[r1] + Registers::vals[r2];
                }

                Opcodes::update_cond(r0);
            }
                break;
            case Opcodes::AND:
            {
                uint16_t r0 = (instr >> 9) & 0x7;
                uint16_t r1 = (instr >> 6) & 0x7;
                uint16_t imm_flag = (instr >> 5) & 0x1;

                if (imm_flag)
                {
                    uint16_t imm5 = Opcodes::sign_extend(instr & 0x1F, 5);
                    Registers::vals[r0] = Registers::vals[r1] & imm5;
                }
                else
                {
                    uint16_t r2 = instr & 0x7;
                    Registers::vals[r0] = Registers::vals[r1] & Registers::vals[r2];
                }
                Opcodes::update_cond(r0);
            }
                break;
            case Opcodes::NOT:
            {
                uint16_t r0 = (instr >> 9) & 0x7;
                uint16_t r1 = (instr >> 6) & 0x7;

                Registers::vals[r0] = ~Registers::vals[r1];
                Opcodes::update_cond(r0);
            }
                break;
            case Opcodes::BR:
            {
                uint16_t pc_offset = Opcodes::sign_extend(instr & 0x1FF, 9);
                uint16_t cond_flag = (instr >> 9) & 0x7;
                if (cond_flag & Registers::vals[Registers::COND])
                {
                    Registers::vals[Registers::PC] += pc_offset;
                }
            }
                break;
            case Opcodes::JMP:
            {
                /* Also handles RET */
                uint16_t r1 = (instr >> 6) & 0x7;
                Registers::vals[Registers::PC] = Registers::vals[r1];
            }
                break;
            case Opcodes::JSR:
            {
                uint16_t long_flag = (instr >> 11) & 1;
                Registers::vals[Registers::R7] = Registers::vals[Registers::PC];
                if (long_flag)
                {
                    uint16_t long_pc_offset = Opcodes::sign_extend(instr & 0x7FF, 11);
                    Registers::vals[Registers::PC] += long_pc_offset;  /* JSR */
                }
                else
                {
                    uint16_t r1 = (instr >> 6) & 0x7;
                    Registers::vals[Registers::PC] = Registers::vals[r1]; /* JSRR */
                }
            }
                break;
            case Opcodes::LD:
            {
                uint16_t r0 = (instr >> 9) & 0x7;
                uint16_t pc_offset = Opcodes::sign_extend(instr & 0x1FF, 9);
                Registers::vals[r0] = mem_read(Registers::vals[Registers::PC] + pc_offset);
                Opcodes::update_cond(r0);
            }
                break;
            case Opcodes::LDI:
            {
                /* destination register (DR) */
                uint16_t r0 = (instr >> 9) & 0x7;
                /* PCoffset 9*/
                uint16_t pc_offset = Opcodes::sign_extend(instr & 0x1FF, 9);
                /* add pc_offset to the current PC, look at that memory location to get the final address */
                Registers::vals[r0] = mem_read(mem_read(Registers::vals[Registers::PC] + pc_offset));
                Opcodes::update_cond(r0);
            }
                break;
            case Opcodes::LDR:
            {
                uint16_t r0 = (instr >> 9) & 0x7;
                uint16_t r1 = (instr >> 6) & 0x7;
                uint16_t offset = Opcodes::sign_extend(instr & 0x3F, 6);
                Registers::vals[r0] = mem_read(Registers::vals[r1] + offset);
                Opcodes::update_cond(r0);
            }
                break;
            case Opcodes::LEA:
            {
                uint16_t r0 = (instr >> 9) & 0x7;
                uint16_t pc_offset = Opcodes::sign_extend(instr & 0x1FF, 9);
                Registers::vals[r0] = Registers::vals[Registers::PC] + pc_offset;
                Opcodes::update_cond(r0);
            }
                break;
            case Opcodes::ST:
            {
                uint16_t r0 = (instr >> 9) & 0x7;
                uint16_t pc_offset = Opcodes::sign_extend(instr & 0x1FF, 9);
                Memory::write(Registers::vals[Registers::PC] + pc_offset, Registers::vals[r0]);
            }
                break;
            case Opcodes::STI:
            {
                uint16_t r0 = (instr >> 9) & 0x7;
                uint16_t pc_offset = Opcodes::sign_extend(instr & 0x1FF, 9);
                Memory::write(mem_read(Registers::vals[Registers::PC] + pc_offset), Registers::vals[r0]);
            }
                break;
            case Opcodes::STR:
            {
                uint16_t r0 = (instr >> 9) & 0x7;
                uint16_t r1 = (instr >> 6) & 0x7;
                uint16_t offset = Opcodes::sign_extend(instr & 0x3F, 6);
                Memory::write(Registers::vals[r1] + offset, Registers::vals[r0]);
            }
                break;
            case Opcodes::TRAP:
                Registers::vals[Registers::R7] = Registers::vals[Registers::PC];

                switch (instr & 0xFF)
                {
                    case Trap::GETC:
                    {
                        /* read a single ASCII char */
                        Registers::vals[Registers::R0] = (uint16_t)getchar();
                        Opcodes::update_cond(Registers::R0);
                    }
                        break;
                    case Trap::OUT:
                    {
                        putc((char)Registers::vals[Registers::R0], stdout);
                        fflush(stdout);
                    }
                        break;
                    case Trap::PUTS:
                    {
                        /* one char per word */
                        uint16_t* c = &Memory::mem[0] + Registers::vals[Registers::R0];
                        while (*c)
                        {
                            putc((char)*c, stdout);
                            ++c;
                        }
                        fflush(stdout);
                    }
                        break;
                    case Trap::IN:
                    {
                        printf("Enter a character: ");
                        char c = getchar();
                        putc(c, stdout);
                        fflush(stdout);
                        Registers::vals[Registers::R0] = (uint16_t)c;
                        Opcodes::update_cond(Registers::R0);
                    }
                        break;
                    case Trap::PUTSP:
                    {
                        /* one char per byte (two bytes per word)
                           here we need to swap back to
                           big endian format */
                        uint16_t* c = &Memory::mem[0] + Registers::vals[Registers::R0];
                        while (*c)
                        {
                            char char1 = (*c) & 0xFF;
                            putc(char1, stdout);
                            char char2 = (*c) >> 8;
                            if (char2) putc(char2, stdout);
                            ++c;
                        }
                        fflush(stdout);
                    }
                        break;
                    case Trap::HALT:
                    {
                        puts("HALT");
                        fflush(stdout);
                        running = 0;
                    }
                        break;
                }
                break;
            case Opcodes::RES:
            case Opcodes::RTI:
            default:
                abort();
                break;
        }
    }

    restore_input_buffering();
    return 0;
}