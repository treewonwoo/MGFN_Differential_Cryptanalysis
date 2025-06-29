import multiprocessing

def split_into_32bits(value):
    high_32 = (value >> 32) & 0xFFFFFFFF
    low_32 = value & 0xFFFFFFFF
    return high_32, low_32

def split_into_4bits(value):
    chunks = [(value >> (4 * i)) & 0xF for i in range(7, -1, -1)]
    return tuple(chunks)

def rotate_bits(pt):
    # Take upper 16 bits (bits 16~31)
    upper = (pt >> 16) & 0xFFFF
    # Take lower 16 bits (bits 0~15)
    lower = pt & 0xFFFF
    # Rotate upper 16 bits right by 3 bits
    upper_rotated = ((upper >> 3) | (upper << (16 - 3))) & 0xFFFF
    # Rotate lower 16 bits right by 8 bits
    lower_rotated = ((lower >> 8) | (lower << (16 - 8))) & 0xFFFF
    # Combine back to 32 bits
    result = (upper_rotated << 16) | lower_rotated
    return result

S = [0x7, 0xe, 0xf, 0x0, 0xd, 0xb, 0x8, 0x1, 0x9, 0x3, 0x4, 0xc, 0x2, 0x5, 0xa, 0x6]

possible_output = {
    0: [0],
    1: [6, 7, 8, 9, 10, 12, 15],
    2: [3, 5, 8, 10, 13, 14, 15],
    3: [1, 3, 4, 5, 7, 12, 15],
    4: [1,5,6,7,10,11,14],
    5: [1,2,3,6,8,12,14],
    6: [2,3,5,6,9,11,15],
    7: [1, 4, 6, 9, 13, 14, 15],
    8: [2, 7, 11, 12, 13, 14, 15],
    9: [3, 4, 7, 8, 9, 11, 14],
    10: [2, 3, 4, 6, 7, 10, 13],
    11: [1, 3, 9, 10, 11, 12, 13],
    12: [4, 5, 6, 8, 11, 12, 13],
    13: [2, 4, 5, 9, 10, 12, 14],
    14: [1, 2, 5, 7, 8, 9, 13],
    15: [1, 2, 4, 6, 10, 11, 15],
}

def custom_permutation(bit_list):
    upkey = [
        bit_list[15], bit_list[2], bit_list[17], bit_list[20],
        bit_list[0], bit_list[22], bit_list[24], bit_list[9],
        bit_list[27], bit_list[5], bit_list[12], bit_list[19],
        bit_list[26], bit_list[29], bit_list[7], bit_list[3],
        bit_list[13], bit_list[23], bit_list[10], bit_list[6],
        bit_list[30], bit_list[1], bit_list[11], bit_list[18],
        bit_list[28], bit_list[8], bit_list[14], bit_list[31],
        bit_list[4], bit_list[25], bit_list[21], bit_list[16]
    ]
    return upkey

def binary_list_to_hex(bit_list):
    return hex(int("".join(map(str, bit_list)), 2))

def generate_all_possible_difference(pt):
    # Split 64-bit value into two 32-bit halves
    high_32, low_32 = split_into_32bits(pt)
    # Rotate the upper 32 bits
    after_rotate_32bit = rotate_bits(high_32)
    after_rotate = after_rotate_32bit & 0xFFFFFFFF
    
    # Split the rotated result into eight 4-bit values
    bs_1, bs_2, bs_3, bs_4, bs_5, bs_6, bs_7, bs_8 = split_into_4bits(after_rotate)
    
    # Get possible outputs for each 4-bit value
    all_bs_outputs = [
        possible_output.get(bs_1, []),
        possible_output.get(bs_2, []),
        possible_output.get(bs_3, []),
        possible_output.get(bs_4, []),
        possible_output.get(bs_5, []),
        possible_output.get(bs_6, []),
        possible_output.get(bs_7, []),
        possible_output.get(bs_8, []),
    ]

    concatenated_hex_result = []

    for b1 in all_bs_outputs[0]:
        for b2 in all_bs_outputs[1]:
            for b3 in all_bs_outputs[2]:
                for b4 in all_bs_outputs[3]:
                    for b5 in all_bs_outputs[4]:
                        for b6 in all_bs_outputs[5]:
                            for b7 in all_bs_outputs[6]:
                                for b8 in all_bs_outputs[7]:
                                    # Create 32-bit value from eight 4-bit values
                                    value = (
                                        (b1 << 28)
                                        | (b2 << 24)
                                        | (b3 << 20)
                                        | (b4 << 16)
                                        | (b5 << 12)
                                        | (b6 << 8)
                                        | (b7 << 4)
                                        | b8
                                    )
                                    # Convert to binary list and apply permutation
                                    binary_list = [int(bit) for bit in format(value, '032b')]
                                    permuted_list = custom_permutation(binary_list)
                                    permuted_hex = binary_list_to_hex(permuted_list)
                                    # XOR with lower 32 bits
                                    permuted_value = int(permuted_hex, 16)
                                    result_32bit = permuted_value ^ low_32
                                    xor_value = result_32bit & 0xFFFFFFFF  
                                    xor_hex = f"{xor_value:08X}"
                                    high_32_hex = f"{high_32:08X}"
                                    concatenated_value = f"0x{xor_hex}{high_32_hex}"
                                    concatenated_hex_result.append(concatenated_value)
    return concatenated_hex_result

def save_results_to_file(results, filename="first.txt"):
    """Save results to file."""
    filepath = f"E:/results/{filename}"
    with open(filepath, "a") as file:
        for result in results:
            file.write(result + "\n")
            
def worker(pt):
    """Process each pt value and save the results to a file."""
    results = generate_all_possible_difference(pt)
    save_results_to_file(results, filename="first.txt")

def chunks(lst, n):
    """Split list into n chunks. If n is 0, treat as one chunk."""
    for i in range(0, len(lst), max(1, len(lst) // n)):
        yield lst[i:i + max(1, len(lst) // n)]

if __name__ == "__main__":
    # Example pt values to process
    list_pt = [0x0700000000000000]   
    num_processes = 32
    num_processes = min(num_processes, len(list_pt))
    with multiprocessing.Pool(processes=num_processes) as pool:
        chunked_list = list(chunks(list_pt, num_processes))
        pool.map(worker, [item for sublist in chunked_list for item in sublist])
    print("All results have been processed and saved to file.")
