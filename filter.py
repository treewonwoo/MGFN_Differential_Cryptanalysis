import os, mmap, struct, tempfile, time, pickle, numpy as np
import multiprocessing as mp, tqdm

SRC = r"E:\results\700000000002000.txt"   # Path to input TXT file (hex values)
PROCS = 32                                # Number of worker processes
PACK  = struct.pack
CHUNK = 10_000_000                        # Process 10 million entries at a time

# --- (1) Distributed TXT→bin conversion: worker for each chunk ---
def worker(bounds):
    start, end = bounds
    with open(SRC, 'r') as f, mmap.mmap(f.fileno(), 0, access=mmap.ACCESS_READ) as mm:
        if start:
            # Align start to next line to avoid partial line
            while mm[start-1:start] != b'\n': start += 1
        if end < mm.size():
            # Align end to next line
            while mm[end:end+1] != b'\n': end += 1

        mm.seek(start)
        tmp = tempfile.NamedTemporaryFile(delete=False, suffix=".bin")
        cnt = 0
        while mm.tell() < end:
            line = mm.readline()
            if not line: break
            tmp.write(PACK("Q", int(line.rstrip(), 16)))  # Write as uint64 binary
            cnt += 1
        tmp.close()
    return tmp.name, cnt   # Return path and count

# --- (2) Main ---
if __name__ == "__main__":
    t0   = time.time()
    size = os.path.getsize(SRC)
    splits = [(i*size//PROCS, (i+1)*size//PROCS) for i in range(PROCS)]

    with mp.Pool(PROCS) as pool:
        tmp_files = list(tqdm.tqdm(pool.imap(worker, splits),
                                   total=PROCS, desc="TXT → bin"))

    # Load cp (ciphertext-pair dict) & prepare key set (fit in memory)
    with open("E:/sixth_merge.bin", "rb") as f:
        cp = pickle.load(f)
    cpkey = set(cp.keys())     # Python set for O(1) membership test

    # Open output file for writing filtered pairs
    with open("E:/sixth_filter.txt", "w") as out:
        for path, cnt in tmp_files:
            with open(path, "rb") as binf:
                remain = cnt
                pbar = tqdm.tqdm(total=cnt, desc=f"Checking {os.path.basename(path)}")
                while remain:
                    take = min(CHUNK, remain)
                    chunk = np.fromfile(binf, dtype=np.uint64, count=take)
                    for val in chunk:
                        if val in cpkey:  # Check intersection
                            for pt1, pt2, cp1, cp2 in cp[val]:
                                out.write(f"{pt1:016X} {cp1:016X}\n"
                                          f"{pt2:016X} {cp2:016X}\n")
                    remain -= take
                    pbar.update(take)
            os.remove(path)  # Remove temporary binary chunk

    print("Done, total elapsed:", round(time.time() - t0, 1), "seconds")
