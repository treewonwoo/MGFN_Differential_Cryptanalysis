#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define LINE_LENGTH 800
#define MAX_I 268435456  // 2^28

uint32_t S[16] = {
    0x7, 0xe, 0xf, 0x0, 0xd, 0xb, 0x8, 0x1, 0x9, 0x3, 0x4, 0xc, 0x2, 0x5, 0xa, 0x6
};

const te1[256] = { 0x40431400, 0x40411408, 0x40431408, 0x40010400, 0x40430408, 0x40031408, 0x40010408, 0x40030400, 0x40030408, 0x40031400, 0x40410400, 0x40410408, 0x40011400, 0x40430400, 0x40011408, 0x40411400, 0x48421400, 0x48401408, 0x48421408, 0x48000400, 0x48420408, 0x48021408, 0x48000408, 0x48020400, 0x48020408, 0x48021400, 0x48400400, 0x48400408, 0x48001400, 0x48420400, 0x48001408, 0x48401400, 0x48431400, 0x48411408, 0x48431408, 0x48010400, 0x48430408, 0x48031408, 0x48010408, 0x48030400, 0x48030408, 0x48031400, 0x48410400, 0x48410408, 0x48011400, 0x48430400, 0x48011408, 0x48411400, 0x00421000, 0x00401008, 0x00421008, 0x00000000, 0x00420008, 0x00021008, 0x00000008, 0x00020000, 0x00020008, 0x00021000, 0x00400000, 0x00400008, 0x00001000, 0x00420000, 0x00001008, 0x00401000, 0x08431400, 0x08411408, 0x08431408, 0x08010400, 0x08430408, 0x08031408, 0x08010408, 0x08030400, 0x08030408, 0x08031400, 0x08410400, 0x08410408, 0x08011400, 0x08430400, 0x08011408, 0x08411400, 0x48431000, 0x48411008, 0x48431008, 0x48010000, 0x48430008, 0x48031008, 0x48010008, 0x48030000, 0x48030008, 0x48031000, 0x48410000, 0x48410008, 0x48011000, 0x48430000, 0x48011008, 0x48411000, 0x08421000, 0x08401008, 0x08421008, 0x08000000, 0x08420008, 0x08021008, 0x08000008, 0x08020000, 0x08020008, 0x08021000, 0x08400000, 0x08400008, 0x08001000, 0x08420000, 0x08001008, 0x08401000, 0x00431000, 0x00411008, 0x00431008, 0x00010000, 0x00430008, 0x00031008, 0x00010008, 0x00030000, 0x00030008, 0x00031000, 0x00410000, 0x00410008, 0x00011000, 0x00430000, 0x00011008, 0x00411000, 0x08431000, 0x08411008, 0x08431008, 0x08010000, 0x08430008, 0x08031008, 0x08010008, 0x08030000, 0x08030008, 0x08031000, 0x08410000, 0x08410008, 0x08011000, 0x08430000, 0x08011008, 0x08411000, 0x40431000, 0x40411008, 0x40431008, 0x40010000, 0x40430008, 0x40031008, 0x40010008, 0x40030000, 0x40030008, 0x40031000, 0x40410000, 0x40410008, 0x40011000, 0x40430000, 0x40011008, 0x40411000, 0x00421400, 0x00401408, 0x00421408, 0x00000400, 0x00420408, 0x00021408, 0x00000408, 0x00020400, 0x00020408, 0x00021400, 0x00400400, 0x00400408, 0x00001400, 0x00420400, 0x00001408, 0x00401400, 0x08421400, 0x08401408, 0x08421408, 0x08000400, 0x08420408, 0x08021408, 0x08000408, 0x08020400, 0x08020408, 0x08021400, 0x08400400, 0x08400408, 0x08001400, 0x08420400, 0x08001408, 0x08401400, 0x40421000, 0x40401008, 0x40421008, 0x40000000, 0x40420008, 0x40021008, 0x40000008, 0x40020000, 0x40020008, 0x40021000, 0x40400000, 0x40400008, 0x40001000, 0x40420000, 0x40001008, 0x40401000, 0x00431400, 0x00411408, 0x00431408, 0x00010400, 0x00430408, 0x00031408, 0x00010408, 0x00030400, 0x00030408, 0x00031400, 0x00410400, 0x00410408, 0x00011400, 0x00430400, 0x00011408, 0x00411400, 0x48421000, 0x48401008, 0x48421008, 0x48000000, 0x48420008, 0x48021008, 0x48000008, 0x48020000, 0x48020008, 0x48021000, 0x48400000, 0x48400008, 0x48001000, 0x48420000, 0x48001008, 0x48401000, 0x40421400, 0x40401408, 0x40421408, 0x40000400, 0x40420408, 0x40021408, 0x40000408, 0x40020400, 0x40020408, 0x40021400, 0x40400400, 0x40400408, 0x40001400, 0x40420400, 0x40001408, 0x40401400 };

