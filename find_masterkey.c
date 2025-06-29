#define _CRT_SECURE_NO_WARNINGS
#include <omp.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>  
#include <time.h>
//#define DEBUG_KEY_SCHEDULE 

/*
	This code recovers the unique 128-bit master key given
	several xored round keys (from differential analysis) and
	known plaintext-ciphertext pair files.

	It performs a brute-force search on constrained bits, uses
	S-box inverses for some bits, and verifies candidates using
	the provided pt-ct pairs.
*/

// S-Box for encryption and decryption
uint8_t S[16] = { 0x7, 0xe, 0xf, 0x0, 0xd, 0xb, 0x8, 0x1, 0x9, 0x3, 0x4, 0xc, 0x2, 0x5, 0xa, 0x6 };

// Four T-tables for fast encryption rounds
const te1[256] = { 0x40431400, 0x40411408, 0x40431408, 0x40010400, 0x40430408, 0x40031408, 0x40010408, 0x40030400, 0x40030408, 0x40031400, 0x40410400, 0x40410408, 0x40011400, 0x40430400, 0x40011408, 0x40411400, 0x48421400, 0x48401408, 0x48421408, 0x48000400, 0x48420408, 0x48021408, 0x48000408, 0x48020400, 0x48020408, 0x48021400, 0x48400400, 0x48400408, 0x48001400, 0x48420400, 0x48001408, 0x48401400, 0x48431400, 0x48411408, 0x48431408, 0x48010400, 0x48430408, 0x48031408, 0x48010408, 0x48030400, 0x48030408, 0x48031400, 0x48410400, 0x48410408, 0x48011400, 0x48430400, 0x48011408, 0x48411400, 0x00421000, 0x00401008, 0x00421008, 0x00000000, 0x00420008, 0x00021008, 0x00000008, 0x00020000, 0x00020008, 0x00021000, 0x00400000, 0x00400008, 0x00001000, 0x00420000, 0x00001008, 0x00401000, 0x08431400, 0x08411408, 0x08431408, 0x08010400, 0x08430408, 0x08031408, 0x08010408, 0x08030400, 0x08030408, 0x08031400, 0x08410400, 0x08410408, 0x08011400, 0x08430400, 0x08011408, 0x08411400, 0x48431000, 0x48411008, 0x48431008, 0x48010000, 0x48430008, 0x48031008, 0x48010008, 0x48030000, 0x48030008, 0x48031000, 0x48410000, 0x48410008, 0x48011000, 0x48430000, 0x48011008, 0x48411000, 0x08421000, 0x08401008, 0x08421008, 0x08000000, 0x08420008, 0x08021008, 0x08000008, 0x08020000, 0x08020008, 0x08021000, 0x08400000, 0x08400008, 0x08001000, 0x08420000, 0x08001008, 0x08401000, 0x00431000, 0x00411008, 0x00431008, 0x00010000, 0x00430008, 0x00031008, 0x00010008, 0x00030000, 0x00030008, 0x00031000, 0x00410000, 0x00410008, 0x00011000, 0x00430000, 0x00011008, 0x00411000, 0x08431000, 0x08411008, 0x08431008, 0x08010000, 0x08430008, 0x08031008, 0x08010008, 0x08030000, 0x08030008, 0x08031000, 0x08410000, 0x08410008, 0x08011000, 0x08430000, 0x08011008, 0x08411000, 0x40431000, 0x40411008, 0x40431008, 0x40010000, 0x40430008, 0x40031008, 0x40010008, 0x40030000, 0x40030008, 0x40031000, 0x40410000, 0x40410008, 0x40011000, 0x40430000, 0x40011008, 0x40411000, 0x00421400, 0x00401408, 0x00421408, 0x00000400, 0x00420408, 0x00021408, 0x00000408, 0x00020400, 0x00020408, 0x00021400, 0x00400400, 0x00400408, 0x00001400, 0x00420400, 0x00001408, 0x00401400, 0x08421400, 0x08401408, 0x08421408, 0x08000400, 0x08420408, 0x08021408, 0x08000408, 0x08020400, 0x08020408, 0x08021400, 0x08400400, 0x08400408, 0x08001400, 0x08420400, 0x08001408, 0x08401400, 0x40421000, 0x40401008, 0x40421008, 0x40000000, 0x40420008, 0x40021008, 0x40000008, 0x40020000, 0x40020008, 0x40021000, 0x40400000, 0x40400008, 0x40001000, 0x40420000, 0x40001008, 0x40401000, 0x00431400, 0x00411408, 0x00431408, 0x00010400, 0x00430408, 0x00031408, 0x00010408, 0x00030400, 0x00030408, 0x00031400, 0x00410400, 0x00410408, 0x00011400, 0x00430400, 0x00011408, 0x00411400, 0x48421000, 0x48401008, 0x48421008, 0x48000000, 0x48420008, 0x48021008, 0x48000008, 0x48020000, 0x48020008, 0x48021000, 0x48400000, 0x48400008, 0x48001000, 0x48420000, 0x48001008, 0x48401000, 0x40421400, 0x40401408, 0x40421408, 0x40000400, 0x40420408, 0x40021408, 0x40000408, 0x40020400, 0x40020408, 0x40021400, 0x40400400, 0x40400408, 0x40001400, 0x40420400, 0x40001408, 0x40401400 };
const te2[256] = { 0x8100A220, 0x0120A220, 0x8120A220, 0x01002200, 0x8120A200, 0x81202220, 0x01202200, 0x81002200, 0x81202200, 0x81002220, 0x0100A200, 0x0120A200, 0x01002220, 0x8100A200, 0x01202220, 0x0100A220, 0x8100A060, 0x0120A060, 0x8120A060, 0x01002040, 0x8120A040, 0x81202060, 0x01202040, 0x81002040, 0x81202040, 0x81002060, 0x0100A040, 0x0120A040, 0x01002060, 0x8100A040, 0x01202060, 0x0100A060, 0x8100A260, 0x0120A260, 0x8120A260, 0x01002240, 0x8120A240, 0x81202260, 0x01202240, 0x81002240, 0x81202240, 0x81002260, 0x0100A240, 0x0120A240, 0x01002260, 0x8100A240, 0x01202260, 0x0100A260, 0x80008020, 0x00208020, 0x80208020, 0x00000000, 0x80208000, 0x80200020, 0x00200000, 0x80000000, 0x80200000, 0x80000020, 0x00008000, 0x00208000, 0x00000020, 0x80008000, 0x00200020, 0x00008020, 0x81008260, 0x01208260, 0x81208260, 0x01000240, 0x81208240, 0x81200260, 0x01200240, 0x81000240, 0x81200240, 0x81000260, 0x01008240, 0x01208240, 0x01000260, 0x81008240, 0x01200260, 0x01008260, 0x8000A260, 0x0020A260, 0x8020A260, 0x00002240, 0x8020A240, 0x80202260, 0x00202240, 0x80002240, 0x80202240, 0x80002260, 0x0000A240, 0x0020A240, 0x00002260, 0x8000A240, 0x00202260, 0x0000A260, 0x80008060, 0x00208060, 0x80208060, 0x00000040, 0x80208040, 0x80200060, 0x00200040, 0x80000040, 0x80200040, 0x80000060, 0x00008040, 0x00208040, 0x00000060, 0x80008040, 0x00200060, 0x00008060, 0x80008220, 0x00208220, 0x80208220, 0x00000200, 0x80208200, 0x80200220, 0x00200200, 0x80000200, 0x80200200, 0x80000220, 0x00008200, 0x00208200, 0x00000220, 0x80008200, 0x00200220, 0x00008220, 0x80008260, 0x00208260, 0x80208260, 0x00000240, 0x80208240, 0x80200260, 0x00200240, 0x80000240, 0x80200240, 0x80000260, 0x00008240, 0x00208240, 0x00000260, 0x80008240, 0x00200260, 0x00008260, 0x8000A220, 0x0020A220, 0x8020A220, 0x00002200, 0x8020A200, 0x80202220, 0x00202200, 0x80002200, 0x80202200, 0x80002220, 0x0000A200, 0x0020A200, 0x00002220, 0x8000A200, 0x00202220, 0x0000A220, 0x81008020, 0x01208020, 0x81208020, 0x01000000, 0x81208000, 0x81200020, 0x01200000, 0x81000000, 0x81200000, 0x81000020, 0x01008000, 0x01208000, 0x01000020, 0x81008000, 0x01200020, 0x01008020, 0x81008060, 0x01208060, 0x81208060, 0x01000040, 0x81208040, 0x81200060, 0x01200040, 0x81000040, 0x81200040, 0x81000060, 0x01008040, 0x01208040, 0x01000060, 0x81008040, 0x01200060, 0x01008060, 0x8000A020, 0x0020A020, 0x8020A020, 0x00002000, 0x8020A000, 0x80202020, 0x00202000, 0x80002000, 0x80202000, 0x80002020, 0x0000A000, 0x0020A000, 0x00002020, 0x8000A000, 0x00202020, 0x0000A020, 0x81008220, 0x01208220, 0x81208220, 0x01000200, 0x81208200, 0x81200220, 0x01200200, 0x81000200, 0x81200200, 0x81000220, 0x01008200, 0x01208200, 0x01000220, 0x81008200, 0x01200220, 0x01008220, 0x8000A060, 0x0020A060, 0x8020A060, 0x00002040, 0x8020A040, 0x80202060, 0x00202040, 0x80002040, 0x80202040, 0x80002060, 0x0000A040, 0x0020A040, 0x00002060, 0x8000A040, 0x00202060, 0x0000A060, 0x8100A020, 0x0120A020, 0x8120A020, 0x01002000, 0x8120A000, 0x81202020, 0x01202000, 0x81002000, 0x81202000, 0x81002020, 0x0100A000, 0x0120A000, 0x01002020, 0x8100A000, 0x01202020, 0x0100A020 };
const te3[256] = { 0x24104102, 0x34100102, 0x34104102, 0x20100100, 0x30104102, 0x34104100, 0x30100100, 0x20104100, 0x30104100, 0x24104100, 0x20100102, 0x30100102, 0x24100100, 0x20104102, 0x34100100, 0x24100102, 0x24004103, 0x34000103, 0x34004103, 0x20000101, 0x30004103, 0x34004101, 0x30000101, 0x20004101, 0x30004101, 0x24004101, 0x20000103, 0x30000103, 0x24000101, 0x20004103, 0x34000101, 0x24000103, 0x24104103, 0x34100103, 0x34104103, 0x20100101, 0x30104103, 0x34104101, 0x30100101, 0x20104101, 0x30104101, 0x24104101, 0x20100103, 0x30100103, 0x24100101, 0x20104103, 0x34100101, 0x24100103, 0x04004002, 0x14000002, 0x14004002, 0x00000000, 0x10004002, 0x14004000, 0x10000000, 0x00004000, 0x10004000, 0x04004000, 0x00000002, 0x10000002, 0x04000000, 0x00004002, 0x14000000, 0x04000002, 0x24104003, 0x34100003, 0x34104003, 0x20100001, 0x30104003, 0x34104001, 0x30100001, 0x20104001, 0x30104001, 0x24104001, 0x20100003, 0x30100003, 0x24100001, 0x20104003, 0x34100001, 0x24100003, 0x04104103, 0x14100103, 0x14104103, 0x00100101, 0x10104103, 0x14104101, 0x10100101, 0x00104101, 0x10104101, 0x04104101, 0x00100103, 0x10100103, 0x04100101, 0x00104103, 0x14100101, 0x04100103, 0x04004003, 0x14000003, 0x14004003, 0x00000001, 0x10004003, 0x14004001, 0x10000001, 0x00004001, 0x10004001, 0x04004001, 0x00000003, 0x10000003, 0x04000001, 0x00004003, 0x14000001, 0x04000003, 0x04104002, 0x14100002, 0x14104002, 0x00100000, 0x10104002, 0x14104000, 0x10100000, 0x00104000, 0x10104000, 0x04104000, 0x00100002, 0x10100002, 0x04100000, 0x00104002, 0x14100000, 0x04100002, 0x04104003, 0x14100003, 0x14104003, 0x00100001, 0x10104003, 0x14104001, 0x10100001, 0x00104001, 0x10104001, 0x04104001, 0x00100003, 0x10100003, 0x04100001, 0x00104003, 0x14100001, 0x04100003, 0x04104102, 0x14100102, 0x14104102, 0x00100100, 0x10104102, 0x14104100, 0x10100100, 0x00104100, 0x10104100, 0x04104100, 0x00100102, 0x10100102, 0x04100100, 0x00104102, 0x14100100, 0x04100102, 0x24004002, 0x34000002, 0x34004002, 0x20000000, 0x30004002, 0x34004000, 0x30000000, 0x20004000, 0x30004000, 0x24004000, 0x20000002, 0x30000002, 0x24000000, 0x20004002, 0x34000000, 0x24000002, 0x24004003, 0x34000003, 0x34004003, 0x20000001, 0x30004003, 0x34004001, 0x30000001, 0x20004001, 0x30004001, 0x24004001, 0x20000003, 0x30000003, 0x24000001, 0x20004003, 0x34000001, 0x24000003, 0x04004102, 0x14000102, 0x14004102, 0x00000100, 0x10004102, 0x14004100, 0x10000100, 0x00004100, 0x10004100, 0x04004100, 0x00000102, 0x10000102, 0x04000100, 0x00004102, 0x14000100, 0x04000102, 0x24104002, 0x34100002, 0x34104002, 0x20100000, 0x30104002, 0x34104000, 0x30100000, 0x20104000, 0x30104000, 0x24104000, 0x20100002, 0x30100002, 0x24100000, 0x20104002, 0x34100000, 0x24100002, 0x04004103, 0x14000103, 0x14004103, 0x00000101, 0x10004103, 0x14004101, 0x10000101, 0x00004101, 0x10004101, 0x04004101, 0x00000103, 0x10000103, 0x04000101, 0x00004103, 0x14000101, 0x04000103, 0x24004102, 0x34000102, 0x34004102, 0x20000100, 0x30004102, 0x34004100, 0x30000100, 0x20004100, 0x30004100, 0x24004100, 0x20000102, 0x30000102, 0x24000100, 0x20004102, 0x34000100, 0x24000102 };
const te4[256] = { 0x008C0814, 0x008C0884, 0x008C0894, 0x00880004, 0x008C0094, 0x00880894, 0x00880084, 0x00880014, 0x00880094, 0x00880814, 0x008C0004, 0x008C0084, 0x00880804, 0x008C0014, 0x00880884, 0x008C0804, 0x020C0814, 0x020C0884, 0x020C0894, 0x02080004, 0x020C0094, 0x02080894, 0x02080084, 0x02080014, 0x02080094, 0x02080814, 0x020C0004, 0x020C0084, 0x02080804, 0x020C0014, 0x02080884, 0x020C0804, 0x028C0814, 0x028C0884, 0x028C0894, 0x02880004, 0x028C0094, 0x02880894, 0x02880084, 0x02880014, 0x02880094, 0x02880814, 0x028C0004, 0x028C0084, 0x02880804, 0x028C0014, 0x02880884, 0x028C0804, 0x00040810, 0x00040880, 0x00040890, 0x00000000, 0x00040090, 0x00000890, 0x00000080, 0x00000010, 0x00000090, 0x00000810, 0x00040000, 0x00040080, 0x00000800, 0x00040010, 0x00000880, 0x00040800, 0x02840814, 0x02840884, 0x02840894, 0x02800004, 0x02840094, 0x02800894, 0x02800084, 0x02800014, 0x02800094, 0x02800814, 0x02840004, 0x02840084, 0x02800804, 0x02840014, 0x02800884, 0x02840804, 0x028C0810, 0x028C0880, 0x028C0890, 0x02880000, 0x028C0090, 0x02880890, 0x02880080, 0x02880010, 0x02880090, 0x02880810, 0x028C0000, 0x028C0080, 0x02880800, 0x028C0010, 0x02880880, 0x028C0800, 0x02040810, 0x02040880, 0x02040890, 0x02000000, 0x02040090, 0x02000890, 0x02000080, 0x02000010, 0x02000090, 0x02000810, 0x02040000, 0x02040080, 0x02000800, 0x02040010, 0x02000880, 0x02040800, 0x00840810, 0x00840880, 0x00840890, 0x00800000, 0x00840090, 0x00800890, 0x00800080, 0x00800010, 0x00800090, 0x00800810, 0x00840000, 0x00840080, 0x00800800, 0x00840010, 0x00800880, 0x00840800, 0x02840810, 0x02840880, 0x02840890, 0x02800000, 0x02840090, 0x02800890, 0x02800080, 0x02800010, 0x02800090, 0x02800810, 0x02840000, 0x02840080, 0x02800800, 0x02840010, 0x02800880, 0x02840800, 0x008C0810, 0x008C0880, 0x008C0890, 0x00880000, 0x008C0090, 0x00880890, 0x00880080, 0x00880010, 0x00880090, 0x00880810, 0x008C0000, 0x008C0080, 0x00880800, 0x008C0010, 0x00880880, 0x008C0800, 0x00040814, 0x00040884, 0x00040894, 0x00000004, 0x00040094, 0x00000894, 0x00000084, 0x00000014, 0x00000094, 0x00000814, 0x00040004, 0x00040084, 0x00000804, 0x00040014, 0x00000884, 0x00040804, 0x02040814, 0x02040884, 0x02040894, 0x02000004, 0x02040094, 0x02000894, 0x02000084, 0x02000014, 0x02000094, 0x02000814, 0x02040004, 0x02040084, 0x02000804, 0x02040014, 0x02000884, 0x02040804, 0x000C0810, 0x000C0880, 0x000C0890, 0x00080000, 0x000C0090, 0x00080890, 0x00080080, 0x00080010, 0x00080090, 0x00080810, 0x000C0000, 0x000C0080, 0x00080800, 0x000C0010, 0x00080880, 0x000C0800, 0x00840814, 0x00840884, 0x00840894, 0x00800004, 0x00840094, 0x00800894, 0x00800084, 0x00800014, 0x00800094, 0x00800814, 0x00840004, 0x00840084, 0x00800804, 0x00840014, 0x00800884, 0x00840804, 0x020C0810, 0x020C0880, 0x020C0890, 0x02080000, 0x020C0090, 0x02080890, 0x02080080, 0x02080010, 0x02080090, 0x02080810, 0x020C0000, 0x020C0080, 0x02080800, 0x020C0010, 0x02080880, 0x020C0800, 0x000C0814, 0x000C0884, 0x000C0894, 0x00080004, 0x000C0094, 0x00080894, 0x00080084, 0x00080014, 0x00080094, 0x00080814, 0x000C0004, 0x000C0084, 0x00080804, 0x000C0014, 0x00080884, 0x000C0804 };


