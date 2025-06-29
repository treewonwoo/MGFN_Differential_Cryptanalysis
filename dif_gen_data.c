/*
    dif_gen_data.c – Differential pair generator for 24-round MGFN

    This tool generates plaintext-ciphertext pairs for Differential cryptanalysis.

    * Stage-1: 16-bit mask filtering using ciphertext difference:
          ciphertext_diff = ciphertext ^ modified_ciphertext
          keep if (ciphertext_diff & 0xE8358db1) == 0x1
      Only 1/65536 pairs are kept (coarse 16-bit filter), so TARGET_PAIRS is set to 2^26.
      This means ~2^42 raw pairs are generated to obtain 2^26 kept pairs.

    * Stage-2: Once the correct last-round subkey nibble (16 bits) is recovered,
      you can use the recovered key value to apply an additional 16-bit filtering to the dataset
      (using the dif_decrypt_half_* functions) for the next attack step:
        uint32_t diff = dif_decrypt_half_one_round(c1, c2, xored_key);
        if ((diff & 0xE8358db1) == 0x1) { ... }
      This enables a *second* 16-bit filter. 
*/


#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

//#define DEBUG_KEY_SCHEDULE
#define _CRT_SECURE_NO_WARNINGS
#define _CRT_RAND_S
#define SBOX_SIZE 16
#define NUM_ROUNDS 24
#define TARGET_PAIRS   67108864  // 2^26 kept pairs after coarse filtering
#define PAIR_BATCH_SIZE   67108864

// S-box used only in key schedule (not in the round function)
uint8_t S[SBOX_SIZE] = { 0x7, 0xe, 0xf, 0x0, 0xd, 0xb, 0x8, 0x1, 0x9, 0x3, 0x4, 0xc, 0x2, 0x5, 0xa, 0x6 };

