import pickle
import tqdm
import collections

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

def sbox16(x: int, S: list[int]) -> int:
    return (S[(x >> 12) & 0xF] << 12 |
            S[(x >> 8) & 0xF] << 8 |
            S[(x >> 4) & 0xF] << 4 |
            S[x & 0xF])

S = [0x7, 0xe, 0xf, 0x0, 0xd, 0xb, 0x8, 0x1,
     0x9, 0x3, 0x4, 0xc, 0x2, 0x5, 0xa, 0x6]
xored_key1 = 0x6E529673

def key_filter_1round(pt1, cp1, pt2, cp2, xored_key1):
    xor_result = cp1 ^ cp2

    f1_hi = (((cp1 & 0xffffffff) ^ xored_key1) >> 16) & 0xffff 
    f1_lo = ((cp1 & 0xffffffff) ^ xored_key1) & 0xffff 
    f2_hi = (((cp2 & 0xffffffff) ^ xored_key1) >> 16) & 0xffff 
    f2_lo = ((cp2 & 0xffffffff) ^ xored_key1) & 0xffff  

    rf1 = ((f1_hi >> 3) | (f1_hi << 13)) & 0xFFFF
    rf2 = ((f1_lo >> 8) | (f1_lo << 8)) & 0xFFFF
    rs1 = ((f2_hi >> 3) | (f2_hi << 13)) & 0xFFFF
    rs2 = ((f2_lo >> 8) | (f2_lo << 8)) & 0xFFFF

    sf1 = sbox16(rf1, S)
    sf2 = sbox16(rf2, S)
    ss1 = sbox16(rs1, S)
    ss2 = sbox16(rs2, S)

    x1 = sf1 ^ ss1
    x2 = sf2 ^ ss2

    merged = (x1 << 16) | x2
    bitlist = int_to_bitlist32(merged)
    permuted = bitlist32_to_int(custom_permutation32(bitlist))

    diff_hi = permuted ^ (xor_result >> 32)
    diff_lo = xor_result & 0xffffffff
    return (diff_lo << 32) | diff_hi

if __name__ == "__main__":
    DATA_FILE = "E:/wonwoo/full_dc/3.txt"
    OUT_BIN   = "third_merge.bin"
    d = collections.defaultdict(set)

    with open(DATA_FILE, "r") as f:
        txt = f.readlines()

    for i in tqdm.tqdm(range(0, len(txt), 2), desc="1-round key filtering"):
        pt1, cp1 = map(lambda x: int(x.strip(), 16), txt[i].split())
        pt2, cp2 = map(lambda x: int(x.strip(), 16), txt[i+1].split())
        diff = key_filter_1round(pt1, cp1, pt2, cp2, xored_key1)
        d[diff].add((pt1, pt2, cp1, cp2))

    with open(OUT_BIN, "wb") as f:
        pickle.dump(d, f)
    print(f"\n✔ Done: {len(d)} unique differences with 1-round key filter saved.")
