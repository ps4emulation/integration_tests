#!/bin/bash
if [[ "$OO_PS4_TOOLCHAIN" == "" ]]; then
	echo "No OO_PS4_TOOLCHAIN is set";
	exit 1;
fi

PKG_TITLE="$1"
PKG_VERSION="$2"
PKG_TITLE_ID="$3"
PKG_CONTENT_ID="$4"
PKG_DOWNLOADSIZE="$5"
PKG_SYSVER="$6"
PKG_ATTRIBS1="$7"
PKG_ATTRIBS2="$8"
PKG_SHOULDBUILD="$9"

$OO_PS4_TOOLCHAIN/bin/linux/PkgTool.Core sfo_new param.sfo
$OO_PS4_TOOLCHAIN/bin/linux/PkgTool.Core sfo_setentry param.sfo APP_TYPE --type Integer --maxsize 4 --value 1
$OO_PS4_TOOLCHAIN/bin/linux/PkgTool.Core sfo_setentry param.sfo APP_VER --type Utf8 --maxsize 8 --value "$PKG_VERSION"
$OO_PS4_TOOLCHAIN/bin/linux/PkgTool.Core sfo_setentry param.sfo ATTRIBUTE --type Integer --maxsize 4 --value $PKG_ATTRIBS1
if [[ "$PKG_ATTRIBS2" != "0" ]]; then
	$OO_PS4_TOOLCHAIN/bin/linux/PkgTool.Core sfo_setentry param.sfo ATTRIBUTE2 --type Integer --maxsize 4 --value $PKG_ATTRIBS2
fi
$OO_PS4_TOOLCHAIN/bin/linux/PkgTool.Core sfo_setentry param.sfo CATEGORY --type Utf8 --maxsize 4 --value "gd"
$OO_PS4_TOOLCHAIN/bin/linux/PkgTool.Core sfo_setentry param.sfo CONTENT_ID --type Utf8 --maxsize 48 --value "$PKG_CONTENT_ID"
$OO_PS4_TOOLCHAIN/bin/linux/PkgTool.Core sfo_setentry param.sfo DOWNLOAD_DATA_SIZE --type Integer --maxsize 4 --value $PKG_DOWNLOADSIZE
$OO_PS4_TOOLCHAIN/bin/linux/PkgTool.Core sfo_setentry param.sfo SYSTEM_VER --type Integer --maxsize 4 --value $PKG_SYSVER
$OO_PS4_TOOLCHAIN/bin/linux/PkgTool.Core sfo_setentry param.sfo TITLE --type Utf8 --maxsize 128 --value "$PKG_TITLE"
$OO_PS4_TOOLCHAIN/bin/linux/PkgTool.Core sfo_setentry param.sfo TITLE_ID --type Utf8 --maxsize 12 --value "$PKG_TITLE_ID"
$OO_PS4_TOOLCHAIN/bin/linux/PkgTool.Core sfo_setentry param.sfo VERSION --type Utf8 --maxsize 8 --value "$PKG_VERSION"