/* Split the 128-bit master key into two 64-bit words */
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

/* Right rotate the 128-bit key by 61 bits (using 64+64-bit split) */
void rotate_right_61_bits(uint64_t* high, uint64_t* low) {
	uint64_t low_masked = *low & 0x1FFFFFFFFFFFFFFF;
	uint64_t high_masked = *high & 0x1FFFFFFFFFFFFFFF;

	*high = (low_masked << 3) | (*high >> 61);
	*low = (high_masked << 3) | (*low >> 61);
}
/* Right rotate by 67 bits (same split) */
void rotate_right_67_bits(uint64_t* high, uint64_t* low)
{
	uint64_t low_masked = *low & 0xFFFFFFFFFFFFFFF8;
	uint64_t high_masked = *high & 0xFFFFFFFFFFFFFFF8;

	*high = (low_masked >> 3) | (*high << 61);
	*low = (high_masked >> 3) | (*low << 61);
}

/* Key schedule structure for round keys */
typedef struct {
	uint64_t round_keys[14];  // 14 round keys
	uint64_t rk[26];          // Extracted subkeys for rounds
} KeySchedule;

/* MGFN-style key schedule: fill KeySchedule from master_key */
void key_schedule(
	uint8_t* master_key,
	KeySchedule* key_schedule
)
{
	uint64_t master_key_high, master_key_low;
	split_master_key(master_key, &master_key_high, &master_key_low);

	int rk_idx = 0;

	for (int round_constant = 0; round_constant < 14; ++round_constant) {

		// (1) K_i = K rotated right by 61 bits
		uint64_t Ki_high = master_key_high;
		uint64_t Ki_low = master_key_low;
		rotate_right_61_bits(&Ki_high, &Ki_low);

		key_schedule->round_keys[round_constant] = Ki_high;

		// (2) Extract subkeys
		if (round_constant == 0) {
			key_schedule->rk[rk_idx++] = key_schedule->round_keys[round_constant];
		}
		else if (round_constant < 13) {
			key_schedule->rk[rk_idx++] = key_schedule->round_keys[round_constant] >> 32;
			key_schedule->rk[rk_idx++] = key_schedule->round_keys[round_constant] & 0xFFFFFFFFULL;
		}
		else {
			key_schedule->rk[rk_idx++] = key_schedule->round_keys[round_constant];
		}

		// (3) K = K rotated left by 61 bits (same as right by 67)
		rotate_right_67_bits(&master_key_high, &master_key_low);

		// (4) S-box substitution on nibbles
		uint8_t nib0 = (master_key_high >> 60) & 0xF;
		uint8_t nib1 = (master_key_high >> 56) & 0xF;
		master_key_high &= 0x00FFFFFFFFFFFFFFULL;
		master_key_high |= (uint64_t)S[nib0] << 60;
		master_key_high |= (uint64_t)S[nib1] << 56;

		// (5) XOR round constant into high and low parts
		uint8_t  rc4 = (round_constant + 1) & 0xF;   // 4-bit
		uint64_t upper_2bits = (rc4 >> 2) & 0x3;
		uint64_t lower_2bits = rc4 & 0x3;

		master_key_high ^= upper_2bits;
		master_key_low ^= (uint64_t)lower_2bits << 62;
	}

#ifdef DEBUG_KEY_SCHEDULE
	puts("=== Update K_i ===");
	for (int i = 0; i < 14; ++i) {
		printf("K_%02d : %016llX\n", i, (unsigned long long)key_schedule->round_keys[i]);
	}
	puts("\n=== Round Key ===");
	for (int i = 0; i < rk_idx; ++i) {
		if (i == 0)
			printf(" Pre-W: %016llX\n", (unsigned long long)key_schedule->rk[i]);
		else if (i == rk_idx - 1)
			printf("Post-W: %016llX\n", (unsigned long long)key_schedule->rk[i]);
		else
			printf(" RK[%2d]: %08X\n", i, (uint32_t)key_schedule->rk[i]);
	}
#endif
}

