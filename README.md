MGFN-24R Differential Cryptanalysis and Master-Key Recovery
This repository provides C and Python code for practical differential cryptanalysis and full 128-bit master-key recovery of the 24-round MGFN-24R block cipher.
The project implements a step-by-step pipeline inspired by real-world block cipher attacks and is designed to let you control every stage, from key selection to key recovery.
All parameters, file paths, and data sizes must be set by editing variables at the top of each script or source file before running each step. This modular approach gives you complete flexibility.

🔗 Pipeline Overview
Below is the typical analysis and recovery flow, with detailed descriptions for each stage and script.

1. Find a Master Key Satisfying Key Schedule Constraints
Run key_condition.py to search for a 128-bit master key whose round-key schedule meets all required bitwise XOR constraints (as described in the target cryptanalysis paper).

Important:
Before running, open key_condition.py and set any variables such as constraint logic, random seed, or number of trials if needed.

bash
복사
편집
python key_condition.py
The script prints the found key and its round key schedule.

2. Generate Differential Pair Dataset with Ciphertext Mask Filtering
Use dif_gen_data.c to generate a very large set of (plaintext, ciphertext) pairs, only saving pairs whose ciphertext difference passes a 16-bit mask filter (as required by the differential characteristic).

Important:
Before compiling and running, open dif_gen_data.c and set:

Number of pairs to generate

Filter mask

Output file path

Batch size

(Optional) Number of OpenMP threads
All are defined as macros or variables at the top of the source file.

bash
복사
편집
gcc -fopenmp -O2 -o dif_gen_data dif_gen_data.c
./dif_gen_data
3. Generate All Possible Ciphertext Differences
Use gen_all_possible_difference.py to enumerate all possible ciphertext differences for the chosen differential characteristic.

Important:
Open gen_all_possible_difference.py and set:

Input value range

Output file path

Any multiprocessing settings
at the top of the script before running.

bash
복사
편집
python gen_all_possible_difference.py
4. Merge and Binarize Data with Differences
Merge the differential pair data with the possible differences using merge_with_dif.py, merge_with_dif_1.py, or merge_with_dif_2.py.

Important:
Before running, open the merge script (e.g., merge_with_dif_2.py) and set:

Input data file path (PAIR_FILE or similar)

Difference file path (DIF_FILE or similar)

Output file path (OUTPUT_FILE)

Any key values or filter masks
at the top of the script.

bash
복사
편집
python merge_with_dif_2.py
5. Filter the Dataset by Difference Intersection
Apply filter.py to intersect the merged dataset with your difference set.

Important:
Before running, open filter.py and set:

Input/output file paths

Chunk size

Number of processes

Any required parameters
at the top of the script.

bash
복사
편집
python filter.py
Repeat steps 3~5 for a different differential characteristic if needed.

6. Recover Round-Key XORs via S-box Difference Analysis
Use MGFN_DC.c with the two filtered datasets to perform S-box difference intersection analysis and recover the final three 32-bit round-key XORs (RK24 ⊕ K14_R, RK23 ⊕ K14_L, RK22 ⊕ K14_*).

Important:
Before compiling and running, open MGFN_DC.c and set:

Input file paths

Analysis function (e.g., analyze_1r_dc)

Any S-box or round-key parameters
at the top of the source file.

bash
복사
편집
gcc -fopenmp -O2 -o MGFN_DC MGFN_DC.c
./MGFN_DC
7. Master Key Recovery with Exhaustive Search
With the three recovered 32-bit round-key XOR values and at least one known (plaintext, ciphertext) pair, run find_masterkey.c to exhaustively search for the unique master key.

Important:
Before compiling and running, open find_masterkey.c and set:

Input/output file paths

Number of OpenMP threads

Verification pair count
at the top of the source file.

bash
복사
편집
gcc -fopenmp -O2 -o find_masterkey find_masterkey.c
./find_masterkey
📁 Project Structure
css
복사
편집
MGFN_24R_DC_CODE/
├── src/
│   ├── dif_gen_data.c
│   ├── MGFN_DC.c
│   ├── find_masterkey.c
│   ├── key_condition.py
│   ├── gen_all_possible_difference.py
│   ├── merge_with_dif.py
│   ├── merge_with_dif_1.py
│   ├── merge_with_dif_2.py
│   └── filter.py
├── data/
│   └── (Generated and filtered data files)
⚠️ How to Customize
All file paths, dataset sizes, mask values, and performance parameters must be set at the top of each script or source file before running.

Each stage is independent and gives you full control; there is no automatic “one click” flow.

Intermediate data files can be large (several GB); choose storage and parameters accordingly.

🧪 Example Output
markdown
복사
편집
[*] Generating filtered pairs...
[*] Enumerating all possible differences...
[*] Merging differences...
[*] Filtering dataset...
[*] Recovering round-key XORs via S-box analysis...
[✓] Found RK24 ⊕ K14_R = 0x6E529673
[✓] Found RK23 ⊕ K14_L = 0x788490D0
[✓] Found RK22 ⊕ K14_* = 0x2D34241B
[*] Searching master key among 2^35 candidates...
Recovered : B745C5C6106198F3CA4CD45E2B9F910F
[OK] master_key matched
📄 License
vbnet
복사
편집
MIT License

Copyright (c) 2025 Wonwoo Song

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the “Software”), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