const te2[256] = { 0x8100A220, 0x0120A220, 0x8120A220, 0x01002200, 0x8120A200, 0x81202220, 0x01202200, 0x81002200, 0x81202200, 0x81002220, 0x0100A200, 0x0120A200, 0x01002220, 0x8100A200, 0x01202220, 0x0100A220, 0x8100A060, 0x0120A060, 0x8120A060, 0x01002040, 0x8120A040, 0x81202060, 0x01202040, 0x81002040, 0x81202040, 0x81002060, 0x0100A040, 0x0120A040, 0x01002060, 0x8100A040, 0x01202060, 0x0100A060, 0x8100A260, 0x0120A260, 0x8120A260, 0x01002240, 0x8120A240, 0x81202260, 0x01202240, 0x81002240, 0x81202240, 0x81002260, 0x0100A240, 0x0120A240, 0x01002260, 0x8100A240, 0x01202260, 0x0100A260, 0x80008020, 0x00208020, 0x80208020, 0x00000000, 0x80208000, 0x80200020, 0x00200000, 0x80000000, 0x80200000, 0x80000020, 0x00008000, 0x00208000, 0x00000020, 0x80008000, 0x00200020, 0x00008020, 0x81008260, 0x01208260, 0x81208260, 0x01000240, 0x81208240, 0x81200260, 0x01200240, 0x81000240, 0x81200240, 0x81000260, 0x01008240, 0x01208240, 0x01000260, 0x81008240, 0x01200260, 0x01008260, 0x8000A260, 0x0020A260, 0x8020A260, 0x00002240, 0x8020A240, 0x80202260, 0x00202240, 0x80002240, 0x80202240, 0x80002260, 0x0000A240, 0x0020A240, 0x00002260, 0x8000A240, 0x00202260, 0x0000A260, 0x80008060, 0x00208060, 0x80208060, 0x00000040, 0x80208040, 0x80200060, 0x00200040, 0x80000040, 0x80200040, 0x80000060, 0x00008040, 0x00208040, 0x00000060, 0x80008040, 0x00200060, 0x00008060, 0x80008220, 0x00208220, 0x80208220, 0x00000200, 0x80208200, 0x80200220, 0x00200200, 0x80000200, 0x80200200, 0x80000220, 0x00008200, 0x00208200, 0x00000220, 0x80008200, 0x00200220, 0x00008220, 0x80008260, 0x00208260, 0x80208260, 0x00000240, 0x80208240, 0x80200260, 0x00200240, 0x80000240, 0x80200240, 0x80000260, 0x00008240, 0x00208240, 0x00000260, 0x80008240, 0x00200260, 0x00008260, 0x8000A220, 0x0020A220, 0x8020A220, 0x00002200, 0x8020A200, 0x80202220, 0x00202200, 0x80002200, 0x80202200, 0x80002220, 0x0000A200, 0x0020A200, 0x00002220, 0x8000A200, 0x00202220, 0x0000A220, 0x81008020, 0x01208020, 0x81208020, 0x01000000, 0x81208000, 0x81200020, 0x01200000, 0x81000000, 0x81200000, 0x81000020, 0x01008000, 0x01208000, 0x01000020, 0x81008000, 0x01200020, 0x01008020, 0x81008060, 0x01208060, 0x81208060, 0x01000040, 0x81208040, 0x81200060, 0x01200040, 0x81000040, 0x81200040, 0x81000060, 0x01008040, 0x01208040, 0x01000060, 0x81008040, 0x01200060, 0x01008060, 0x8000A020, 0x0020A020, 0x8020A020, 0x00002000, 0x8020A000, 0x80202020, 0x00202000, 0x80002000, 0x80202000, 0x80002020, 0x0000A000, 0x0020A000, 0x00002020, 0x8000A000, 0x00202020, 0x0000A020, 0x81008220, 0x01208220, 0x81208220, 0x01000200, 0x81208200, 0x81200220, 0x01200200, 0x81000200, 0x81200200, 0x81000220, 0x01008200, 0x01208200, 0x01000220, 0x81008200, 0x01200220, 0x01008220, 0x8000A060, 0x0020A060, 0x8020A060, 0x00002040, 0x8020A040, 0x80202060, 0x00202040, 0x80002040, 0x80202040, 0x80002060, 0x0000A040, 0x0020A040, 0x00002060, 0x8000A040, 0x00202060, 0x0000A060, 0x8100A020, 0x0120A020, 0x8120A020, 0x01002000, 0x8120A000, 0x81202020, 0x01202000, 0x81002000, 0x81202000, 0x81002020, 0x0100A000, 0x0120A000, 0x01002020, 0x8100A000, 0x01202020, 0x0100A020 };

const te3[256] = { 0x24104102, 0x34100102, 0x34104102, 0x20100100, 0x30104102, 0x34104100, 0x30100100, 0x20104100, 0x30104100, 0x24104100, 0x20100102, 0x30100102, 0x24100100, 0x20104102, 0x34100100, 0x24100102, 0x24004103, 0x34000103, 0x34004103, 0x20000101, 0x30004103, 0x34004101, 0x30000101, 0x20004101, 0x30004101, 0x24004101, 0x20000103, 0x30000103, 0x24000101, 0x20004103, 0x34000101, 0x24000103, 0x24104103, 0x34100103, 0x34104103, 0x20100101, 0x30104103, 0x34104101, 0x30100101, 0x20104101, 0x30104101, 0x24104101, 0x20100103, 0x30100103, 0x24100101, 0x20104103, 0x34100101, 0x24100103, 0x04004002, 0x14000002, 0x14004002, 0x00000000, 0x10004002, 0x14004000, 0x10000000, 0x00004000, 0x10004000, 0x04004000, 0x00000002, 0x10000002, 0x04000000, 0x00004002, 0x14000000, 0x04000002, 0x24104003, 0x34100003, 0x34104003, 0x20100001, 0x30104003, 0x34104001, 0x30100001, 0x20104001, 0x30104001, 0x24104001, 0x20100003, 0x30100003, 0x24100001, 0x20104003, 0x34100001, 0x24100003, 0x04104103, 0x14100103, 0x14104103, 0x00100101, 0x10104103, 0x14104101, 0x10100101, 0x00104101, 0x10104101, 0x04104101, 0x00100103, 0x10100103, 0x04100101, 0x00104103, 0x14100101, 0x04100103, 0x04004003, 0x14000003, 0x14004003, 0x00000001, 0x10004003, 0x14004001, 0x10000001, 0x00004001, 0x10004001, 0x04004001, 0x00000003, 0x10000003, 0x04000001, 0x00004003, 0x14000001, 0x04000003, 0x04104002, 0x14100002, 0x14104002, 0x00100000, 0x10104002, 0x14104000, 0x10100000, 0x00104000, 0x10104000, 0x04104000, 0x00100002, 0x10100002, 0x04100000, 0x00104002, 0x14100000, 0x04100002, 0x04104003, 0x14100003, 0x14104003, 0x00100001, 0x10104003, 0x14104001, 0x10100001, 0x00104001, 0x10104001, 0x04104001, 0x00100003, 0x10100003, 0x04100001, 0x00104003, 0x14100001, 0x04100003, 0x04104102, 0x14100102, 0x14104102, 0x00100100, 0x10104102, 0x14104100, 0x10100100, 0x00104100, 0x10104100, 0x04104100, 0x00100102, 0x10100102, 0x04100100, 0x00104102, 0x14100100, 0x04100102, 0x24004002, 0x34000002, 0x34004002, 0x20000000, 0x30004002, 0x34004000, 0x30000000, 0x20004000, 0x30004000, 0x24004000, 0x20000002, 0x30000002, 0x24000000, 0x20004002, 0x34000000, 0x24000002, 0x24004003, 0x34000003, 0x34004003, 0x20000001, 0x30004003, 0x34004001, 0x30000001, 0x20004001, 0x30004001, 0x24004001, 0x20000003, 0x30000003, 0x24000001, 0x20004003, 0x34000001, 0x24000003, 0x04004102, 0x14000102, 0x14004102, 0x00000100, 0x10004102, 0x14004100, 0x10000100, 0x00004100, 0x10004100, 0x04004100, 0x00000102, 0x10000102, 0x04000100, 0x00004102, 0x14000100, 0x04000102, 0x24104002, 0x34100002, 0x34104002, 0x20100000, 0x30104002, 0x34104000, 0x30100000, 0x20104000, 0x30104000, 0x24104000, 0x20100002, 0x30100002, 0x24100000, 0x20104002, 0x34100000, 0x24100002, 0x04004103, 0x14000103, 0x14004103, 0x00000101, 0x10004103, 0x14004101, 0x10000101, 0x00004101, 0x10004101, 0x04004101, 0x00000103, 0x10000103, 0x04000101, 0x00004103, 0x14000101, 0x04000103, 0x24004102, 0x34000102, 0x34004102, 0x20000100, 0x30004102, 0x34004100, 0x30000100, 0x20004100, 0x30004100, 0x24004100, 0x20000102, 0x30000102, 0x24000100, 0x20004102, 0x34000100, 0x24000102 };

