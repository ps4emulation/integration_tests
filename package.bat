@echo off
SETLOCAL EnableDelayedExpansion

set PKG_CONTENT_ID=%1

Rem Get a list of assets for packaging
set module_files=
for %%f in (sce_module\\*) do set module_files=!module_files! sce_module/%%~nxf

set asset_audio_files=
for %%f in (assets\\audio\\*) do set asset_audio_files=!asset_audio_files! assets/audio/%%~nxf

set asset_fonts_files=
for %%f in (assets\\fonts\\*) do set asset_fonts_files=!asset_fonts_files! assets/fonts/%%~nxf

set asset_images_files=
for %%f in (assets\\images\\*) do set asset_images_files=!asset_images_files! assets/images/%%~nxf

set asset_misc_files=
for %%f in (assets\\misc\\*) do set asset_misc_files=!asset_misc_files! assets/misc/%%~nxf

set asset_videos_files=
for %%f in (assets\\videos\\*) do set asset_videos_files=!asset_videos_files! assets/videos/%%~nxf

Rem Create gp4
%OO_PS4_TOOLCHAIN%\bin\windows\create-gp4.exe -out pkg.gp4 --content-id=%PKG_CONTENT_ID% --files "eboot.bin sce_sys/about/right.sprx sce_sys/param.sfo sce_sys/icon0.png %module_files% %asset_audio_files% %asset_fonts_files% %asset_images_files% %asset_misc_files% %asset_videos_files%"

Rem Create pkg
%OO_PS4_TOOLCHAIN%\bin\windows\PkgTool.Core.exe pkg_build pkg.gp4 .
