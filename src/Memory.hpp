//
// Created by Lucas Watkins on 8/5/25.
//

#ifndef LC3VM_MEMORY_HPP
#define LC3VM_MEMORY_HPP
#include <array>

namespace Memory {

    /* Amount of memory that the VM has access to (128 KiB, 65536 locations each 16 bits wide) */
    constexpr std::size_t mem_amt { 1 << 16 };

    /* The memory as an array */
    inline std::array<std::uint16_t, mem_amt> mem {};

}

#endif //LC3VM_MEMORY_HPP