/* Converts a 32-byte array to an int */
uint32_t array_to_int(uint8_t* bit_list) {
	uint32_t result = 0;
	for (int i = 0; i < 32; i++) {
		result |= (bit_list[i] << (31 - i));
	}
	return result;
}

/* Fast lookup using T-tables */
uint64_t Table_lookup(uint64_t input) {
	uint8_t b1 = (((input >> 16) & 0b111) << 5) | ((input >> 27) & 0x1F);
	uint8_t b2 = (input >> 19) & 0xFF;
	uint8_t b3 = input & 0xFF;
	uint8_t b4 = (input >> 8) & 0xFF;
	return te1[b1] ^ te2[b2] ^ te3[b3] ^ te4[b4];
}

/* Encrypt one round (Feistel structure, SWAP) */
uint64_t encrypt_single_round(uint64_t P, uint64_t key) {
	uint64_t P_H = ((P >> 32) & 0xFFFFFFFF);  // Left half
	uint64_t P_L = P & 0xFFFFFFFF;            // Right half
	uint64_t Updated_P_H = Table_lookup(P_H ^ key) ^ P_L;
	return ((uint64_t)Updated_P_H << 32) | P_H;
}

/* Structure for a pt-ct pair */
typedef struct {
	uint64_t plaintext;
	uint64_t ciphertext;
} Pair;