// External lookup tables used in Table_lookup (to be defined elsewhere)
extern const const te1[256] = { 0x40431400, 0x40411408, 0x40431408, 0x40010400, 0x40430408, 0x40031408, 0x40010408, 0x40030400, 0x40030408, 0x40031400, 0x40410400, 0x40410408, 0x40011400, 0x40430400, 0x40011408, 0x40411400, 0x48421400, 0x48401408, 0x48421408, 0x48000400, 0x48420408, 0x48021408, 0x48000408, 0x48020400, 0x48020408, 0x48021400, 0x48400400, 0x48400408, 0x48001400, 0x48420400, 0x48001408, 0x48401400, 0x48431400, 0x48411408, 0x48431408, 0x48010400, 0x48430408, 0x48031408, 0x48010408, 0x48030400, 0x48030408, 0x48031400, 0x48410400, 0x48410408, 0x48011400, 0x48430400, 0x48011408, 0x48411400, 0x00421000, 0x00401008, 0x00421008, 0x00000000, 0x00420008, 0x00021008, 0x00000008, 0x00020000, 0x00020008, 0x00021000, 0x00400000, 0x00400008, 0x00001000, 0x00420000, 0x00001008, 0x00401000, 0x08431400, 0x08411408, 0x08431408, 0x08010400, 0x08430408, 0x08031408, 0x08010408, 0x08030400, 0x08030408, 0x08031400, 0x08410400, 0x08410408, 0x08011400, 0x08430400, 0x08011408, 0x08411400, 0x48431000, 0x48411008, 0x48431008, 0x48010000, 0x48430008, 0x48031008, 0x48010008, 0x48030000, 0x48030008, 0x48031000, 0x48410000, 0x48410008, 0x48011000, 0x48430000, 0x48011008, 0x48411000, 0x08421000, 0x08401008, 0x08421008, 0x08000000, 0x08420008, 0x08021008, 0x08000008, 0x08020000, 0x08020008, 0x08021000, 0x08400000, 0x08400008, 0x08001000, 0x08420000, 0x08001008, 0x08401000, 0x00431000, 0x00411008, 0x00431008, 0x00010000, 0x00430008, 0x00031008, 0x00010008, 0x00030000, 0x00030008, 0x00031000, 0x00410000, 0x00410008, 0x00011000, 0x00430000, 0x00011008, 0x00411000, 0x08431000, 0x08411008, 0x08431008, 0x08010000, 0x08430008, 0x08031008, 0x08010008, 0x08030000, 0x08030008, 0x08031000, 0x08410000, 0x08410008, 0x08011000, 0x08430000, 0x08011008, 0x08411000, 0x40431000, 0x40411008, 0x40431008, 0x40010000, 0x40430008, 0x40031008, 0x40010008, 0x40030000, 0x40030008, 0x40031000, 0x40410000, 0x40410008, 0x40011000, 0x40430000, 0x40011008, 0x40411000, 0x00421400, 0x00401408, 0x00421408, 0x00000400, 0x00420408, 0x00021408, 0x00000408, 0x00020400, 0x00020408, 0x00021400, 0x00400400, 0x00400408, 0x00001400, 0x00420400, 0x00001408, 0x00401400, 0x08421400, 0x08401408, 0x08421408, 0x08000400, 0x08420408, 0x08021408, 0x08000408, 0x08020400, 0x08020408, 0x08021400, 0x08400400, 0x08400408, 0x08001400, 0x08420400, 0x08001408, 0x08401400, 0x40421000, 0x40401008, 0x40421008, 0x40000000, 0x40420008, 0x40021008, 0x40000008, 0x40020000, 0x40020008, 0x40021000, 0x40400000, 0x40400008, 0x40001000, 0x40420000, 0x40001008, 0x40401000, 0x00431400, 0x00411408, 0x00431408, 0x00010400, 0x00430408, 0x00031408, 0x00010408, 0x00030400, 0x00030408, 0x00031400, 0x00410400, 0x00410408, 0x00011400, 0x00430400, 0x00011408, 0x00411400, 0x48421000, 0x48401008, 0x48421008, 0x48000000, 0x48420008, 0x48021008, 0x48000008, 0x48020000, 0x48020008, 0x48021000, 0x48400000, 0x48400008, 0x48001000, 0x48420000, 0x48001008, 0x48401000, 0x40421400, 0x40401408, 0x40421408, 0x40000400, 0x40420408, 0x40021408, 0x40000408, 0x40020400, 0x40020408, 0x40021400, 0x40400400, 0x40400408, 0x40001400, 0x40420400, 0x40001408, 0x40401400 };
extern const const te2[256] = { 0x8100A220, 0x0120A220, 0x8120A220, 0x01002200, 0x8120A200, 0x81202220, 0x01202200, 0x81002200, 0x81202200, 0x81002220, 0x0100A200, 0x0120A200, 0x01002220, 0x8100A200, 0x01202220, 0x0100A220, 0x8100A060, 0x0120A060, 0x8120A060, 0x01002040, 0x8120A040, 0x81202060, 0x01202040, 0x81002040, 0x81202040, 0x81002060, 0x0100A040, 0x0120A040, 0x01002060, 0x8100A040, 0x01202060, 0x0100A060, 0x8100A260, 0x0120A260, 0x8120A260, 0x01002240, 0x8120A240, 0x81202260, 0x01202240, 0x81002240, 0x81202240, 0x81002260, 0x0100A240, 0x0120A240, 0x01002260, 0x8100A240, 0x01202260, 0x0100A260, 0x80008020, 0x00208020, 0x80208020, 0x00000000, 0x80208000, 0x80200020, 0x00200000, 0x80000000, 0x80200000, 0x80000020, 0x00008000, 0x00208000, 0x00000020, 0x80008000, 0x00200020, 0x00008020, 0x81008260, 0x01208260, 0x81208260, 0x01000240, 0x81208240, 0x81200260, 0x01200240, 0x81000240, 0x81200240, 0x81000260, 0x01008240, 0x01208240, 0x01000260, 0x81008240, 0x01200260, 0x01008260, 0x8000A260, 0x0020A260, 0x8020A260, 0x00002240, 0x8020A240, 0x80202260, 0x00202240, 0x80002240, 0x80202240, 0x80002260, 0x0000A240, 0x0020A240, 0x00002260, 0x8000A240, 0x00202260, 0x0000A260, 0x80008060, 0x00208060, 0x80208060, 0x00000040, 0x80208040, 0x80200060, 0x00200040, 0x80000040, 0x80200040, 0x80000060, 0x00008040, 0x00208040, 0x00000060, 0x80008040, 0x00200060, 0x00008060, 0x80008220, 0x00208220, 0x80208220, 0x00000200, 0x80208200, 0x80200220, 0x00200200, 0x80000200, 0x80200200, 0x80000220, 0x00008200, 0x00208200, 0x00000220, 0x80008200, 0x00200220, 0x00008220, 0x80008260, 0x00208260, 0x80208260, 0x00000240, 0x80208240, 0x80200260, 0x00200240, 0x80000240, 0x80200240, 0x80000260, 0x00008240, 0x00208240, 0x00000260, 0x80008240, 0x00200260, 0x00008260, 0x8000A220, 0x0020A220, 0x8020A220, 0x00002200, 0x8020A200, 0x80202220, 0x00202200, 0x80002200, 0x80202200, 0x80002220, 0x0000A200, 0x0020A200, 0x00002220, 0x8000A200, 0x00202220, 0x0000A220, 0x81008020, 0x01208020, 0x81208020, 0x01000000, 0x81208000, 0x81200020, 0x01200000, 0x81000000, 0x81200000, 0x81000020, 0x01008000, 0x01208000, 0x01000020, 0x81008000, 0x01200020, 0x01008020, 0x81008060, 0x01208060, 0x81208060, 0x01000040, 0x81208040, 0x81200060, 0x01200040, 0x81000040, 0x81200040, 0x81000060, 0x01008040, 0x01208040, 0x01000060, 0x81008040, 0x01200060, 0x01008060, 0x8000A020, 0x0020A020, 0x8020A020, 0x00002000, 0x8020A000, 0x80202020, 0x00202000, 0x80002000, 0x80202000, 0x80002020, 0x0000A000, 0x0020A000, 0x00002020, 0x8000A000, 0x00202020, 0x0000A020, 0x81008220, 0x01208220, 0x81208220, 0x01000200, 0x81208200, 0x81200220, 0x01200200, 0x81000200, 0x81200200, 0x81000220, 0x01008200, 0x01208200, 0x01000220, 0x81008200, 0x01200220, 0x01008220, 0x8000A060, 0x0020A060, 0x8020A060, 0x00002040, 0x8020A040, 0x80202060, 0x00202040, 0x80002040, 0x80202040, 0x80002060, 0x0000A040, 0x0020A040, 0x00002060, 0x8000A040, 0x00202060, 0x0000A060, 0x8100A020, 0x0120A020, 0x8120A020, 0x01002000, 0x8120A000, 0x81202020, 0x01202000, 0x81002000, 0x81202000, 0x81002020, 0x0100A000, 0x0120A000, 0x01002020, 0x8100A000, 0x01202020, 0x0100A020 };
extern const const te3[256] = { 0x24104102, 0x34100102, 0x34104102, 0x20100100, 0x30104102, 0x34104100, 0x30100100, 0x20104100, 0x30104100, 0x24104100, 0x20100102, 0x30100102, 0x24100100, 0x20104102, 0x34100100, 0x24100102, 0x24004103, 0x34000103, 0x34004103, 0x20000101, 0x30004103, 0x34004101, 0x30000101, 0x20004101, 0x30004101, 0x24004101, 0x20000103, 0x30000103, 0x24000101, 0x20004103, 0x34000101, 0x24000103, 0x24104103, 0x34100103, 0x34104103, 0x20100101, 0x30104103, 0x34104101, 0x30100101, 0x20104101, 0x30104101, 0x24104101, 0x20100103, 0x30100103, 0x24100101, 0x20104103, 0x34100101, 0x24100103, 0x04004002, 0x14000002, 0x14004002, 0x00000000, 0x10004002, 0x14004000, 0x10000000, 0x00004000, 0x10004000, 0x04004000, 0x00000002, 0x10000002, 0x04000000, 0x00004002, 0x14000000, 0x04000002, 0x24104003, 0x34100003, 0x34104003, 0x20100001, 0x30104003, 0x34104001, 0x30100001, 0x20104001, 0x30104001, 0x24104001, 0x20100003, 0x30100003, 0x24100001, 0x20104003, 0x34100001, 0x24100003, 0x04104103, 0x14100103, 0x14104103, 0x00100101, 0x10104103, 0x14104101, 0x10100101, 0x00104101, 0x10104101, 0x04104101, 0x00100103, 0x10100103, 0x04100101, 0x00104103, 0x14100101, 0x04100103, 0x04004003, 0x14000003, 0x14004003, 0x00000001, 0x10004003, 0x14004001, 0x10000001, 0x00004001, 0x10004001, 0x04004001, 0x00000003, 0x10000003, 0x04000001, 0x00004003, 0x14000001, 0x04000003, 0x04104002, 0x14100002, 0x14104002, 0x00100000, 0x10104002, 0x14104000, 0x10100000, 0x00104000, 0x10104000, 0x04104000, 0x00100002, 0x10100002, 0x04100000, 0x00104002, 0x14100000, 0x04100002, 0x04104003, 0x14100003, 0x14104003, 0x00100001, 0x10104003, 0x14104001, 0x10100001, 0x00104001, 0x10104001, 0x04104001, 0x00100003, 0x10100003, 0x04100001, 0x00104003, 0x14100001, 0x04100003, 0x04104102, 0x14100102, 0x14104102, 0x00100100, 0x10104102, 0x14104100, 0x10100100, 0x00104100, 0x10104100, 0x04104100, 0x00100102, 0x10100102, 0x04100100, 0x00104102, 0x14100100, 0x04100102, 0x24004002, 0x34000002, 0x34004002, 0x20000000, 0x30004002, 0x34004000, 0x30000000, 0x20004000, 0x30004000, 0x24004000, 0x20000002, 0x30000002, 0x24000000, 0x20004002, 0x34000000, 0x24000002, 0x24004003, 0x34000003, 0x34004003, 0x20000001, 0x30004003, 0x34004001, 0x30000001, 0x20004001, 0x30004001, 0x24004001, 0x20000003, 0x30000003, 0x24000001, 0x20004003, 0x34000001, 0x24000003, 0x04004102, 0x14000102, 0x14004102, 0x00000100, 0x10004102, 0x14004100, 0x10000100, 0x00004100, 0x10004100, 0x04004100, 0x00000102, 0x10000102, 0x04000100, 0x00004102, 0x14000100, 0x04000102, 0x24104002, 0x34100002, 0x34104002, 0x20100000, 0x30104002, 0x34104000, 0x30100000, 0x20104000, 0x30104000, 0x24104000, 0x20100002, 0x30100002, 0x24100000, 0x20104002, 0x34100000, 0x24100002, 0x04004103, 0x14000103, 0x14004103, 0x00000101, 0x10004103, 0x14004101, 0x10000101, 0x00004101, 0x10004101, 0x04004101, 0x00000103, 0x10000103, 0x04000101, 0x00004103, 0x14000101, 0x04000103, 0x24004102, 0x34000102, 0x34004102, 0x20000100, 0x30004102, 0x34004100, 0x30000100, 0x20004100, 0x30004100, 0x24004100, 0x20000102, 0x30000102, 0x24000100, 0x20004102, 0x34000100, 0x24000102 };
extern const const te4[256] = { 0x008C0814, 0x008C0884, 0x008C0894, 0x00880004, 0x008C0094, 0x00880894, 0x00880084, 0x00880014, 0x00880094, 0x00880814, 0x008C0004, 0x008C0084, 0x00880804, 0x008C0014, 0x00880884, 0x008C0804, 0x020C0814, 0x020C0884, 0x020C0894, 0x02080004, 0x020C0094, 0x02080894, 0x02080084, 0x02080014, 0x02080094, 0x02080814, 0x020C0004, 0x020C0084, 0x02080804, 0x020C0014, 0x02080884, 0x020C0804, 0x028C0814, 0x028C0884, 0x028C0894, 0x02880004, 0x028C0094, 0x02880894, 0x02880084, 0x02880014, 0x02880094, 0x02880814, 0x028C0004, 0x028C0084, 0x02880804, 0x028C0014, 0x02880884, 0x028C0804, 0x00040810, 0x00040880, 0x00040890, 0x00000000, 0x00040090, 0x00000890, 0x00000080, 0x00000010, 0x00000090, 0x00000810, 0x00040000, 0x00040080, 0x00000800, 0x00040010, 0x00000880, 0x00040800, 0x02840814, 0x02840884, 0x02840894, 0x02800004, 0x02840094, 0x02800894, 0x02800084, 0x02800014, 0x02800094, 0x02800814, 0x02840004, 0x02840084, 0x02800804, 0x02840014, 0x02800884, 0x02840804, 0x028C0810, 0x028C0880, 0x028C0890, 0x02880000, 0x028C0090, 0x02880890, 0x02880080, 0x02880010, 0x02880090, 0x02880810, 0x028C0000, 0x028C0080, 0x02880800, 0x028C0010, 0x02880880, 0x028C0800, 0x02040810, 0x02040880, 0x02040890, 0x02000000, 0x02040090, 0x02000890, 0x02000080, 0x02000010, 0x02000090, 0x02000810, 0x02040000, 0x02040080, 0x02000800, 0x02040010, 0x02000880, 0x02040800, 0x00840810, 0x00840880, 0x00840890, 0x00800000, 0x00840090, 0x00800890, 0x00800080, 0x00800010, 0x00800090, 0x00800810, 0x00840000, 0x00840080, 0x00800800, 0x00840010, 0x00800880, 0x00840800, 0x02840810, 0x02840880, 0x02840890, 0x02800000, 0x02840090, 0x02800890, 0x02800080, 0x02800010, 0x02800090, 0x02800810, 0x02840000, 0x02840080, 0x02800800, 0x02840010, 0x02800880, 0x02840800, 0x008C0810, 0x008C0880, 0x008C0890, 0x00880000, 0x008C0090, 0x00880890, 0x00880080, 0x00880010, 0x00880090, 0x00880810, 0x008C0000, 0x008C0080, 0x00880800, 0x008C0010, 0x00880880, 0x008C0800, 0x00040814, 0x00040884, 0x00040894, 0x00000004, 0x00040094, 0x00000894, 0x00000084, 0x00000014, 0x00000094, 0x00000814, 0x00040004, 0x00040084, 0x00000804, 0x00040014, 0x00000884, 0x00040804, 0x02040814, 0x02040884, 0x02040894, 0x02000004, 0x02040094, 0x02000894, 0x02000084, 0x02000014, 0x02000094, 0x02000814, 0x02040004, 0x02040084, 0x02000804, 0x02040014, 0x02000884, 0x02040804, 0x000C0810, 0x000C0880, 0x000C0890, 0x00080000, 0x000C0090, 0x00080890, 0x00080080, 0x00080010, 0x00080090, 0x00080810, 0x000C0000, 0x000C0080, 0x00080800, 0x000C0010, 0x00080880, 0x000C0800, 0x00840814, 0x00840884, 0x00840894, 0x00800004, 0x00840094, 0x00800894, 0x00800084, 0x00800014, 0x00800094, 0x00800814, 0x00840004, 0x00840084, 0x00800804, 0x00840014, 0x00800884, 0x00840804, 0x020C0810, 0x020C0880, 0x020C0890, 0x02080000, 0x020C0090, 0x02080890, 0x02080080, 0x02080010, 0x02080090, 0x02080810, 0x020C0000, 0x020C0080, 0x02080800, 0x020C0010, 0x02080880, 0x020C0800, 0x000C0814, 0x000C0884, 0x000C0894, 0x00080004, 0x000C0094, 0x00080894, 0x00080084, 0x00080014, 0x00080094, 0x00080814, 0x000C0004, 0x000C0084, 0x00080804, 0x000C0014, 0x00080884, 0x000C0804 };

