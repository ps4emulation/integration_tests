PKG_CONTENT_ID="$1"
OO_PS4_TOOLCHAIN="$2"

FILES=$(find assets/ -type f -printf '%p ')
FILES="$FILES$(find sce_module/ -type f -name '*.prx' -printf '%p ')"

# Create gp4
$OO_PS4_TOOLCHAIN/bin/linux/create-gp4 -out pkg.gp4 --content-id="$PKG_CONTENT_ID" --files "eboot.bin sce_sys/about/right.sprx sce_sys/param.sfo sce_sys/icon0.png $FILES"

# Create pkg
$OO_PS4_TOOLCHAIN/bin/linux/PkgTool.Core pkg_build pkg.gp4 .