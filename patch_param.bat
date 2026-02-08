@echo off
SETLOCAL EnableDelayedExpansion

if "%OO_PS4_TOOLCHAIN%"=="" (
	echo "Toolchain is not set"
	exit /b 1
)

Rem Package information
set PKG_TITLE=%1
set PKG_VERSION="%2"
set PKG_TITLE_ID="%3"
set PKG_CONTENT_ID="%4"
set PKG_DOWNLOADSIZE="%5"
set PKG_SYSVER="%6"
set PKG_ATTRIBS1="%7"
set PKG_ATTRIBS2="%8"
set PKG_SHOULDBUILD="%9"

Rem Create param.sfo
%OO_PS4_TOOLCHAIN%\bin\windows\PkgTool.Core.exe sfo_new param.sfo
%OO_PS4_TOOLCHAIN%\bin\windows\PkgTool.Core.exe sfo_setentry param.sfo APP_TYPE --type Integer --maxsize 4 --value 1
%OO_PS4_TOOLCHAIN%\bin\windows\PkgTool.Core.exe sfo_setentry param.sfo APP_VER --type Utf8 --maxsize 8 --value %PKG_VERSION%
%OO_PS4_TOOLCHAIN%\bin\windows\PkgTool.Core.exe sfo_setentry param.sfo ATTRIBUTE --type Integer --maxsize 4 --value %PKG_ATTRIBS1%
if NOT %PKG_ATTRIBS2%=="0" %OO_PS4_TOOLCHAIN%\bin\windows\PkgTool.Core.exe sfo_setentry param.sfo ATTRIBUTE2 --type Integer --maxsize 4 --value %PKG_ATTRIBS2%
%OO_PS4_TOOLCHAIN%\bin\windows\PkgTool.Core.exe sfo_setentry param.sfo CATEGORY --type Utf8 --maxsize 4 --value "gd"
%OO_PS4_TOOLCHAIN%\bin\windows\PkgTool.Core.exe sfo_setentry param.sfo CONTENT_ID --type Utf8 --maxsize 48 --value %PKG_CONTENT_ID%
%OO_PS4_TOOLCHAIN%\bin\windows\PkgTool.Core.exe sfo_setentry param.sfo DOWNLOAD_DATA_SIZE --type Integer --maxsize 4 --value %PKG_DOWNLOADSIZE%
%OO_PS4_TOOLCHAIN%\bin\windows\PkgTool.Core.exe sfo_setentry param.sfo SYSTEM_VER --type Integer --maxsize 4 --value %PKG_SYSVER%
%OO_PS4_TOOLCHAIN%\bin\windows\PkgTool.Core.exe sfo_setentry param.sfo TITLE --type Utf8 --maxsize 128 --value %PKG_TITLE%
%OO_PS4_TOOLCHAIN%\bin\windows\PkgTool.Core.exe sfo_setentry param.sfo TITLE_ID --type Utf8 --maxsize 12 --value %PKG_TITLE_ID%
%OO_PS4_TOOLCHAIN%\bin\windows\PkgTool.Core.exe sfo_setentry param.sfo VERSION --type Utf8 --maxsize 8 --value "1.0"