// Split a 128-bit master key into two 64-bit values (high, low)
void split_master_key(uint8_t* master_key, uint64_t* high, uint64_t* low) {
    *high = ((uint64_t)master_key[0] << 56) | ((uint64_t)master_key[1] << 48) |
        ((uint64_t)master_key[2] << 40) | ((uint64_t)master_key[3] << 32) |
        ((uint64_t)master_key[4] << 24) | ((uint64_t)master_key[5] << 16) |
        ((uint64_t)master_key[6] << 8) | (uint64_t)master_key[7];
    *low = ((uint64_t)master_key[8] << 56) | ((uint64_t)master_key[9] << 48) |
        ((uint64_t)master_key[10] << 40) | ((uint64_t)master_key[11] << 32) |
        ((uint64_t)master_key[12] << 24) | ((uint64_t)master_key[13] << 16) |
        ((uint64_t)master_key[14] << 8) | (uint64_t)master_key[15];
}

// Right rotate (high, low) pair by 61 bits
void rotate_right_61_bits(uint64_t* high, uint64_t* low) {
    uint64_t low_masked = *low & 0x1FFFFFFFFFFFFFFF;
    uint64_t high_masked = *high & 0x1FFFFFFFFFFFFFFF;
    *high = (low_masked << 3) | (*high >> 61);
    *low = (high_masked << 3) | (*low >> 61);
}