/* Encrypt full rounds for a given plaintext and round key schedule */
void encrypt(uint64_t plaintext, KeySchedule* key_schedule, uint64_t* ciphertext) {
	uint64_t current_plaintext = plaintext;
	current_plaintext ^= key_schedule->rk[0];

	for (int i = 0; i < 24; i++) {
		current_plaintext = encrypt_single_round(current_plaintext, key_schedule->rk[i + 1]);
	}
	current_plaintext ^= key_schedule->rk[25];
	*ciphertext = current_plaintext;
}

/* 4-bit S-box and inverse */
uint8_t substitute_4bit(uint8_t input) {
	const uint8_t S_box[16] = { 0x7, 0xE, 0xF, 0x0, 0xD, 0xB, 0x8, 0x1, 0x9, 0x3, 0x4, 0xC, 0x2, 0x5, 0xA, 0x6 };
	return S_box[input & 0xF];
}
uint8_t inverse_substitute_4bit(uint8_t input) {
	const uint8_t inverse_sbox[16] = { 0x3, 0x7, 0xC, 0x9, 0xA, 0xD, 0xF, 0x0, 0x6, 0x8, 0xE, 0x5, 0xB, 0x4, 0x1, 0x2 };
	return inverse_sbox[input & 0xF];
}

/* 128-bit rotate left (split into two 64-bits) by 61 bits */
void rotate_left_61_bits(uint64_t* high, uint64_t* low) {
	uint64_t h = *high, l = *low;
	*high = (h << 61) | (l >> 3);
	*low = (l << 61) | (h >> 3);
}
/* Rotate left by 67 bits (64+3) */
void rotate_left_67_bits(uint64_t* high, uint64_t* low) {
	uint64_t h = *high, l = *low;
	*high = (l << 3) | (h >> 61);
	*low = (h << 3) | (l >> 61);
}