const te4[256] = { 0x008C0814, 0x008C0884, 0x008C0894, 0x00880004, 0x008C0094, 0x00880894, 0x00880084, 0x00880014, 0x00880094, 0x00880814, 0x008C0004, 0x008C0084, 0x00880804, 0x008C0014, 0x00880884, 0x008C0804, 0x020C0814, 0x020C0884, 0x020C0894, 0x02080004, 0x020C0094, 0x02080894, 0x02080084, 0x02080014, 0x02080094, 0x02080814, 0x020C0004, 0x020C0084, 0x02080804, 0x020C0014, 0x02080884, 0x020C0804, 0x028C0814, 0x028C0884, 0x028C0894, 0x02880004, 0x028C0094, 0x02880894, 0x02880084, 0x02880014, 0x02880094, 0x02880814, 0x028C0004, 0x028C0084, 0x02880804, 0x028C0014, 0x02880884, 0x028C0804, 0x00040810, 0x00040880, 0x00040890, 0x00000000, 0x00040090, 0x00000890, 0x00000080, 0x00000010, 0x00000090, 0x00000810, 0x00040000, 0x00040080, 0x00000800, 0x00040010, 0x00000880, 0x00040800, 0x02840814, 0x02840884, 0x02840894, 0x02800004, 0x02840094, 0x02800894, 0x02800084, 0x02800014, 0x02800094, 0x02800814, 0x02840004, 0x02840084, 0x02800804, 0x02840014, 0x02800884, 0x02840804, 0x028C0810, 0x028C0880, 0x028C0890, 0x02880000, 0x028C0090, 0x02880890, 0x02880080, 0x02880010, 0x02880090, 0x02880810, 0x028C0000, 0x028C0080, 0x02880800, 0x028C0010, 0x02880880, 0x028C0800, 0x02040810, 0x02040880, 0x02040890, 0x02000000, 0x02040090, 0x02000890, 0x02000080, 0x02000010, 0x02000090, 0x02000810, 0x02040000, 0x02040080, 0x02000800, 0x02040010, 0x02000880, 0x02040800, 0x00840810, 0x00840880, 0x00840890, 0x00800000, 0x00840090, 0x00800890, 0x00800080, 0x00800010, 0x00800090, 0x00800810, 0x00840000, 0x00840080, 0x00800800, 0x00840010, 0x00800880, 0x00840800, 0x02840810, 0x02840880, 0x02840890, 0x02800000, 0x02840090, 0x02800890, 0x02800080, 0x02800010, 0x02800090, 0x02800810, 0x02840000, 0x02840080, 0x02800800, 0x02840010, 0x02800880, 0x02840800, 0x008C0810, 0x008C0880, 0x008C0890, 0x00880000, 0x008C0090, 0x00880890, 0x00880080, 0x00880010, 0x00880090, 0x00880810, 0x008C0000, 0x008C0080, 0x00880800, 0x008C0010, 0x00880880, 0x008C0800, 0x00040814, 0x00040884, 0x00040894, 0x00000004, 0x00040094, 0x00000894, 0x00000084, 0x00000014, 0x00000094, 0x00000814, 0x00040004, 0x00040084, 0x00000804, 0x00040014, 0x00000884, 0x00040804, 0x02040814, 0x02040884, 0x02040894, 0x02000004, 0x02040094, 0x02000894, 0x02000084, 0x02000014, 0x02000094, 0x02000814, 0x02040004, 0x02040084, 0x02000804, 0x02040014, 0x02000884, 0x02040804, 0x000C0810, 0x000C0880, 0x000C0890, 0x00080000, 0x000C0090, 0x00080890, 0x00080080, 0x00080010, 0x00080090, 0x00080810, 0x000C0000, 0x000C0080, 0x00080800, 0x000C0010, 0x00080880, 0x000C0800, 0x00840814, 0x00840884, 0x00840894, 0x00800004, 0x00840094, 0x00800894, 0x00800084, 0x00800014, 0x00800094, 0x00800814, 0x00840004, 0x00840084, 0x00800804, 0x00840014, 0x00800884, 0x00840804, 0x020C0810, 0x020C0880, 0x020C0890, 0x02080000, 0x020C0090, 0x02080890, 0x02080080, 0x02080010, 0x02080090, 0x02080810, 0x020C0000, 0x020C0080, 0x02080800, 0x020C0010, 0x02080880, 0x020C0800, 0x000C0814, 0x000C0884, 0x000C0894, 0x00080004, 0x000C0094, 0x00080894, 0x00080084, 0x00080014, 0x00080094, 0x00080814, 0x000C0004, 0x000C0084, 0x00080804, 0x000C0014, 0x00080884, 0x000C0804 };


// XOR-based S-box table lookup using four 8-bit slices of a 32-bit word.
uint64_t Table_lookup(uint64_t input) {
    uint8_t b1 = (((input >> 16) & 0b111) << 5) | ((input >> 27) & 0x1F);  // bits 13,14,15,0,1,2,3,4
    uint8_t b2 = (input >> 19) & 0xFF;  // bits 5–12
    uint8_t b3 = input & 0xFF;          // bits 24–31
    uint8_t b4 = (input >> 8) & 0xFF;   // bits 16–23
    return te1[b1] ^ te2[b2] ^ te3[b3] ^ te4[b4];
}

// Converts hex string to 64-bit unsigned integer.
uint64_t hex_to_uint64(const char* hex) {
    uint64_t result = 0;
    while (*hex) {
        result *= 16;
        if (*hex >= '0' && *hex <= '9') result += *hex - '0';
        else if (*hex >= 'a' && *hex <= 'f') result += *hex - 'a' + 10;
        else if (*hex >= 'A' && *hex <= 'F') result += *hex - 'A' + 10;
        hex++;
    }
    return result;
}

// Extracts 28 bits from a 32-bit word (pattern 1).
uint32_t extract_28_bits(uint32_t C2_R) {
    uint32_t result = ((((C2_R >> 16) & 0x7) << 1 | (C2_R >> 31) & 0x1)) << 24 |
        (((C2_R >> 27) & 0xF) << 20) |
        (((C2_R >> 23) & 0xF) << 16) |
        (((C2_R >> 19) & 0xF) << 12) |
        (((C2_R >> 4) & 0xF) << 8) |
        (((C2_R >> 12) & 0xF) << 4) |
        ((C2_R >> 8) & 0xF);
    return result;
}

// Extracts 28 bits from a 32-bit word (pattern 2: skip nibble 5).
uint32_t extract_28_bits_2(uint32_t C2_R) {
    uint32_t result = ((((C2_R >> 16) & 0x7) << 1 | (C2_R >> 31) & 0x1)) << 24 |
        (((C2_R >> 27) & 0xF) << 20) |
        (((C2_R >> 19) & 0xF) << 16) |
        (((C2_R >> 4) & 0xF) << 12) |
        (C2_R & 0xF) << 8 |
        (((C2_R >> 12) & 0xF) << 4) |
        ((C2_R >> 8) & 0xF);
    return result;
}