// Right rotate (high, low) pair by 67 bits (equivalent to left 61)
void rotate_right_67_bits(uint64_t* high, uint64_t* low) {
    uint64_t low_masked = *low & 0xFFFFFFFFFFFFFFF8;
    uint64_t high_masked = *high & 0xFFFFFFFFFFFFFFF8;
    *high = (low_masked >> 3) | (*high << 61);
    *low = (high_masked >> 3) | (*low << 61);
}

typedef struct {
    uint64_t round_keys[14];  // 14 round keys for debugging
    uint64_t rk[26];          // Whitening and round keys for encryption
} KeySchedule;

// Generate the round key schedule from the master key
void key_schedule(uint8_t* master_key, KeySchedule* key_schedule) {
    uint64_t master_key_high, master_key_low;
    split_master_key(master_key, &master_key_high, &master_key_low);

    int rk_idx = 0;  // Output index for round keys

    for (int round_constant = 0; round_constant < 14; ++round_constant) {
        uint64_t Ki_high = master_key_high;
        uint64_t Ki_low = master_key_low;
        rotate_right_61_bits(&Ki_high, &Ki_low);
        key_schedule->round_keys[round_constant] = Ki_high;

        if (round_constant == 0) { // Pre-whitening 64 bits
            key_schedule->rk[rk_idx++] = key_schedule->round_keys[round_constant];
        }
        else if (round_constant < 13) { // 12 rounds × 32 bits × 2
            key_schedule->rk[rk_idx++] = key_schedule->round_keys[round_constant] >> 32;
            key_schedule->rk[rk_idx++] = key_schedule->round_keys[round_constant] & 0xFFFFFFFFULL;
        }
        else { // Post-whitening 64 bits
            key_schedule->rk[rk_idx++] = key_schedule->round_keys[round_constant];
        }

        rotate_right_67_bits(&master_key_high, &master_key_low);

        uint8_t nib0 = (master_key_high >> 60) & 0xF;
        uint8_t nib1 = (master_key_high >> 56) & 0xF;
        master_key_high &= 0x00FFFFFFFFFFFFFFULL;
        master_key_high |= (uint64_t)S[nib0] << 60;
        master_key_high |= (uint64_t)S[nib1] << 56;

        uint8_t rc4 = (round_constant + 1) & 0xF;   // 4-bit round constant
        uint64_t upper_2bits = (rc4 >> 2) & 0x3;
        uint64_t lower_2bits = rc4 & 0x3;
        master_key_high ^= upper_2bits;                        // bits 1..0
        master_key_low ^= (uint64_t)lower_2bits << 62;        // bits 63..62
    }
#ifdef DEBUG_KEY_SCHEDULE
    puts("=== Update K_i ===");
    for (int i = 0; i < 14; ++i) {
        printf("K_%02d : %016llX\n",
            i,
            (unsigned long long)key_schedule->round_keys[i]);
    }
    puts("\n=== Round Key ===");
    for (int i = 0; i < rk_idx; ++i) {
        if (i == 0)
            printf(" Pre-W: %016llX\n",
                (unsigned long long)key_schedule->rk[i]);
        else if (i == rk_idx - 1)
            printf("Post-W: %016llX\n",
                (unsigned long long)key_schedule->rk[i]);
        else
            printf(" RK[%2d]: %08X\n",
                i,
                (uint32_t)key_schedule->rk[i]);
    }
#endif
}