/*
 * Checks whether the given master key correctly encrypts the given pt-ct pairs.
 * The first pair is already in memory; further pairs are read from the given file.
 * Returns 1 if valid, 0 otherwise.
 */
static int check_key_candidate(const Pair* first_pair, KeySchedule* ks, const char* pair_file) {
	uint64_t ct;

	// (1) Fast check: encrypt first pair and compare
	encrypt(first_pair->plaintext, ks, &ct);
	if (ct != first_pair->ciphertext) return 0;

	// (2) If passed, check next pair from file (single file pointer for thread safety)
	static FILE* fp = NULL;
	Pair extra;

#pragma omp critical(extra_file_open)
	{
		if (fp == NULL) {
			fp = fopen(pair_file, "r");
			if (!fp) { perror("extra pair file"); exit(1); }
			// Skip the first line (already checked)
			uint64_t dummy1, dummy2;
			fscanf(fp, "%llx %llx", &dummy1, &dummy2);
		}
	}
	// Read the next pair (thread safe)
	int ok_read = 0;
#pragma omp critical(extra_file_read)
	{
		ok_read = (fscanf(fp, "%llx %llx", &extra.plaintext, &extra.ciphertext) == 2);
	}
	if (!ok_read)  return 1;      // No more pairs, first pair is sufficient

	encrypt(extra.plaintext, ks, &ct);
	return (ct == extra.ciphertext);
}

