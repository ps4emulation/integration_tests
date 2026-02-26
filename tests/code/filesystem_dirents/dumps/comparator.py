import os
import sys
import re
from dataclasses import dataclass

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


@dataclass(kw_only=True)
class Dirent:
    chk: str
    offset: int
    end: int
    fileno: str  # 4 for pfs, 4 for reg
    entry_type: str  # 4 for pfs, 1 for reg
    namelen: str  # 4 for pfs, 1 for reg
    reclen: str  # 4 for pfs, 2 for reg
    name: str  # up to 255 characters + null terminator
    padding: str

    def __repr__(self) -> str:
        return self.chk

    def __eq__(self, other):
        if isinstance(other, Dirent):
            return False
        if (
            (self.offset != other.offset)
            # fileno is ignored, those can be different
            or (self.entry_type != other.entry_type)
            or (self.namelen != other.namelen)
            or (self.reclen != other.reclen)
            or (self.name != other.name)
            or (len(self.padding) != len(other.padding))
        ):
            return False


find_buffer_end_re = re.compile(b"\x00+(\xaa+)")
regular_dirent_query_re = re.compile(
    b"(?P<fileno>....)(?P<reclen>..)(?P<entry_type>[\x02\x04\x08])(?P<namelen>.)(?P<name>[ -~]{1,255})(?P<padding>\x00*)"
)
pfs_query_getdents_re = re.compile(
    b"(?P<fileno>.{4})(?P<entry_type>[\x02\x04\x08]\x00{3})(?P<namelen>....)(?P<reclen>....)(?P<name>[ -~]{1,255})(?P<padding>\x00*)"
)

for filename in dir_left_contents:
    file_left_path = os.path.join(os.path.abspath(dir_left), filename)
    file_right_path = os.path.join(os.path.abspath(dir_right), filename)
    is_pfs = "PFS" in filename
    is_read = "read" in filename
    is_getdents = "dirent" in filename

    if not (is_read or is_getdents):
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

    ### Verify size

    if size_left == 0 and size_right == 0:
        print("Both are empty. Continuing...")
        continue

    print(f"Size:\t{size_left}\t{size_right}")

    if not size_match:
        print("Error: sizes don't match. Continuing...")
        continue

    #
    ### Search for entries
    ### Search for entry offsets
    ### Search for skipped bytes (0-fills, cut off data)
    #
    left_dirent_list: list[Dirent] = []
    right_dirent_list: list[Dirent] = []
    search_query = None

    lsresult = None
    if is_pfs and is_read:
        search_query = pfs_query_getdents_re.finditer(content_left)
    else:
        search_query = regular_dirent_query_re.finditer(content_left)
    for lsresult in search_query:
        dirent_init_dict = lsresult.groupdict()
        dirent_init_dict["offset"] = lsresult.start()
        dirent_init_dict["end"] = lsresult.end()
        _name = dirent_init_dict["name"]
        _reclen = dirent_init_dict["reclen"]
        _offset = dirent_init_dict["offset"]
        dirent_init_dict["chk"] = f"{_name[-2:]}{str(_offset)}{str(_reclen)}"
        new_dirent = Dirent(**dirent_init_dict)
        left_dirent_list.append(new_dirent)
    if lsresult is None:
        print("Left: can't match file entries")

    lsresult = None
    if is_pfs and is_read:
        search_query = pfs_query_getdents_re.finditer(content_right)
    else:
        search_query = regular_dirent_query_re.finditer(content_right)
    for lsresult in search_query:
        dirent_init_dict = lsresult.groupdict()
        dirent_init_dict["offset"] = lsresult.start()
        dirent_init_dict["end"] = lsresult.end()
        _name = dirent_init_dict["name"]
        _reclen = dirent_init_dict["reclen"]
        _offset = dirent_init_dict["offset"]
        dirent_init_dict["chk"] = f"{_name[-2:]}{str(_offset)}{str(_reclen)}"
        new_dirent = Dirent(**dirent_init_dict)
        right_dirent_list.append(new_dirent)
    if lsresult is None:
        print("Right: can't match file entries")

    left_dirent_list_len = len(left_dirent_list)
    right_dirent_list_len = len(right_dirent_list)

    if left_dirent_list_len != right_dirent_list_len:
        print(
            f"Error: Different amount of dirents: L:{left_dirent_list_len} R:{right_dirent_list_len}. Continuing..."
        )
        continue

    for idx, lval in enumerate(left_dirent_list):
        rval = right_dirent_list[idx]
        if repr(lval) == repr(rval):
            continue
        print(f"Mismatch at\tL:{lval.offset}\tR:{rval.offset}")
        print(f"\t{lval.name}\t{rval.name}")

    print("Tests complete")
    pass