// Convert a 32-element uint8_t array to a 32-bit integer
uint32_t array_to_int(uint8_t* bit_list) {
    uint32_t result = 0;
    for (int i = 0; i < 32; i++) {
        result |= (bit_list[i] << (31 - i));
    }
    return result;
}

// Table lookup using external S-box tables
uint64_t Table_lookup(uint64_t input) {
    uint8_t b1 = (((input >> 16) & 0b111) << 5) | ((input >> 27) & 0x1F); // bits 13..15, 0..4
    uint8_t b2 = (input >> 19) & 0xFF;  // bits 5..12
    uint8_t b3 = input & 0xFF;          // bits 24..31
    uint8_t b4 = (input >> 8) & 0xFF;   // bits 16..23
    return te1[b1] ^ te2[b2] ^ te3[b3] ^ te4[b4];
}

// Single Feistel round encryption
uint64_t encrypt_single_round(uint64_t P, uint64_t key) {
    uint64_t P_H = ((P >> 32) & 0xFFFFFFFF);  // High 32 bits
    uint64_t P_L = P & 0xFFFFFFFF;            // Low 32 bits
    uint64_t Updated_P_H = Table_lookup(P_H ^ key) ^ P_L;
    uint64_t C = ((uint64_t)Updated_P_H << 32) | P_H;
    return C;
}

