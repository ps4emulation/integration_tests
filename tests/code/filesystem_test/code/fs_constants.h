#include <orbis/libkernel.h>

namespace DumpedConstants {

// real data
// st_dev, st_ino will differ between everything
// size, blocks and blksize seem constant

const OrbisKernelStat stat_root {
    .st_dev     = 0,
    .st_ino     = 0,
    .st_mode    = 040775,
    .st_nlink   = 0,
    .st_uid     = 0,
    .st_gid     = 0,
    .st_rdev    = 0,
    .st_size    = 360,
    .st_blocks  = 32,
    .st_blksize = 16384,
    .st_flags   = 0,
    .st_gen     = 0,
    .st_lspare  = 0,
};

const OrbisKernelStat stat_root_app0 = {
    .st_dev     = 0,
    .st_ino     = 0,
    .st_mode    = 040555,
    .st_nlink   = 0,
    .st_uid     = 0,
    .st_gid     = 0,
    .st_rdev    = 0,
    .st_size    = 65536,
    .st_blocks  = 128,
    .st_blksize = 65536,
    .st_flags   = 0,
    .st_gen     = 0,
    .st_lspare  = 0,
};

const OrbisKernelStat stat_root_app0_eboot = {
    .st_dev     = 0,
    .st_ino     = 0,
    .st_mode    = 0100555,
    .st_nlink   = 0,
    .st_uid     = 0,
    .st_gid     = 0,
    .st_rdev    = 0,
    .st_size    = 1645264,
    .st_blocks  = 3328,
    .st_blksize = 65536,
    .st_flags   = 0,
    .st_gen     = 0,
    .st_lspare  = 0,
};

const OrbisKernelStat stat_root_app0_assets_misc_file = {
    .st_dev     = 0,
    .st_ino     = 0,
    .st_mode    = 0100555,
    .st_nlink   = 0,
    .st_uid     = 0,
    .st_gid     = 0,
    .st_rdev    = 0,
    .st_size    = 45,
    .st_blocks  = 128,
    .st_blksize = 65536,
    .st_flags   = 0,
    .st_gen     = 0,
    .st_lspare  = 0,
};

const OrbisKernelStat stat_root_data = {
    .st_dev     = 0,
    .st_ino     = 0,
    .st_mode    = 040777,
    .st_nlink   = 0,
    .st_uid     = 0,
    .st_gid     = 0,
    .st_rdev    = 0,
    .st_size    = 512,
    .st_blocks  = 8,
    .st_blksize = 32768,
    .st_flags   = 0,
    .st_gen     = 0,
    .st_lspare  = 0,
};

const OrbisKernelStat stat_root_dev = {
    .st_dev     = 0,
    .st_ino     = 0,
    .st_mode    = 040555,
    .st_nlink   = 0,
    .st_uid     = 0,
    .st_gid     = 0,
    .st_rdev    = 0,
    .st_size    = 512,
    .st_blocks  = 1,
    .st_blksize = 16384,
    .st_flags   = 0,
    .st_gen     = 0,
    .st_lspare  = 0,
};

const OrbisKernelStat stat_blkdev = {
    .st_dev              = 0,
    .st_ino              = 0,
    .st_mode             = 020666,
    .st_nlink            = 0,
    .st_uid              = 0,
    .st_gid              = 0,
    .st_rdev             = 0,
    .st_size             = 0,
    .st_blocks           = 0,
    .st_blksize          = 16384,
    .st_flags            = 0,
    .st_gen              = 0,
    .st_lspare           = 0,
    .st_birthtim.tv_sec  = -1, // leave it alone :c
    .st_birthtim.tv_nsec = 0,
};

const OrbisKernelStat stat_root_host = {
    .st_dev     = 0,
    .st_ino     = 0,
    .st_mode    = 040777,
    .st_nlink   = 0,
    .st_uid     = 0,
    .st_gid     = 0,
    .st_rdev    = 0,
    .st_size    = 4096,
    .st_blocks  = 8,
    .st_blksize = 16384,
    .st_flags   = 0,
    .st_gen     = 0,
    .st_lspare  = 0,
};

const OrbisKernelStat stat_root_hostapp = {
    .st_dev     = 0,
    .st_ino     = 0,
    .st_mode    = 040777,
    .st_nlink   = 0,
    .st_uid     = 0,
    .st_gid     = 0,
    .st_rdev    = 0,
    .st_size    = 4096,
    .st_blocks  = 8,
    .st_blksize = 16384,
    .st_flags   = 0,
    .st_gen     = 0,
    .st_lspare  = 0,
};

const OrbisKernelStat stat_root_av_contents = {
    .st_dev     = 0,
    .st_ino     = 0,
    .st_mode    = 040775,
    .st_nlink   = 0,
    .st_uid     = 0,
    .st_gid     = 0,
    .st_rdev    = 0,
    .st_size    = 160,
    .st_blocks  = 32,
    .st_blksize = 16384,
    .st_flags   = 0,
    .st_gen     = 0,
    .st_lspare  = 0,
};
} // namespace DumpedConstants