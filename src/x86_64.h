#pragma once
#include <cstdint>

#define JUMP_TO(addr)                                                        \
    0xFF, 0x25, 0x00, 0x00, 0x00, 0x00, (uint8_t)(addr >> 0),                \
        (uint8_t)(addr >> 8), (uint8_t)(addr >> 16), (uint8_t)(addr >> 24),  \
        (uint8_t)(addr >> 32), (uint8_t)(addr >> 40), (uint8_t)(addr >> 48), \
        (uint8_t)(addr >> 56)

#define CALL(addr)                                                           \
    0xFF, 0x15, 0x02, 0x00, 0x00, 0x00, 0xEB, 0x08, (uint8_t)(addr >> 0),    \
        (uint8_t)(addr >> 8), (uint8_t)(addr >> 16), (uint8_t)(addr >> 24),  \
        (uint8_t)(addr >> 32), (uint8_t)(addr >> 40), (uint8_t)(addr >> 48), \
        (uint8_t)(addr >> 56)

constexpr uint8_t PUSHF_1 = 0x66;
constexpr uint8_t PUSHF_2 = 0x9C;
constexpr uint8_t POPF_1 = 0x66;
constexpr uint8_t POPF_2 = 0x9D;
constexpr uint8_t RETN = 0xC3;
constexpr uint8_t NOP = 0x90;
constexpr uint8_t PushRax = 0x50;
constexpr uint8_t PushRcx = 0x51;
constexpr uint8_t PushRdx = 0x52;
constexpr uint8_t PushRbx = 0x53;
constexpr uint8_t PushRsp = 0x54;
constexpr uint8_t PushRbp = 0x55;
constexpr uint8_t PushRsi = 0x56;
constexpr uint8_t PushRdi = 0x57;
constexpr uint8_t PopRax = 0x58;
constexpr uint8_t PopRcx = 0x59;
constexpr uint8_t PopRdx = 0x5A;
constexpr uint8_t PopRbx = 0x5B;
constexpr uint8_t PopRsp = 0x5C;
constexpr uint8_t PopRbp = 0x5D;
constexpr uint8_t PopRsi = 0x5E;
constexpr uint8_t PopRdi = 0x5F;
constexpr uint8_t PushR8_1 = 0x41;
constexpr uint8_t PushR8_2 = 0x50;
constexpr uint8_t PopR8_1 = 0x41;
constexpr uint8_t PopR8_2 = 0x58;
constexpr uint8_t PushR9_1 = 0x41;
constexpr uint8_t PushR9_2 = 0x51;
constexpr uint8_t PopR9_1 = 0x41;
constexpr uint8_t PopR9_2 = 0x59;
constexpr uint8_t PushR10_1 = 0x41;
constexpr uint8_t PushR10_2 = 0x52;
constexpr uint8_t PopR10_1 = 0x41;
constexpr uint8_t PopR10_2 = 0x5A;
constexpr uint8_t PushR11_1 = 0x41;
constexpr uint8_t PushR11_2 = 0x53;
constexpr uint8_t PopR11_1 = 0x41;
constexpr uint8_t PopR11_2 = 0x5B;
constexpr uint8_t PushR12_1 = 0x41;
constexpr uint8_t PushR12_2 = 0x54;
constexpr uint8_t PopR12_1 = 0x41;
constexpr uint8_t PopR12_2 = 0x5C;
constexpr uint8_t PushR13_1 = 0x41;
constexpr uint8_t PushR13_2 = 0x55;
constexpr uint8_t PopR13_1 = 0x41;
constexpr uint8_t PopR13_2 = 0x5D;
constexpr uint8_t PushR14_1 = 0x41;
constexpr uint8_t PushR14_2 = 0x56;
constexpr uint8_t PopR14_1 = 0x41;
constexpr uint8_t PopR14_2 = 0x5E;
constexpr uint8_t PushR15_1 = 0x41;
constexpr uint8_t PushR15_2 = 0x57;
constexpr uint8_t PopR15_1 = 0x41;
constexpr uint8_t PopR15_2 = 0x5F;