// Struct for plaintext-ciphertext pair
typedef struct {
    uint64_t plaintext;   // 64-bit plaintext
    uint64_t ciphertext;  // 64-bit ciphertext
} Pair;

// Full encryption for 24-round cipher
void encrypt(uint64_t plaintext, KeySchedule* key_schedule, uint64_t* ciphertext) {
    uint64_t current_plaintext = plaintext;
    current_plaintext ^= key_schedule->rk[0];         // Initial whitening
    for (int i = 0; i < 24; i++) {
        current_plaintext = encrypt_single_round(current_plaintext, key_schedule->rk[i + 1]);
    }
    current_plaintext ^= key_schedule->rk[25];        // Final whitening
    *ciphertext = current_plaintext;
}

// Random 64-bit unsigned integer generator
void generate_random_data(uint64_t* data) {
    unsigned int randomValue1 = 0, randomValue2 = 0;

    // �� ���� 32��Ʈ ���� ���� �����Ͽ� 64��Ʈ ������ ����
    if (rand_s(&randomValue1) != 0 || rand_s(&randomValue2) != 0) {
        // ���� ���� ���� �� ó��: �⺻�� 0 �Ҵ�
        *data = 0;
    }
    else {
        *data = ((uint64_t)randomValue1 << 32) | (uint64_t)randomValue2;  // 64-bit ����
    }
}