// Extracts 28 bits from a 32-bit word (pattern 3: skip nibble X, customize as needed).
uint32_t extract_28_bits_3(uint32_t C2_R) {
    uint32_t result = ((((C2_R >> 16) & 0x7) << 1 | (C2_R >> 31) & 0x1)) << 24 |
        (((C2_R >> 23) & 0xF) << 20) |
        (((C2_R >> 19) & 0xF) << 16) |
        (((C2_R >> 4) & 0xF) << 12) |
        (C2_R & 0xF) << 8 |
        (((C2_R >> 12) & 0xF) << 4) |
        ((C2_R >> 8) & 0xF);
    return result;
}

// Extracts 28 bits from a 32-bit word (pattern 4: skip nibble Y, customize as needed).
uint32_t extract_28_bits_4(uint32_t C2_R) {
    uint32_t result = ((((C2_R >> 16) & 0x7) << 1 | (C2_R >> 31) & 0x1)) << 24 |
        (((C2_R >> 27) & 0xF) << 20) |
        (((C2_R >> 23) & 0xF) << 16) |
        (((C2_R >> 19) & 0xF) << 12) |
        (((C2_R >> 4) & 0xF) << 8) |
        (C2_R & 0xF) << 4 |
        ((C2_R >> 8) & 0xF);
    return result;
}

// Inverse one round Feistel (right half), using Table_lookup and xored_key.
uint32_t decrypt_half_one_round(uint64_t ciphertext, uint32_t xored_key) {
    uint32_t C_H = (ciphertext >> 32) & 0xFFFFFFFF;
    uint32_t C_L = ciphertext & 0xFFFFFFFF;
    uint32_t half_xor_key = C_L ^ xored_key;
    uint32_t update_C_L = Table_lookup(half_xor_key);
    return C_H ^ update_C_L;
}

// Differential inverse over one round: returns the difference of right halves after inverse, using xored_key.
uint32_t dif_decrypt_half_one_round(uint64_t ciphertext1, uint64_t ciphertext2, uint32_t xored_key) {
    uint64_t C_H_1 = (ciphertext1 >> 32) & 0xFFFFFFFF;
    uint64_t C_L_1 = ciphertext1 & 0xFFFFFFFF;
    uint64_t C_H_2 = (ciphertext2 >> 32) & 0xFFFFFFFF;
    uint64_t C_L_2 = ciphertext2 & 0xFFFFFFFF;
    uint64_t half_xor_key1 = C_L_1 ^ xored_key;
    uint64_t half_xor_key2 = C_L_2 ^ xored_key;
    uint64_t update_C_L_1 = Table_lookup(half_xor_key1);
    uint64_t update_C_L_2 = Table_lookup(half_xor_key2);
    return update_C_L_1 ^ update_C_L_2 ^ C_H_1 ^ C_H_2;
}

// Differential inverse over two rounds.
uint32_t dif_decrypt_half_two_round(uint64_t ciphertext1, uint64_t ciphertext2, uint32_t xored_key1, uint32_t xored_key2) {
    uint64_t C_H_1 = (ciphertext1 >> 32) & 0xFFFFFFFF;
    uint64_t C_L_1 = ciphertext1 & 0xFFFFFFFF;
    uint64_t C_H_2 = (ciphertext2 >> 32) & 0xFFFFFFFF;
    uint64_t C_L_2 = ciphertext2 & 0xFFFFFFFF;
    uint64_t half_xor_key1 = C_L_1 ^ xored_key1;
    uint64_t half_xor_key2 = C_L_2 ^ xored_key1;
    uint64_t update1_C_L_1 = Table_lookup(half_xor_key1);
    uint64_t update1_C_L_2 = Table_lookup(half_xor_key2);
    uint64_t update2_C_L_1 = Table_lookup(update1_C_L_1 ^ C_H_1 ^ xored_key2);
    uint64_t update2_C_L_2 = Table_lookup(update1_C_L_2 ^ C_H_2 ^ xored_key2);
    return update2_C_L_1 ^ update2_C_L_2 ^ C_L_1 ^ C_L_2;
}

// One-round inverse for a single ciphertext.
uint32_t decrypt_half_two_round(uint64_t ciphertext1, uint32_t xored_key1, uint32_t xored_key2) {
    uint64_t C_H_1 = (ciphertext1 >> 32) & 0xFFFFFFFF;
    uint64_t C_L_1 = ciphertext1 & 0xFFFFFFFF;
    uint64_t half_xor_key1 = C_L_1 ^ xored_key1;
    uint64_t update1_C_L_1 = Table_lookup(half_xor_key1);
    uint64_t update2_C_L_1 = Table_lookup(update1_C_L_1 ^ C_H_1 ^ xored_key2);
    return update2_C_L_1 ^ C_L_1;
}

// Differential inverse over three rounds.
uint32_t dif_decrypt_half_three_round(uint64_t ciphertext1, uint64_t ciphertext2, uint32_t xored_key1, uint32_t xored_key2, uint32_t xored_key3) {
    uint64_t C_H_1 = (ciphertext1 >> 32) & 0xFFFFFFFF;
    uint64_t C_L_1 = ciphertext1 & 0xFFFFFFFF;
    uint64_t C_H_2 = (ciphertext2 >> 32) & 0xFFFFFFFF;
    uint64_t C_L_2 = ciphertext2 & 0xFFFFFFFF;
    uint64_t update1_C_L_1 = Table_lookup(C_L_1 ^ xored_key1);
    uint64_t update1_C_L_2 = Table_lookup(C_L_2 ^ xored_key1);
    uint64_t update2_C_L_1 = Table_lookup(update1_C_L_1 ^ C_H_1 ^ xored_key2);
    uint64_t update2_C_L_2 = Table_lookup(update1_C_L_2 ^ C_H_2 ^ xored_key2);
    uint64_t update3_C_L_1 = Table_lookup(update2_C_L_1 ^ C_L_1 ^ xored_key3);
    uint64_t update3_C_L_2 = Table_lookup(update2_C_L_2 ^ C_L_2 ^ xored_key3);
    return update3_C_L_1 ^ update3_C_L_2 ^ update1_C_L_1 ^ update1_C_L_2 ^ C_H_1 ^ C_H_2;
}

// Three-round inverse for a single ciphertext.
uint32_t decrypt_half_three_round(uint64_t ciphertext1, uint32_t xored_key1, uint32_t xored_key2, uint32_t xored_key3) {
    uint64_t C_H_1 = (ciphertext1 >> 32) & 0xFFFFFFFF;
    uint64_t C_L_1 = ciphertext1 & 0xFFFFFFFF;
    uint64_t update1_C_L_1 = Table_lookup(C_L_1 ^ xored_key1);
    uint64_t update2_C_L_1 = Table_lookup(update1_C_L_1 ^ C_H_1 ^ xored_key2);
    uint64_t update3_C_L_1 = Table_lookup(update2_C_L_1 ^ C_L_1 ^ xored_key3);
    return update3_C_L_1 ^ update1_C_L_1 ^ C_H_1;
}

// Inserts a nibble at skip_nib (0=LSB, 7=MSB) into a 28-bit base, creating a full 32-bit candidate.
static inline uint32_t insert_nibble(uint32_t base28, int skip_nib, uint8_t nib_val) {
    uint32_t low_mask = (1u << (skip_nib * 4)) - 1u;
    uint32_t low = base28 & low_mask;
    uint32_t high = base28 >> (skip_nib * 4);
    return (high << ((skip_nib + 1) * 4)) | ((uint32_t)nib_val << (skip_nib * 4)) | low;
}