/*
 * Given a partially-recovered master key (mkh, mkl), recover the original master key.
 * This undoes the final round constants, S-box substitutions, and rotations.
 */
void recover_master_key(uint64_t mkh, uint64_t mkl, uint64_t* out_high, uint64_t* out_low) {
	rotate_left_61_bits(&mkh, &mkl);
	for (int round = 13; round > 0; round--) {
		uint8_t rc = round;
		uint8_t upper2 = (rc >> 2) & 0x3;
		uint8_t lower2 = rc & 0x3;
		uint8_t curr_upper2 = mkh & 0x3;
		uint8_t curr_lower2 = (mkl >> 62) & 0x3;
		mkh = (mkh & ~0x3ULL) | (curr_upper2 ^ upper2);
		mkl = (mkl & ~(0x3ULL << 62)) | ((uint64_t)(curr_lower2 ^ lower2) << 62);

		uint8_t substituted = (mkh >> 56) & 0xFF;
		uint8_t nib0 = (substituted >> 4) & 0xF;
		uint8_t nib1 = (substituted >> 0) & 0xF;
		uint8_t orig0 = inverse_substitute_4bit(nib0);
		uint8_t orig1 = inverse_substitute_4bit(nib1);
		mkh &= 0x00FFFFFFFFFFFFFFULL;
		mkh |= ((uint64_t)orig0 << 60);
		mkh |= ((uint64_t)orig1 << 56);

		rotate_left_67_bits(&mkh, &mkl);
	}
	*out_high = mkh;
	*out_low = mkl;
}

/*
 * Loads the first two (plaintext, ciphertext) pairs from the specified file.
 * Used as input constraints for the master key search.
 */
int load_pairs(const char* path, Pair* pairs, size_t* count) {
	FILE* fp = fopen(path, "r");
	if (!fp) { perror("file"); return 0; }
	*count = 0;
	while (*count < 2 && fscanf(fp, "%llx %llx", &pairs[*count].plaintext, &pairs[*count].ciphertext) == 2)
		++(*count);
	fclose(fp);
	return *count == 2;
}

/*****************************************************************
 * 2. Master key search loop: Given the recovered xored 32-bit keys,
 *    brute-forces all 2^35 possible master key candidates and tests
 *    them against known pt-ct pairs.
 *****************************************************************/

#define PRINT_STEP   (1ULL << 23)  