/* Difference computation for key-recovery filtering (after key is recovered) */
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

// Save a pair (plaintext, ciphertext) as a line in a file
void save_to_file(FILE* file, uint64_t plaintext, uint64_t ciphertext) {
    fprintf(file, "%016llX %016llX\n", plaintext, ciphertext);
}

// Load pairs from file (if needed)
void load_from_file(const char* filename, Pair** pairs_out, size_t* pair_count_out) {
    FILE* file = NULL;
    errno_t err = fopen_s(&file, filename, "rb");
    if (err != 0 || file == NULL) {
        printf("Failed to open file: %s\n", filename);
        *pairs_out = NULL;
        *pair_count_out = 0;
        return;
    }
    fseek(file, 0, SEEK_END);
    long filesize = ftell(file);
    fseek(file, 0, SEEK_SET);
    size_t num_pairs = filesize / (2 * sizeof(uint64_t));
    Pair* buffer = (Pair*)malloc(num_pairs * sizeof(Pair));
    if (buffer == NULL) {
        printf("Memory allocation failed.\n");
        fclose(file);
        *pairs_out = NULL;
        *pair_count_out = 0;
        return;
    }
    for (size_t i = 0; i < num_pairs; ++i) {
        fread(&buffer[i].plaintext, sizeof(uint64_t), 1, file);
        fread(&buffer[i].ciphertext, sizeof(uint64_t), 1, file);
    }
    fclose(file);
    *pairs_out = buffer;
    *pair_count_out = num_pairs;
}

