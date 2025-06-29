import pickle
import collections
from multiprocessing import Pool, cpu_count
from tqdm import tqdm

def parse_block_to_diffdict(lines: list[str]) -> dict[int, set[tuple[int, int, int, int]]]:
    """
    Each block (2 lines per pair): (pt1, cp1, pt2, cp2) → diff → defaultdict(set)
    """
    d = collections.defaultdict(set)
    for i in range(0, len(lines), 2):
        pt1, cp1 = (int(x, 16) for x in lines[i].strip().split())
        pt2, cp2 = (int(x, 16) for x in lines[i+1].strip().split())
        diff = cp1 ^ cp2
        d[diff].add((pt1, pt2, cp1, cp2))
    return d

def merge_diffdicts(dicts: list[dict[int, set]]) -> dict[int, set]:
    """
    Merge a list of defaultdict(set) into a single defaultdict(set).
    """
    merged = collections.defaultdict(set)
    for d in dicts:
        for k, v in d.items():
            merged[k].update(v)
    return merged

if __name__ == "__main__":
    DATA_FILE = "E:/wonwoo/full_dc/2.txt"
    OUT_BIN   = "second_merge.bin"
    with open(DATA_FILE, "r") as f:
        lines = f.readlines()
    assert len(lines) % 2 == 0, "Input file must have an even number of lines."

    block_size = 1024
    blocks = [lines[i:i + block_size] for i in range(0, len(lines), block_size)]

    with Pool(processes=cpu_count()) as pool:
        results = list(tqdm(pool.imap_unordered(parse_block_to_diffdict, blocks), total=len(blocks)))

    merged_d = merge_diffdicts(results)
    with open(OUT_BIN, "wb") as f:
        pickle.dump(merged_d, f)
    print(f"\n✔ Done: {len(merged_d)} unique ciphertext differences saved.")
