# MGFN-24R Differential Cryptanalysis and Master-Key Recovery

This repository provides C and Python code for differential cryptanalysis and 128-bit master-key recovery of the 24-round MGFN-24R block cipher.

**All file paths, dataset sizes, mask values, and key parameters must be set by editing variables at the top of each script or source file before running. There is no “one-click” automation; you have full control and must configure each step.**

---

## Pipeline Overview

1. **Find a Master Key Satisfying Key Schedule Constraints**

   - Run `key_condition.py` to search for a 128-bit master key whose key schedule satisfies the required bitwise XOR constraints.
   - Open `key_condition.py` and set any variables such as constraint logic, random seed, or maximum trials as needed.
   - Run:  
     `python key_condition.py`

2. **Generate Differential Pair Dataset with Ciphertext Mask Filtering**

   - Use `dif_gen_data.c` to generate a large set of (plaintext, ciphertext) pairs, saving only pairs that pass a 16-bit ciphertext mask filter.
   - Open `dif_gen_data.c` and set:
     - Number of pairs to generate
     - Filter mask
     - Output file path
     - Batch size
     - (Optional) OpenMP thread count
   - Compile and run:  
     `gcc -fopenmp -O2 -o dif_gen_data dif_gen_data.c`  
     `./dif_gen_data`

3. **Generate All Possible Ciphertext Differences**

   - Use `gen_all_possible_difference.py` to enumerate all possible ciphertext differences for your chosen characteristic.
   - Open `gen_all_possible_difference.py` and set:
     - Value ranges
     - Output file path
     - Any multiprocessing settings
   - Run:  
     `python gen_all_possible_difference.py`

4. **Merge and Binarize Data with Differences**

   - Merge the pair data with the difference list using `merge_with_dif.py`, `merge_with_dif_1.py`, or `merge_with_dif_2.py`.
   - Open the script (e.g., `merge_with_dif_2.py`) and set:
     - Input pair file path
     - Difference file path
     - Output file path
     - Any key values or mask values needed
   - Run:  
     `python merge_with_dif_2.py`

5. **Filter the Dataset by Difference Intersection**

   - Use `filter.py` to filter/merge the datasets.
   - Open `filter.py` and set:
     - Input/output file paths
     - Chunk size
     - Number of processes
     - Any other parameters
   - Run:  
     `python filter.py`

6. **(Optional) Repeat for Another Differential Characteristic**

   - Repeat steps 3–5 for another characteristic if needed to recover all round-key XORs.

7. **Recover Round-Key XORs via S-box Analysis**

   - Use `MGFN_DC.c` to recover three target round-key XOR values (RK24 xor K14_R, RK23 xor K14_L, RK22 xor K14_*).
   - Open `MGFN_DC.c` and set:
     - Input file paths
     - Analysis function to use
     - Any key or S-box parameters needed
   - Compile and run:  
     `gcc -fopenmp -O2 -o MGFN_DC MGFN_DC.c`  
     `./MGFN_DC`

8. **Master Key Recovery with Exhaustive Search**

   - With the recovered round-key XOR values and at least one known (plaintext, ciphertext) pair, run `find_masterkey.c` to recover the master key.
   - Open `find_masterkey.c` and set:
     - Input/output file paths
     - Number of OpenMP threads
     - Verification pair count
   - Compile and run:  
     `gcc -fopenmp -O2 -o find_masterkey find_masterkey.c`  
     `./find_masterkey`

---

## Project Structure

