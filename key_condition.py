#!/usr/bin/env python3
"""
find_key_by_constraints.py
──────────────────────────────────────────────────────────────────
Given the MGFN KeySchedule, this tool generates 26 round keys (RK),
and searches for a 128-bit MasterKey that satisfies specific constraints
on the round keys (as described in the original paper/figure).
"""

View_KeySchedule = True
# View_KeySchedule = False
View_Round = True
# View_Round = False
View_Round_Detail = True
# View_Round_Detail = False

import secrets
from typing import List
import sys
import argparse

# ────────────────────────────────────────────────────────────────
# ①  MGFN KeySchedule (original; output = [RK_0, RK_1, ..., RK_25])
# ────────────────────────────────────────────────────────────────
_SBOX = [0x7, 0xE, 0xF, 0x0, 0xD, 0xB, 0x8, 0x1,
         0x9, 0x3, 0x4, 0xC, 0x2, 0x5, 0xA, 0x6]

def Key_Schedule(MK):
    Sbox = [7,0xE,0xF,0x0,0xD,0xB,0x8,0x1,0x9,0x3,0x4,0xC,0x2,0x5,0xA,0x6]
    RK = []; K_i = []
    K = MK
    if View_KeySchedule: print()
    
    for i in range(14):
        rc = (i+1)<<62

        K_i.append(((K>>61)|(K<<67))&0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF)
        if i == 0:    RK.append(K_i[-1]>>64)
        elif i < 13:  RK.append(K_i[-1]>>96); RK.append((K_i[-1]>>64)&0xFFFFFFFF)
        elif i == 13: RK.append(K_i[-1]>>64)
        
        K = ((K<<61)|(K>>67))&0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
        K = (Sbox[(K >> 124)&0xF]<<124)|(Sbox[(K >> 120)&0xF]<<120)|(K&0x00FFFFFFFFFFFFFFFFFFFFFFFFFFFFFF)
        K ^= rc

    if View_KeySchedule:
        print('=== Update K_i ===')
        for i in range(len(K_i)):
            print(f'K_{i:02d} : {K_i[i]:032X}',end=' ')
            if i == 0:            print('-> Pre-whitening Key')
            elif i == len(K_i)-1: print('-> Post-whitening Key')
            else: print(f'-> {2*i-2}, {2*i-1} Round key ')
        print('')

        print('=== Round Key ===')
        for i in range(len(RK)):
            if i == 0: print(f' Pre-W Round Key: {RK[i]:016X}')
            elif i < len(RK)-1: print(f'  {i:2d}th Round Key: {RK[i]:08X}')
            elif i == len(RK)-1: print(f'Post-W Round Key: {RK[i]:016X}')
        print('')
    return RK


# ────────────────────────────────────────────────────────────────
# ②  Bit-access Helper
# ────────────────────────────────────────────────────────────────
def bit(val: int, idx: int) -> int:
    """Return the value (0 or 1) of bit at position idx (LSB = 0)."""
    return (val >> idx) & 1

# ────────────────────────────────────────────────────────────────
# ③  Six XOR constraints from the "figure" (key schedule constraints)
# ────────────────────────────────────────────────────────────────
def satisfy_constraints(rk: List[int]) -> bool:
    # Constraint 1
    if bit(rk[2], 3) ^ bit(rk[3], 28) ^ bit(rk[4], 3) ^ \
       bit(rk[6], 3) ^ bit(rk[7], 28) ^ bit(rk[8], 3):
        return False

    # Constraint 2
    if bit(rk[10], 3) ^ bit(rk[11], 28) ^ bit(rk[12], 3) ^ \
       bit(rk[14], 3) ^ bit(rk[15], 28) ^ bit(rk[16], 3):
        return False

    # Constraint 3
    if bit(rk[1], 3) ^ bit(rk[2], 28) ^ bit(rk[3], 3) ^ \
       bit(rk[5], 3) ^ bit(rk[6], 28) ^ bit(rk[7], 3):
        return False

    # Constraint 4
    if bit(rk[9], 3) ^ bit(rk[10], 28) ^ bit(rk[11], 3) ^ \
       bit(rk[13], 3) ^ bit(rk[14], 28) ^ bit(rk[15], 3):
        return False

    # Constraint 5
    if bit(rk[2], 25) ^ bit(rk[3], 13) ^ bit(rk[4], 25) ^ \
       bit(rk[6], 25) ^ bit(rk[7], 13) ^ bit(rk[8], 25):
        return False

    # Constraint 6
    if bit(rk[10], 25) ^ bit(rk[11], 13) ^ bit(rk[12], 25) ^ \
       bit(rk[14], 25) ^ bit(rk[15], 13) ^ bit(rk[16], 25):
        return False

    if bit(rk[1], 25) ^ bit(rk[2], 13) ^ bit(rk[3], 25) ^ \
       bit(rk[5], 25) ^ bit(rk[6], 13) ^ bit(rk[7], 25):
        return False
    
    if bit(rk[9], 25) ^ bit(rk[10], 13) ^ bit(rk[11], 25) ^ \
       bit(rk[13], 25) ^ bit(rk[14], 13) ^ bit(rk[15], 25):
        return False
    return True

# ────────────────────────────────────────────────────────────────
# ④  Brute-force search loop (single-threaded version)
#     – For parallelization, simply split into Pool workers.
# ────────────────────────────────────────────────────────────────
def find_key(max_trials: int = 10_000_000) -> int | None:
    """
    Try up to max_trials random 128-bit keys, returning the first that satisfies all constraints.
    """
    for trial in range(1, max_trials + 1):
        mk = secrets.randbits(128)
        if satisfy_constraints(Key_Schedule(mk)):
            print(f"[!]  Found key satisfying constraints (trial {trial:,d})")
            return mk

        if trial % 100_000 == 0:
            print(f"  … {trial:,d} keys tested", end="\r")

    return None

# ────────────────────────────────────────────────────────────────
# ⑤  Key check from hex string
# ────────────────────────────────────────────────────────────────
def check_single_key(
        hex_key: str,
) -> None:
    """
    Takes a 32-hex-digit (128-bit) string and checks whether it satisfies the key schedule constraints.
    """
    # Remove '0x' prefix if present
    if hex_key.lower().startswith("0x"):
        hex_key = hex_key[2:]

    if len(hex_key) != 32 or any(c not in "0123456789abcdefABCDEF" for c in hex_key):
        print("❌  Key must be a 32-digit hexadecimal string.")
        return

    mk = int(hex_key, 16)
    rks = Key_Schedule(mk)        # Generate all 26 round keys
    ok  = satisfy_constraints(rks)

    print(f"\nInput 128-bit key : 0x{mk:032X}")
    print(f"Constraint status  : {'YES 🎉' if ok else 'NO ❌'}\n")

# ────────────────────────────────────────────────────────────────
# ⑥  Main
# ────────────────────────────────────────────────────────────────
def main() -> None:
    """
    Prompt for 128-bit key as a hex string from stdin, and check constraints.
    """
    try:
        hex_key = input("Enter 32-digit hex (128-bit) key > ").strip()
    except (EOFError, KeyboardInterrupt):
        print()
        return
    check_single_key(hex_key)

if __name__ == "__main__":
    main()