int search_master_key(
	uint32_t     RK22_K14,
	uint32_t     RK23_K14,
	uint32_t     RK24_K14,
	const Pair* pairs,
	size_t       pair_cnt,
	uint64_t* found_hi,
	uint64_t* found_lo
) {
	volatile int      found = 0;            // Found flag for OpenMP
	volatile uint64_t processed = 0;        // Total tested
	const  uint64_t   TOTAL = 1ULL << 35;   // 34,359,738,368 candidates

	uint64_t hi_out = 0, lo_out = 0;
	const double t_start = omp_get_wtime();
	int input_bits = 0;
#pragma omp parallel for schedule(static) shared(found,processed)
	for (input_bits = 0; input_bits < 64; ++input_bits)
	{
		if (found) continue;    // Stop if another thread succeeded

		for (uint64_t idx = 0; idx < (1ULL << 29) && !found; ++idx)
		{
			// (1) Construct 128-bit master key candidate according to bit constraints
			//     (using S-box inverses and XORed round key constraints)
			uint8_t MK64 = (input_bits >> 5) & 1;
			uint8_t MK63 = (input_bits >> 4) & 1;
			uint8_t MK62 = (input_bits >> 3) & 1;
			uint8_t MK61 = (input_bits >> 2) & 1;
			uint8_t MK60 = (input_bits >> 1) & 1;
			uint8_t MK59 = input_bits & 1;

			uint8_t MK62_61_60_59 = (MK62 << 3) | (MK61 << 2) | (MK60 << 1) | MK59;
			uint8_t MK68_67_66_65 = inverse_substitute_4bit(MK62_61_60_59) ^ ((RK22_K14 >> 1) & 0xF);
			uint8_t MK66_65 = MK68_67_66_65 & 0x3;
			uint8_t MK66_65_64_63 = (MK66_65 << 2) | (MK64 << 1) | MK63;
			uint8_t MK72_71_70_69 = inverse_substitute_4bit(MK66_65_64_63) ^ 0x6 ^ ((RK22_K14 >> 5) & 0xF);

			uint8_t MK72 = (MK72_71_70_69 >> 3) & 1;
			uint8_t MK71 = (MK72_71_70_69 >> 2) & 1;
			uint8_t MK70 = (MK72_71_70_69 >> 1) & 1;
			uint8_t MK69 = MK72_71_70_69 & 1;
			uint8_t MK68 = (MK68_67_66_65 >> 3) & 1;
			uint8_t MK67 = (MK68_67_66_65 >> 2) & 1;
			uint8_t MK66 = (MK68_67_66_65 >> 1) & 1;
			uint8_t MK65 = MK68_67_66_65 & 1;

			// Stage 1-5 expansion
			uint8_t MK125 = MK64 ^ ((RK24_K14 >> 0) & 1);
			uint8_t MK126 = MK65 ^ ((RK24_K14 >> 1) & 1);
			uint8_t MK127 = MK66 ^ ((RK24_K14 >> 2) & 1);
			uint8_t MK0 = MK67 ^ ((RK24_K14 >> 3) & 1);
			uint8_t MK1 = MK68 ^ ((RK24_K14 >> 4) & 1) ^ 1;
			uint8_t MK2 = MK69 ^ ((RK24_K14 >> 5) & 1);
			uint8_t MK3 = MK70 ^ ((RK24_K14 >> 6) & 1) ^ 1;
			uint8_t MK4 = MK71 ^ ((RK24_K14 >> 7) & 1) ^ 1;
			uint8_t MK5 = MK72 ^ ((RK24_K14 >> 8) & 1);

			uint8_t MK58 = MK64 ^ ((RK22_K14 >> 0) & 1);
			uint8_t MK73 = MK67 ^ ((RK22_K14 >> 9) & 1);
			uint8_t MK74 = MK68 ^ ((RK22_K14 >> 10) & 1);
			uint8_t MK75 = MK69 ^ ((RK22_K14 >> 11) & 1);
			uint8_t MK76 = MK70 ^ ((RK22_K14 >> 12) & 1);
			uint8_t MK77 = MK71 ^ ((RK22_K14 >> 13) & 1);
			uint8_t MK78 = MK72 ^ ((RK22_K14 >> 14) & 1);

			uint8_t MK6 = MK73 ^ ((RK24_K14 >> 9) & 1);
			uint8_t MK7 = MK74 ^ ((RK24_K14 >> 10) & 1);
			uint8_t MK8 = MK75 ^ ((RK24_K14 >> 11) & 1);
			uint8_t MK9 = MK76 ^ ((RK24_K14 >> 12) & 1);
			uint8_t MK10 = MK77 ^ ((RK24_K14 >> 13) & 1);
			uint8_t MK11 = MK78 ^ ((RK24_K14 >> 14) & 1);

			uint8_t MK79 = MK73 ^ ((RK22_K14 >> 15) & 1);
			uint8_t MK80 = MK74 ^ ((RK22_K14 >> 16) & 1);
			uint8_t MK81 = MK75 ^ ((RK22_K14 >> 17) & 1);
			uint8_t MK82 = MK76 ^ ((RK22_K14 >> 18) & 1);
			uint8_t MK83 = MK77 ^ ((RK22_K14 >> 19) & 1);
			uint8_t MK84 = MK78 ^ ((RK22_K14 >> 20) & 1);

			uint8_t MK12 = MK79 ^ ((RK24_K14 >> 15) & 1);
			uint8_t MK13 = MK80 ^ ((RK24_K14 >> 16) & 1);
			uint8_t MK14 = MK81 ^ ((RK24_K14 >> 17) & 1);
			uint8_t MK15 = MK82 ^ ((RK24_K14 >> 18) & 1);
			uint8_t MK16 = MK83 ^ ((RK24_K14 >> 19) & 1);
			uint8_t MK17 = MK84 ^ ((RK24_K14 >> 20) & 1);

			uint8_t MK85 = MK79 ^ ((RK22_K14 >> 21) & 1);
			uint8_t MK86 = MK80 ^ ((RK22_K14 >> 22) & 1);
			uint8_t MK87 = MK81 ^ ((RK22_K14 >> 23) & 1);
			uint8_t MK88 = MK82 ^ ((RK22_K14 >> 24) & 1);
			uint8_t MK89 = MK83 ^ ((RK22_K14 >> 25) & 1);
			uint8_t MK90 = MK84 ^ ((RK22_K14 >> 26) & 1);

			uint8_t MK18 = MK85 ^ ((RK24_K14 >> 21) & 1);
			uint8_t MK19 = MK86 ^ ((RK24_K14 >> 22) & 1);
			uint8_t MK20 = MK87 ^ ((RK24_K14 >> 23) & 1);
			uint8_t MK21 = MK88 ^ ((RK24_K14 >> 24) & 1);
			uint8_t MK22 = MK89 ^ ((RK24_K14 >> 25) & 1);
			uint8_t MK23 = MK90 ^ ((RK24_K14 >> 26) & 1);

			uint8_t MK91 = MK85 ^ ((RK22_K14 >> 27) & 1);
			uint8_t MK92 = MK86 ^ ((RK22_K14 >> 28) & 1);
			uint8_t MK93 = MK87 ^ ((RK22_K14 >> 29) & 1);
			uint8_t MK94 = MK88 ^ ((RK22_K14 >> 30) & 1);
			uint8_t MK95 = MK89 ^ ((RK22_K14 >> 31) & 1);

			uint8_t MK24 = MK91 ^ ((RK24_K14 >> 27) & 1);
			uint8_t MK25 = MK92 ^ ((RK24_K14 >> 28) & 1);
			uint8_t MK26 = MK93 ^ ((RK24_K14 >> 29) & 1);
			uint8_t MK27 = MK94 ^ ((RK24_K14 >> 30) & 1);
			uint8_t MK28 = MK95 ^ ((RK24_K14 >> 31) & 1);

			// (2) Insert 64+29 free bits for brute-force
			uint64_t MK_hi = 0, MK_lo = 0;
			uint64_t MK57_29 = idx & 0x1FFFFFFFULL;
			uint64_t MK124_96 = idx ^ (RK23_K14 & 0x1FFFFFFF);

			MK_lo = MK57_29 << 29;
			MK_hi = MK124_96 << 32;

			// (3) Set all fixed/inferred bits into the master key
			MK_hi |= (uint64_t)MK64 << 0;  /* ... and so on, see full bit assignments above ... */
			// ... [bit assignments omitted for brevity, see above] ...

			// (4) Reverse transformations to original master key
			uint64_t mk_hi, mk_lo;
			recover_master_key(MK_hi, MK_lo, &mk_hi, &mk_lo);

			// (5) Test this master key on pt-ct pairs
			uint8_t mk_bytes[16];
			memcpy(mk_bytes, &mk_hi, 8);
			memcpy(mk_bytes + 8, &mk_lo, 8);

			KeySchedule ks;
			key_schedule(mk_bytes, &ks);

			if (check_key_candidate(&pairs[0], &ks, "E:/CHECK.txt")) {
#pragma omp critical
				if (!found) {
					found = 1;
					hi_out = mk_hi;
					lo_out = mk_lo;

					double el = omp_get_wtime() - t_start;
					printf("\n[✓] Master key found  (input_bits=%d, idx=%llu)\n"
						"    elapsed %.1f s (%.2f h)\n\n",
						input_bits, (unsigned long long)idx,
						el, el / 3600.0);
				}
				break; // Exit idx loop
			}

			// (6) Print progress and ETA
#pragma omp atomic
			++processed;

			if ((processed % PRINT_STEP) == 0 && !found) {
				double el = omp_get_wtime() - t_start;
				double pct = (double)processed / (double)TOTAL;
				double eta = (el / pct) - el;

#pragma omp critical
				{
					printf("\r[*] %6.2f%%  processed %llu / %llu  "
						"elapsed %.0f s  ETA %.0f s      ",
						pct * 100.0,
						(unsigned long long)processed,
						(unsigned long long)TOTAL,
						el,
						eta);
					fflush(stdout);
				}
			}
		}
	}
}

/*****************************************************************
 * 3. Main: Load files, launch master key search
 *****************************************************************/

int main(void)
{
	// These are 32-bit xored keys recovered from DC attacks
	uint32_t RK24_K14 = 0x2367374C;
	uint32_t RK23_K14 = 0x4B2B06E5;
	uint32_t RK22_K14 = 0x34B4420E;

	Pair pairs[2];
	size_t pair_cnt;
	if (!load_pairs("E:\\CHECK.txt", pairs, &pair_cnt)) {
		fprintf(stderr, "pair file load error\n");
		return 1;
	}

	uint64_t mk_hi, mk_lo;
	if (search_master_key(RK22_K14, RK23_K14, RK24_K14, pairs, pair_cnt, &mk_hi, &mk_lo)) {
		printf("Master key found:\n"
			"  HI = %016llX\n"
			"  LO = %016llX\n",
			(unsigned long long)mk_hi,
			(unsigned long long)mk_lo);
	}
	else {
		puts("Master key not found in 2^35 candidates.");
	}
	return 0;
}