/*
 * 1-round differential cryptanalysis: Finds the intersection of possible 32-bit candidates
 * by analyzing pairs from two files with different missing nibbles.
 * - Each file contains (plaintext, ciphertext) pairs formatted as hex strings.
 * - For each pair, 7 S-box difference constraints are checked.
 * - The function finds all 32-bit candidates that maximize the count for each missing nibble position,
 *   then prints the intersection of both candidate sets.
 */
void analyze_1r_dc(
    const char* file1_path,  // Path to the first pair file (pattern 1, missing nibble 2)
    const char* file2_path   // Path to the second pair file (pattern 2, missing nibble 5)
)
{
    // 1. Allocate and initialize count arrays for both patterns
    uint32_t* count1 = (uint32_t*)calloc(MAX_I, sizeof(uint32_t));
    uint32_t  max1 = 0;
    uint32_t* count2 = (uint32_t*)calloc(MAX_I, sizeof(uint32_t));
    uint32_t  max2 = 0;

    // 2. Process file 1 (pattern 1: nibble 2 missing)
    {
        FILE* file = fopen(file1_path, "r");
        if (!file) { perror("Failed to open file1"); return; }

        char l1[LINE_LENGTH], l2[LINE_LENGTH];
        uint64_t ct1, ct2, diff;
        int prog = 0;

        while (fgets(l1, sizeof(l1), file) && fgets(l2, sizeof(l2), file)) {
            if (++prog % 10 == 0) printf("[Pattern1] %d pairs processed\n", prog);

            l1[strcspn(l1, "\r\n")] = '\0';
            l2[strcspn(l2, "\r\n")] = '\0';

            ct1 = hex_to_uint64(strchr(l1, ' ') + 1);
            ct2 = hex_to_uint64(strchr(l2, ' ') + 1);
            diff = ct1 ^ ct2;

            uint32_t C1_R = ct1 & 0xFFFFFFFF;
            uint32_t C2_R = ct2 & 0xFFFFFFFF;
            uint32_t E1 = extract_28_bits(C1_R);
            uint32_t E2 = extract_28_bits(C2_R);
            uint32_t Ud = diff >> 32;

            uint8_t d[7] = {
                ((Ud >> 27) & 1) << 3 | ((Ud >> 10) & 1) << 2 | ((Ud >> 30) & 1) << 1 | ((Ud >> 16) & 1),
                ((Ud >> 3) & 1) << 3 | ((Ud >> 22) & 1) << 2 | ((Ud >> 12) & 1) << 1 | ((Ud >> 17) & 1),
                ((Ud >> 6) & 1) << 3 | ((Ud >> 24) & 1) << 2 | ((Ud >> 13) & 1) << 1 | ((Ud >> 9) & 1),
                ((Ud >> 21) & 1) << 3 | ((Ud >> 15) & 1) << 2 | ((Ud >> 5) & 1) << 1 | ((Ud >> 31) & 1),
                ((Ud) & 1) << 3 | ((Ud >> 29) & 1) << 2 | ((Ud >> 8) & 1) << 1 | ((Ud >> 20) & 1),
                ((Ud >> 25) & 1) << 3 | ((Ud >> 2) & 1) << 2 | ((Ud >> 19) & 1) << 1 | ((Ud >> 23) & 1),
                ((Ud >> 7) & 1) << 3 | ((Ud >> 18) & 1) << 2 | ((Ud >> 11) & 1) << 1 | ((Ud >> 4) & 1)
            };

            for (uint32_t i = 0; i < MAX_I; ++i) {
                uint32_t a = i ^ E1;
                uint32_t b = i ^ E2;
                int ok = 1;
                for (int j = 0; j < 7 && ok; ++j) {
                    uint8_t as = S[(a >> ((6 - j) * 4)) & 0xF];
                    uint8_t bs = S[(b >> ((6 - j) * 4)) & 0xF];
                    ok &= ((as ^ bs) == d[j]);
                }
                if (ok && ++count1[i] > max1) max1 = count1[i];
            }
        }
        fclose(file);
    }

    // 3. Process file 2 (pattern 2: nibble 5 missing)
    {
        FILE* file = fopen(file2_path, "r");
        if (!file) { perror("Failed to open file2"); return; }

        char l1[LINE_LENGTH], l2[LINE_LENGTH];
        uint64_t ct1, ct2, diff;
        int prog = 0;

        while (fgets(l1, sizeof(l1), file) && fgets(l2, sizeof(l2), file)) {
            if (++prog % 10 == 0) printf("[Pattern2] %d pairs processed\n", prog);

            l1[strcspn(l1, "\r\n")] = '\0';
            l2[strcspn(l2, "\r\n")] = '\0';

            ct1 = hex_to_uint64(strchr(l1, ' ') + 1);
            ct2 = hex_to_uint64(strchr(l2, ' ') + 1);
            diff = ct1 ^ ct2;

            uint32_t C1_R = ct1 & 0xFFFFFFFF;
            uint32_t C2_R = ct2 & 0xFFFFFFFF;
            uint32_t E1 = extract_28_bits_2(C1_R);
            uint32_t E2 = extract_28_bits_2(C2_R);
            uint32_t Ud = diff >> 32;

            uint8_t d1 = ((Ud >> 27) & 1) << 3 | ((Ud >> 10) & 1) << 2 | ((Ud >> 30) & 1) << 1 | ((Ud >> 16) & 1);
            uint8_t d2 = ((Ud >> 3) & 1) << 3 | ((Ud >> 22) & 1) << 2 | ((Ud >> 12) & 1) << 1 | ((Ud >> 17) & 1);
            uint8_t d3 = ((Ud >> 21) & 1) << 3 | ((Ud >> 15) & 1) << 2 | ((Ud >> 5) & 1) << 1 | ((Ud >> 31) & 1);
            uint8_t d4 = ((Ud) & 1) << 3 | ((Ud >> 29) & 1) << 2 | ((Ud >> 8) & 1) << 1 | ((Ud >> 20) & 1);
            uint8_t d5 = ((Ud >> 28) & 1) << 3 | ((Ud >> 1) & 1) << 2 | ((Ud >> 26) & 1) << 1 | ((Ud >> 14) & 1);
            uint8_t d6 = ((Ud >> 25) & 1) << 3 | ((Ud >> 2) & 1) << 2 | ((Ud >> 19) & 1) << 1 | ((Ud >> 23) & 1);
            uint8_t d7 = ((Ud >> 7) & 1) << 3 | ((Ud >> 18) & 1) << 2 | ((Ud >> 11) & 1) << 1 | ((Ud >> 4) & 1);

            for (uint32_t i = 0; i < MAX_I; ++i) {
                uint32_t a = i ^ E1;
                uint32_t b = i ^ E2;

                int ok =
                    ((S[(a >> 24) & 0xF] ^ S[(b >> 24) & 0xF]) == d1) &&
                    ((S[(a >> 20) & 0xF] ^ S[(b >> 20) & 0xF]) == d2) &&
                    ((S[(a >> 16) & 0xF] ^ S[(b >> 16) & 0xF]) == d3) &&
                    ((S[(a >> 12) & 0xF] ^ S[(b >> 12) & 0xF]) == d4) &&
                    ((S[(a >> 8) & 0xF] ^ S[(b >> 8) & 0xF]) == d5) &&
                    ((S[(a >> 4) & 0xF] ^ S[(b >> 4) & 0xF]) == d6) &&
                    ((S[a & 0xF] ^ S[b & 0xF]) == d7);

                if (ok && ++count2[i] > max2) max2 = count2[i];
            }
        }
        fclose(file);
    }

    // 4. Expand both 28-bit candidate sets to 32 bits by inserting missing nibble values
    uint32_t* list1 = NULL; size_t n1 = 0, cap1 = 0;
    for (uint32_t i = 0; i < MAX_I; ++i) if (count1[i] == max1)
        for (uint8_t nib = 0; nib < 16; ++nib) {
            if (n1 == cap1) {
                cap1 = cap1 ? cap1 * 2 : 64;
                list1 = realloc(list1, cap1 * sizeof(uint32_t));
            }
            list1[n1++] = insert_nibble(i, 2, nib);
        }

    uint32_t* list2 = NULL; size_t n2 = 0, cap2 = 0;
    for (uint32_t i = 0; i < MAX_I; ++i) if (count2[i] == max2)
        for (uint8_t nib = 0; nib < 16; ++nib) {
            if (n2 == cap2) {
                cap2 = cap2 ? cap2 * 2 : 64;
                list2 = realloc(list2, cap2 * sizeof(uint32_t));
            }
            list2[n2++] = insert_nibble(i, 5, nib);
        }

    // 5. Output the intersection of the two 32-bit candidate sets
    printf("\n=== Intersection of 32-bit candidates ===\n");
    for (size_t i = 0; i < n1; ++i) {
        for (size_t j = 0; j < n2; ++j) {
            if (list1[i] == list2[j]) {
                printf("0x%08x\n", list1[i]);
                break; // prevent duplicate printing
            }
        }
    }

    // 6. Free all allocated memory
    free(count1); free(count2);
    free(list1);  free(list2);
}


