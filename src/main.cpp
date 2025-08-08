#include "Registers.hpp"
#include "Opcodes.hpp"
#include <cassert>

int main() {
    /* Program counter needs to be in the starting position */
    Registers::vals[Registers::PC] = Registers::pc_start;


    return 0;
}