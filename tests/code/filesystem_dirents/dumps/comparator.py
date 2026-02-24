import os
import sys
import hashlib
import re

if len(sys.argv) != 3:
    print(f"comparator.py [PS4] [Emulator]")
    print("Compare dirent dumps between console and dump")
    sys.exit(0)

dir_left = sys.argv[1]
dir_right = sys.argv[2]

dir_left_contents = None
dir_right_contents = None

try:
    dir_left_contents = os.listdir(dir_left)
    dir_right_contents = os.listdir(dir_right)
except:
    print("Error during listing directories")
    sys.exit(-1)

for filename in dir_left_contents:
    file_left_path = os.path.join(os.path.abspath(dir_left), filename)
    file_right_path = os.path.join(os.path.abspath(dir_right), filename)
    is_pfs = "PFS" in filename

    # temporarily
    if is_pfs:
        continue

    print()
    print(f"<<<< Testing file {filename} >>>>")

    size_left = os.path.getsize(file_left_path)
    size_right = os.path.getsize(file_right_path)
    size_match = size_left == size_right

    content_left = None
    content_right = None
    with open(file_left_path, "rb") as lhsf:
        content_left = lhsf.read()
    with open(file_right_path, "rb") as rhsf:
        content_right = rhsf.read()

    hash_left = hashlib.md5(content_left).hexdigest()
    hash_right = hashlib.md5(content_right).hexdigest()
    hash_match = hash_left == hash_right

    print(f"Size:\t{size_match}\t{size_left}\t{size_right}")
    print(f"MD5:\t{hash_match}\t{hash_left}\t{hash_right}")

    if size_match and hash_match:
        continue

    left_file_list = []
    right_file_list = []

    left_dirent_offsets = []
    right_dirent_offsets = []

    left_skipped_bytes = []
    right_skipped_bytes = []

    prev_end = 0
    search_query = re.finditer(b"(.{8})([ -~]{1,255})\x00", content_left)
    for lsresult in search_query:
        left_file_list.append(lsresult.group(2))
        result_pos = lsresult.start()
        left_dirent_offsets.append(result_pos)
        left_skipped_bytes_temp = result_pos - prev_end
        if left_skipped_bytes_temp != 0:
            left_skipped_bytes.append(left_skipped_bytes_temp)
        prev_end = lsresult.end()

    prev_end = 0
    search_query = re.finditer(b"(.{8})([ -~]{1,255})\x00", content_left)
    for lsresult in search_query:
        right_file_list.append(lsresult.group(2))
        result_pos = lsresult.start()
        right_dirent_offsets.append(result_pos)
        right_skipped_bytes_temp = result_pos - prev_end
        if right_skipped_bytes_temp != 0:
            right_skipped_bytes.append(right_skipped_bytes_temp)

        prev_end = lsresult.end()

    left_set = set(left_file_list)
    right_set = set(right_file_list)
    if len(left_set) != len(left_file_list):
        print("Left has repeating filenames")
    if len(right_set) != len(right_file_list):
        print("Right has repeating filenames")
    merged_results = set(left_file_list + right_file_list)
    if (len(merged_results) != len(left_file_list)) or (
        len(merged_results) != len(right_file_list)
    ):
        print(
            f"Unique differences between files: L:{len(left_file_list)}<->R:{len(right_file_list)}\tTotal:{len(merged_results)}"
        )

    for idx, left_excl_filename in enumerate(right_file_list):
        if left_excl_filename not in right_file_list:
            print(f"Right exclusive:{left_excl_filename}")

    for idx, right_excl_filename in enumerate(left_file_list):
        if right_excl_filename not in left_file_list:
            print(f"Right exclusive:{right_excl_filename}")

    for idx, offset_value in enumerate(left_dirent_offsets):
        if offset_value != right_dirent_offsets[idx]:
            print(
                f"Left: Inconsistent offsets: L:{offset_value}<=>R:{right_dirent_offsets[idx]}"
            )
    for idx, offset_value in enumerate(right_dirent_offsets):
        if offset_value != left_dirent_offsets[idx]:
            print(
                f"Right: Inconsistent offsets: L:{offset_value}<=>R:{left_dirent_offsets[idx]}"
            )

    for idx, skipped_bytes in enumerate(left_skipped_bytes):
        if skipped_bytes != right_skipped_bytes[idx]:
            print(
                f"Left: Inconsistent skipped byted: L:{skipped_bytes}<=>R:{right_skipped_bytes[idx]}"
            )
    for idx, skipped_bytes in enumerate(right_skipped_bytes):
        if skipped_bytes != left_skipped_bytes[idx]:
            print(
                f"Right: Inconsistent skipped byted: L:{skipped_bytes}<=>R:{left_skipped_bytes[idx]}"
            )


    

    
    pass