/*
 * 2-round differential cryptanalysis: Finds the intersection of possible 32-bit candidates
 * by analyzing two files with pairs using different missing nibble patterns.
 * - Each file contains (plaintext, ciphertext) pairs formatted as hex strings.
 * - For each pair, 7 S-box difference constraints are checked after 1-round decryption.
 * - The function finds all 32-bit candidates that maximize the count for each missing nibble position,
 *   then prints the intersection of both candidate sets.
 */
void analyze_2r_dc(
    const char* file1_path,  // Path to first pair file (pattern 1, missing nibble 2)
    const char* file2_path,  // Path to second pair file (pattern 2, missing nibble 5)
    uint32_t    xored_key1   // Subkey for 1-round decryption
)
{
    // 1. Allocate and initialize count arrays for both patterns
    uint32_t* cnt1 = calloc(MAX_I, sizeof(uint32_t));
    uint32_t* cnt2 = calloc(MAX_I, sizeof(uint32_t));
    if (!cnt1 || !cnt2) {
        perror("calloc");
        free(cnt1); free(cnt2);
        return;
    }
    uint32_t max1 = 0, max2 = 0;

    // 2. Process file 1 (pattern 1: missing nibble 2)
    FILE* f1 = fopen(file1_path, "r");
    if (!f1) {
        perror("file1");
        free(cnt1); free(cnt2);
        return;
    }
    char lineA[LINE_LENGTH], lineB[LINE_LENGTH];
    uint64_t ct1, ct2, diff;
    while (fgets(lineA, sizeof lineA, f1) &&
        fgets(lineB, sizeof lineB, f1))
    {
        ct1 = hex_to_uint64(strchr(lineA, ' ') + 1);
        ct2 = hex_to_uint64(strchr(lineB, ' ') + 1);
        diff = ct1 ^ ct2;

        // Reverse right half by 1 round
        uint32_t dec1 = decrypt_half_one_round(ct1, xored_key1);
        uint32_t dec2 = decrypt_half_one_round(ct2, xored_key1);

        uint32_t E1 = extract_28_bits(dec1);   // pattern 1 extraction (missing nibble 2)
        uint32_t E2 = extract_28_bits(dec2);

        uint32_t Ud = (uint32_t)diff; // lower 32 bits

        uint8_t d[7] = {
            ((Ud >> 27) & 1) << 3 | ((Ud >> 10) & 1) << 2 | ((Ud >> 30) & 1) << 1 | ((Ud >> 16) & 1),
            ((Ud >> 3) & 1) << 3 | ((Ud >> 22) & 1) << 2 | ((Ud >> 12) & 1) << 1 | ((Ud >> 17) & 1),
            ((Ud >> 6) & 1) << 3 | ((Ud >> 24) & 1) << 2 | ((Ud >> 13) & 1) << 1 | ((Ud >> 9) & 1),
            ((Ud >> 21) & 1) << 3 | ((Ud >> 15) & 1) << 2 | ((Ud >> 5) & 1) << 1 | ((Ud >> 31) & 1),
            (Ud & 1) << 3 | ((Ud >> 29) & 1) << 2 | ((Ud >> 8) & 1) << 1 | ((Ud >> 20) & 1),
            ((Ud >> 25) & 1) << 3 | ((Ud >> 2) & 1) << 2 | ((Ud >> 19) & 1) << 1 | ((Ud >> 23) & 1),
            ((Ud >> 7) & 1) << 3 | ((Ud >> 18) & 1) << 2 | ((Ud >> 11) & 1) << 1 | ((Ud >> 4) & 1)
        };

        for (uint32_t i = 0; i < MAX_I; ++i) {
            uint32_t a = i ^ E1;
            uint32_t b = i ^ E2;
            int ok = 1;
            for (int j = 0; j < 7 && ok; ++j) {
                uint8_t as = S[(a >> ((6 - j) * 4)) & 0xF];
                uint8_t bs = S[(b >> ((6 - j) * 4)) & 0xF];
                ok = ((as ^ bs) == d[j]);
            }
            if (ok && ++cnt1[i] > max1) max1 = cnt1[i];
        }
    }
    fclose(f1);

    // 3. Process file 2 (pattern 2: missing nibble 5)
    FILE* f2 = fopen(file2_path, "r");
    if (!f2) {
        perror("file2");
        free(cnt1); free(cnt2);
        return;
    }
    while (fgets(lineA, sizeof lineA, f2) &&
        fgets(lineB, sizeof lineB, f2))
    {
        ct1 = hex_to_uint64(strchr(lineA, ' ') + 1);
        ct2 = hex_to_uint64(strchr(lineB, ' ') + 1);
        diff = ct1 ^ ct2;

        uint32_t dec1 = decrypt_half_one_round(ct1, xored_key1);
        uint32_t dec2 = decrypt_half_one_round(ct2, xored_key1);

        uint32_t E1 = extract_28_bits_2(dec1); // pattern 2 extraction (missing nibble 5)
        uint32_t E2 = extract_28_bits_2(dec2);

        uint32_t Ud = (uint32_t)diff;

        uint8_t d1 = ((Ud >> 27) & 1) << 3 | ((Ud >> 10) & 1) << 2 | ((Ud >> 30) & 1) << 1 | ((Ud >> 16) & 1);
        uint8_t d2 = ((Ud >> 3) & 1) << 3 | ((Ud >> 22) & 1) << 2 | ((Ud >> 12) & 1) << 1 | ((Ud >> 17) & 1);
        uint8_t d3 = ((Ud >> 21) & 1) << 3 | ((Ud >> 15) & 1) << 2 | ((Ud >> 5) & 1) << 1 | ((Ud >> 31) & 1);
        uint8_t d4 = (Ud & 1) << 3 | ((Ud >> 29) & 1) << 2 | ((Ud >> 8) & 1) << 1 | ((Ud >> 20) & 1);
        uint8_t d5 = ((Ud >> 28) & 1) << 3 | ((Ud >> 1) & 1) << 2 | ((Ud >> 26) & 1) << 1 | ((Ud >> 14) & 1);
        uint8_t d6 = ((Ud >> 25) & 1) << 3 | ((Ud >> 2) & 1) << 2 | ((Ud >> 19) & 1) << 1 | ((Ud >> 23) & 1);
        uint8_t d7 = ((Ud >> 7) & 1) << 3 | ((Ud >> 18) & 1) << 2 | ((Ud >> 11) & 1) << 1 | ((Ud >> 4) & 1);

        for (uint32_t i = 0; i < MAX_I; ++i) {
            uint32_t a = i ^ E1;
            uint32_t b = i ^ E2;

            int ok =
                ((S[(a >> 24) & 0xF] ^ S[(b >> 24) & 0xF]) == d1) &&
                ((S[(a >> 20) & 0xF] ^ S[(b >> 20) & 0xF]) == d2) &&
                ((S[(a >> 16) & 0xF] ^ S[(b >> 16) & 0xF]) == d3) &&
                ((S[(a >> 12) & 0xF] ^ S[(b >> 12) & 0xF]) == d4) &&
                ((S[(a >> 8) & 0xF] ^ S[(b >> 8) & 0xF]) == d5) &&
                ((S[(a >> 4) & 0xF] ^ S[(b >> 4) & 0xF]) == d6) &&
                ((S[a & 0xF] ^ S[b & 0xF]) == d7);

            if (ok && ++cnt2[i] > max2) max2 = cnt2[i];
        }
    }
    fclose(f2);

    // 4. Expand both 28-bit candidate sets to 32 bits by inserting missing nibble values
    uint32_t* L1 = NULL, * L2 = NULL;
    size_t n1 = 0, n2 = 0, cap1 = 0, cap2 = 0;
    for (uint32_t i = 0; i < MAX_I; ++i) if (cnt1[i] == max1)
        for (uint8_t v = 0; v < 16; ++v) {
            if (n1 == cap1) {
                cap1 = cap1 ? cap1 * 2 : 64;
                L1 = realloc(L1, cap1 * sizeof(uint32_t));
            }
            L1[n1++] = insert_nibble(i, 2, v);
        }
    for (uint32_t i = 0; i < MAX_I; ++i) if (cnt2[i] == max2)
        for (uint8_t v = 0; v < 16; ++v) {
            if (n2 == cap2) {
                cap2 = cap2 ? cap2 * 2 : 64;
                L2 = realloc(L2, cap2 * sizeof(uint32_t));
            }
            L2[n2++] = insert_nibble(i, 5, v);
        }

    // 5. Output the intersection of the two 32-bit candidate sets
    puts("\n=== Intersection of 32-bit candidates ===");
    for (size_t i = 0; i < n1; ++i) {
        for (size_t j = 0; j < n2; ++j) {
            if (L1[i] == L2[j]) {
                printf("0x%08x\n", L1[i]);
                break; // prevent duplicates
            }
        }
    }

    // 6. Free all allocated memory
    free(cnt1); free(cnt2);
    free(L1);   free(L2);
}