/* Main function: Generates TARGET_PAIRS satisfying the coarse 16-bit differential */
int main() {
    uint8_t key[16] = {
        0xB7, 0x45, 0xC5, 0xC6,
        0x10, 0x61, 0x98, 0xF3,
        0xCA, 0x4C, 0xD4, 0x5E,
        0x2B, 0x9F, 0x91, 0x0F
    };

    int count = 0;                 // Number of successful (filtered) pairs
    long long total_attempts = 0;  // Total attempts made

    printf("Generating %d plaintext-ciphertext pairs...\n", TARGET_PAIRS);

    uint32_t xored_key1 = 0x6E523673; // For Exxample use with dif_decrypt_half_* after key recovery

    int max_threads = omp_get_max_threads();
    omp_set_num_threads(max_threads);

    int num_files = (TARGET_PAIRS + PAIR_BATCH_SIZE - 1) / PAIR_BATCH_SIZE;
    FILE** files = (FILE**)malloc(num_files * sizeof(FILE*));
    if (files == NULL) {
        printf("Memory allocation for files failed.\n");
        return -1;
    }

    char filename[256];
    for (int i = 0; i < num_files; i++) {
        snprintf(filename, sizeof(filename), "E:\\wonwoo\\DC_FULL_2\\0628_3output_%d.txt", i);
        errno_t err = fopen_s(&files[i], filename, "wb");
        if (err != 0 || files[i] == NULL) {
            printf("Failed to open output file %d for writing.\n", i);
            for (int j = 0; j < i; j++) fclose(files[j]);
            free(files);
            return -1;
        }
    }

    Pair* pair_buffer = (Pair*)malloc(PAIR_BATCH_SIZE * 2 * sizeof(Pair)); // Buffer for two pairs per hit
    if (pair_buffer == NULL) {
        printf("Failed to allocate memory for pair buffer.\n");
        for (int i = 0; i < num_files; i++) fclose(files[i]);
        free(files);
        return -1;
    }
    int buffer_count = 0;

    double start_time = omp_get_wtime();

    KeySchedule key_schedule_struct;
    key_schedule(key, &key_schedule_struct);

#pragma omp parallel reduction(+:total_attempts)
    {
        uint64_t plaintext, ciphertext;
        uint64_t modified_plaintext, modified_ciphertext;

        while (1) {
            total_attempts++; // Increment global total_attempts
            generate_random_data(&plaintext);
            // Generate a pair with chosen difference (example: difference on 32-bit boundary)
            modified_plaintext = (plaintext ^ 0x1);

            // Encrypt both plaintexts
            encrypt(plaintext, &key_schedule_struct, &ciphertext);
            encrypt(modified_plaintext, &key_schedule_struct, &modified_ciphertext);

             // Modify
            //modified_plaintext = (plaintext ^ 0x1);
            //modified_plaintext = (plaintext ^ 0x5000000);
            modified_plaintext = (plaintext ^ 0x100000000);
            //modified_plaintext = (plaintext ^ 0x500000000000000);


            // Calculate ciphertext difference
            //uint64_t ciphertext_diff = dif_decrypt_half_two_round(ciphertext, modified_ciphertext, xored_key1, xored_key2);
            //uint64_t ciphertext_diff = dif_decrypt_half_one_round(ciphertext, modified_ciphertext, xored_key1);

            // Stage-1: Apply 16-bit filter to ciphertext difference (coarse filtering)
            uint64_t ciphertext_diff = ciphertext ^ modified_ciphertext;
            if ((ciphertext_diff & 0xE8358db1) == 0x1) {
#pragma omp critical
                {
                    pair_buffer[buffer_count].plaintext = plaintext;
                    pair_buffer[buffer_count].ciphertext = ciphertext;
                    buffer_count++;
                    pair_buffer[buffer_count].plaintext = modified_plaintext;
                    pair_buffer[buffer_count].ciphertext = modified_ciphertext;
                    buffer_count++;
                }

                if (buffer_count >= PAIR_BATCH_SIZE * 2) {
#pragma omp critical
                    {
                        int file_index = (count / PAIR_BATCH_SIZE) % num_files;
                        for (int i = 0; i < buffer_count; i++) {
                            save_to_file(files[file_index], pair_buffer[i].plaintext, pair_buffer[i].ciphertext);
                        }
                        buffer_count = 0;
                    }
                }

#pragma omp atomic
                count++;

                if (count % (TARGET_PAIRS / 100) == 0) {
                    double elapsed = omp_get_wtime() - start_time;
                    double progress = (double)count / TARGET_PAIRS * 100.0;
                    double estimated_total_time = (elapsed / count) * TARGET_PAIRS;
                    double remaining_time = estimated_total_time - elapsed;
                    printf("\rProgress: %.2f%% | Count: %d/%d | Total Iterations: %lld | Elapsed: %.2fs | ETA: %.2fs",
                        progress, count, TARGET_PAIRS, total_attempts, elapsed, remaining_time);
                    fflush(stdout);
                }
            }
            if (count >= TARGET_PAIRS) break;
        }
    } // End of parallel region

#pragma omp critical
    {
        for (int i = 0; i < buffer_count; i++) {
            save_to_file(files[(count / PAIR_BATCH_SIZE) % num_files], pair_buffer[i].plaintext, pair_buffer[i].ciphertext);
        }
    }

    for (int i = 0; i < num_files; i++) fclose(files[i]);
    free(pair_buffer);
    free(files);

    printf("\nFinished generating pairs and saving to files.\n");
    printf("Total iterations performed: %lld\n", total_attempts);
    return 0;
}

