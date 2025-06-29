import pickle
import collections
from pathlib import Path
from tqdm import tqdm

def int_to_bitlist32(x: int) -> list[int]:
    return [(x >> i) & 1 for i in range(31, -1, -1)]

def bitlist32_to_int(bits: list[int]) -> int:
    out = 0
    for b in bits:
        out = (out << 1) | b
    return out

def custom_permutation32(bits: list[int]) -> list[int]:
    idx = [
        15, 2, 17, 20, 0, 22, 24, 9,
        27, 5, 12, 19, 26, 29, 7, 3,
        13, 23, 10, 6, 30, 1, 11, 18,
        28, 8, 14, 31, 4, 25, 21, 16
    ]
    return [bits[i] for i in idx]

def rot16(x: int, r: int) -> int:
    r &= 0xF
    return ((x >> r) | (x << (16 - r))) & 0xFFFF

def sbox16(x: int, S: list[int]) -> int:
    return (S[(x >> 12) & 0xF] << 12 |
            S[(x >> 8) & 0xF] << 8 |
            S[(x >> 4) & 0xF] << 4 |
            S[x & 0xF])

def round_transform(ct: int, rk: int, S: list[int]) -> int:
    tmp = (ct & 0xFFFFFFFF) ^ rk
    a, b = (tmp >> 16) & 0xFFFF, tmp & 0xFFFF
    a = rot16(a, 3)
    b = rot16(b, 8)
    a, b = sbox16(a, S), sbox16(b, S)
    merged = (a << 16) | b
    permuted = bitlist32_to_int(custom_permutation32(int_to_bitlist32(merged)))
    new_hi = permuted ^ ((ct >> 32) & 0xFFFFFFFF)
    new_lo = ct & 0xFFFFFFFF
    return (new_lo << 32) | new_hi

S = [0x7, 0xE, 0xF, 0x0, 0xD, 0xB, 0x8, 0x1,
     0x9, 0x3, 0x4, 0xC, 0x2, 0x5, 0xA, 0x6]

xored_key1 = 0x6E529673
xored_key2 = 0x788490d0

if __name__ == "__main__":
    DATA_FILE = Path("E:/wonwoo/full_dc/6.txt")
    OUT_BIN = Path("sixth_merge.bin")
    d = collections.defaultdict(set)
    with DATA_FILE.open("r") as f:
        lines = f.readlines()
    for i in tqdm(range(0, len(lines), 2), desc="2-round key filtering"):
        pt1, cp1 = map(lambda s: int(s.strip(), 16), lines[i].split())
        pt2, cp2 = map(lambda s: int(s.strip(), 16), lines[i + 1].split())

        mid1 = round_transform(cp1, xored_key1, S)
        mid2 = round_transform(cp2, xored_key1, S)
        out1 = round_transform(mid1, xored_key2, S)
        out2 = round_transform(mid2, xored_key2, S)

        diff = out1 ^ out2
        d[diff].add((pt1, pt2, cp1, cp2))

    with OUT_BIN.open("wb") as f:
        pickle.dump(d, f)
    print(f"\n✔ Done: {len(d)} unique differences with 2-round key filter saved.")