/*
 * 3-round differential cryptanalysis: Finds the intersection of possible 32-bit candidates
 * by analyzing two files with pairs using different missing nibble patterns,
 * after applying 1-round and 2-round inverse decryption.
 * - Each file contains (plaintext, ciphertext) pairs formatted as hex strings.
 * - For each pair, 7 S-box difference constraints are checked after two rounds of decryption.
 * - The function finds all 32-bit candidates that maximize the count for each missing nibble position,
 *   then prints the intersection of both candidate sets.
 */
void analyze_3r_dc(
    const char* file1_path,  // Path to first pair file (pattern 1, custom extraction)
    const char* file2_path,  // Path to second pair file (pattern 2, custom extraction)
    uint32_t    xored_key1,  // Subkey for 1st round decryption
    uint32_t    xored_key2   // Subkey for 2nd round decryption
)
{
    // 1. Allocate and initialize count arrays for both patterns
    uint32_t* cnt1 = calloc(MAX_I, sizeof(uint32_t));
    uint32_t* cnt2 = calloc(MAX_I, sizeof(uint32_t));
    if (!cnt1 || !cnt2) {
        perror("calloc");
        free(cnt1); free(cnt2);
        return;
    }
    uint32_t max1 = 0, max2 = 0;

    // 2. Process file 1 (pattern 1: custom extraction)
    FILE* f1 = fopen(file1_path, "r");
    if (!f1) {
        perror("file1");
        free(cnt1); free(cnt2);
        return;
    }
    char lineA[LINE_LENGTH], lineB[LINE_LENGTH];
    uint64_t ct1, ct2, diff;
    while (fgets(lineA, sizeof lineA, f1) &&
        fgets(lineB, sizeof lineB, f1))
    {
        ct1 = hex_to_uint64(strchr(lineA, ' ') + 1);
        ct2 = hex_to_uint64(strchr(lineB, ' ') + 1);
        diff = ct1 ^ ct2;

        // 1. Reverse right half by 1 round
        uint32_t dec1 = decrypt_half_one_round(ct1, xored_key1);
        uint32_t dec2 = decrypt_half_one_round(ct2, xored_key1);

        // 2. Reverse right half by the 2nd round
        uint32_t dec3 = decrypt_half_two_round(ct1, xored_key1, xored_key2);
        uint32_t dec4 = decrypt_half_two_round(ct2, xored_key1, xored_key2);

        // 3. Extract 28 bits (custom for pattern-1)
        uint32_t E1 = extract_28_bits(dec3);
        uint32_t E2 = extract_28_bits(dec4);

        uint32_t Ud = dec1 ^ dec2 ^ 0x10000000;

        uint8_t d[7] = {
            ((Ud >> 27) & 1) << 3 | ((Ud >> 10) & 1) << 2 | ((Ud >> 30) & 1) << 1 | ((Ud >> 16) & 1),
            ((Ud >> 3) & 1) << 3 | ((Ud >> 22) & 1) << 2 | ((Ud >> 12) & 1) << 1 | ((Ud >> 17) & 1),
            ((Ud >> 6) & 1) << 3 | ((Ud >> 24) & 1) << 2 | ((Ud >> 13) & 1) << 1 | ((Ud >> 9) & 1),
            ((Ud >> 21) & 1) << 3 | ((Ud >> 15) & 1) << 2 | ((Ud >> 5) & 1) << 1 | ((Ud >> 31) & 1),
            (Ud & 1) << 3 | ((Ud >> 29) & 1) << 2 | ((Ud >> 8) & 1) << 1 | ((Ud >> 20) & 1),
            ((Ud >> 25) & 1) << 3 | ((Ud >> 2) & 1) << 2 | ((Ud >> 19) & 1) << 1 | ((Ud >> 23) & 1),
            ((Ud >> 7) & 1) << 3 | ((Ud >> 18) & 1) << 2 | ((Ud >> 11) & 1) << 1 | ((Ud >> 4) & 1)
        };

        for (uint32_t i = 0; i < MAX_I; ++i) {
            uint32_t a = i ^ E1;
            uint32_t b = i ^ E2;
            int ok = 1;
            for (int j = 0; j < 7 && ok; ++j) {
                uint8_t as = S[(a >> ((6 - j) * 4)) & 0xF];
                uint8_t bs = S[(b >> ((6 - j) * 4)) & 0xF];
                ok = ((as ^ bs) == d[j]);
            }
            if (ok && ++cnt1[i] > max1) max1 = cnt1[i];
        }
    }
    fclose(f1);

    // 3. Process file 2 (pattern 2: custom extraction)
    FILE* f2 = fopen(file2_path, "r");
    if (!f2) {
        perror("file2");
        free(cnt1); free(cnt2);
        return;
    }
    while (fgets(lineA, sizeof lineA, f2) &&
        fgets(lineB, sizeof lineB, f2))
    {
        ct1 = hex_to_uint64(strchr(lineA, ' ') + 1);
        ct2 = hex_to_uint64(strchr(lineB, ' ') + 1);
        diff = ct1 ^ ct2;

        uint32_t dec1 = decrypt_half_one_round(ct1, xored_key1);
        uint32_t dec2 = decrypt_half_one_round(ct2, xored_key1);

        uint32_t dec3 = decrypt_half_two_round(ct1, xored_key1, xored_key2);
        uint32_t dec4 = decrypt_half_two_round(ct2, xored_key1, xored_key2);

        uint32_t E1 = extract_28_bits_2(dec3);
        uint32_t E2 = extract_28_bits_2(dec4);

        uint32_t Ud = dec1 ^ dec2 ^ 0x2000; 

        uint8_t d1 = ((Ud >> 27) & 1) << 3 | ((Ud >> 10) & 1) << 2 | ((Ud >> 30) & 1) << 1 | ((Ud >> 16) & 1);
        uint8_t d2 = ((Ud >> 3) & 1) << 3 | ((Ud >> 22) & 1) << 2 | ((Ud >> 12) & 1) << 1 | ((Ud >> 17) & 1);
        uint8_t d3 = ((Ud >> 21) & 1) << 3 | ((Ud >> 15) & 1) << 2 | ((Ud >> 5) & 1) << 1 | ((Ud >> 31) & 1);
        uint8_t d4 = (Ud & 1) << 3 | ((Ud >> 29) & 1) << 2 | ((Ud >> 8) & 1) << 1 | ((Ud >> 20) & 1);
        uint8_t d5 = ((Ud >> 28) & 1) << 3 | ((Ud >> 1) & 1) << 2 | ((Ud >> 26) & 1) << 1 | ((Ud >> 14) & 1);
        uint8_t d6 = ((Ud >> 25) & 1) << 3 | ((Ud >> 2) & 1) << 2 | ((Ud >> 19) & 1) << 1 | ((Ud >> 23) & 1);
        uint8_t d7 = ((Ud >> 7) & 1) << 3 | ((Ud >> 18) & 1) << 2 | ((Ud >> 11) & 1) << 1 | ((Ud >> 4) & 1);

        for (uint32_t i = 0; i < MAX_I; ++i) {
            uint32_t a = i ^ E1;
            uint32_t b = i ^ E2;

            int ok =
                ((S[(a >> 24) & 0xF] ^ S[(b >> 24) & 0xF]) == d1) &&
                ((S[(a >> 20) & 0xF] ^ S[(b >> 20) & 0xF]) == d2) &&
                ((S[(a >> 16) & 0xF] ^ S[(b >> 16) & 0xF]) == d3) &&
                ((S[(a >> 12) & 0xF] ^ S[(b >> 12) & 0xF]) == d4) &&
                ((S[(a >> 8) & 0xF] ^ S[(b >> 8) & 0xF]) == d5) &&
                ((S[(a >> 4) & 0xF] ^ S[(b >> 4) & 0xF]) == d6) &&
                ((S[a & 0xF] ^ S[b & 0xF]) == d7);

            if (ok && ++cnt2[i] > max2) max2 = cnt2[i];
        }
    }
    fclose(f2);

    // 4. Expand both 28-bit candidate sets to 32 bits by inserting missing nibble values
    uint32_t* L1 = NULL, * L2 = NULL;
    size_t n1 = 0, n2 = 0, cap1 = 0, cap2 = 0;
    for (uint32_t i = 0; i < MAX_I; ++i) if (cnt1[i] == max1)
        for (uint8_t v = 0; v < 16; ++v) {
            if (n1 == cap1) {
                cap1 = cap1 ? cap1 * 2 : 64;
                L1 = realloc(L1, cap1 * sizeof(uint32_t));
            }
            L1[n1++] = insert_nibble(i, 2, v);   // For pattern-1, insert at nibble 2
        }
    for (uint32_t i = 0; i < MAX_I; ++i) if (cnt2[i] == max2)
        for (uint8_t v = 0; v < 16; ++v) {
            if (n2 == cap2) {
                cap2 = cap2 ? cap2 * 2 : 64;
                L2 = realloc(L2, cap2 * sizeof(uint32_t));
            }
            L2[n2++] = insert_nibble(i, 5, v);   // For pattern-2, insert at nibble 5
        }

    // 5. Output the intersection of the two 32-bit candidate sets
    puts("\n=== Intersection of 32-bit candidates ===");
    for (size_t i = 0; i < n1; ++i) {
        for (size_t j = 0; j < n2; ++j) {
            if (L1[i] == L2[j]) {
                printf("0x%08x\n", L1[i]);
                break; // prevent duplicates
            }
        }
    }

    // 6. Free all allocated memory
    free(cnt1); free(cnt2);
    free(L1);   free(L2);
}


int main() {
    //analyze_1r_dc("E:\\all_filtered\\1.txt", "E:\\all_filtered\\2.txt");
    //analyze_2r_dc("E:\\all_filtered\\3.txt", "E:\\all_filtered\\4.txt", 0x6E529673);
    //analyze_2r_dc("E:\\all_filtered\\3.txt", "E:\\all_filtered\\4.txt", 0x6E529673,0x78e4b2b0);
}



