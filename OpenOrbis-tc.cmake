if(NOT DEFINED ENV{OO_PS4_TOOLCHAIN})
  if(DEFINED OO_PS4_TOOLCHAIN)
    set(ENV{OO_PS4_TOOLCHAIN} ${OO_PS4_TOOLCHAIN})
  else()
    # # Get toolchain before project
    message(STATUS "Downloading openOrbis")
    FetchContent_Declare(
      openorbis
      URL https://github.com/ps4emulation/OpenOrbis-PS4-Toolchain/releases/download/0.5.4-4/v0.5.4-4.tar.gz
      URL_HASH SHA256=53fcaa7d25c200738968bb55b9a99ac03718c38713efd7261845ab2c45641b18 # optional, hash of zip
    )

    FetchContent_MakeAvailable(openorbis)

    set(OO_PS4_TOOLCHAIN ${openorbis_SOURCE_DIR}/PS4Toolchain)
    set(ENV{OO_PS4_TOOLCHAIN} ${OO_PS4_TOOLCHAIN})
  endif()
endif()

STRING(REGEX REPLACE "\\\\" "/" OO_PS4_TOOLCHAIN "$ENV{OO_PS4_TOOLCHAIN}")

define_property(TARGET PROPERTY OO_FSELF_PATH BRIEF_DOCS "This property reports a ready-to use binary, should never be edited manually")
define_property(TARGET PROPERTY OO_PKG_ROOT BRIEF_DOCS "This property reports a path where pkg file will be created, should never be edited manually")
define_property(TARGET PROPERTY OO_PKG_FINALIZED BRIEF_DOCS "This property reports whether package was finalized or not, should never be edited manually")
define_property(TARGET PROPERTY OO_PKG_TITLE BRIEF_DOCS "This property specifies TITLE for generated param.sfo file, can be edited between create and finalize blocks")
define_property(TARGET PROPERTY OO_PKG_APPVER BRIEF_DOCS "This property specifies APPVER for generated param.sfo file, can be edited between create and finalize blocks")
define_property(TARGET PROPERTY OO_PKG_CONTENTID BRIEF_DOCS "This property specifies CONTENT_ID for generated param.sfo file, can be edited between create and finalize blocks")
define_property(TARGET PROPERTY OO_PKG_DOWNSIZE BRIEF_DOCS "This property specifies DOWNLOAD_DATA_SIZE for generated param.sfo file, can be edited between create and finalize blocks")
define_property(TARGET PROPERTY OO_PKG_ATTRIBS1 BRIEF_DOCS "This property specifies ATTRIBUTE for generated param.sfo file, can be edited between create and finalize blocks")
define_property(TARGET PROPERTY OO_PKG_ATTRIBS2 BRIEF_DOCS "This property specifies ATTRIBUTE2 for generated param.sfo file, can be edited between create and finalize blocks")

set(CMAKE_SYSTEM_NAME FreeBSD CACHE STRING "" FORCE)
set(CMAKE_C_COMPILER_TARGET "x86_64-pc-freebsd12-elf" CACHE STRING "" FORCE)
set(CMAKE_CXX_COMPILER_TARGET "${CMAKE_C_COMPILER_TARGET}" CACHE STRING "" FORCE)
set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY" CACHE STRING "" FORCE)
set(CMAKE_SYSROOT "${OO_PS4_TOOLCHAIN}" CACHE PATH "" FORCE)

if(CMAKE_HOST_WIN32)
  set(OO_BINARIES_PATH ${OO_PS4_TOOLCHAIN}/bin/windows)
elseif(CMAKE_HOST_LINUX)
  set(OO_BINARIES_PATH ${OO_PS4_TOOLCHAIN}/bin/linux)
else()
  message(FATAL_ERROR "Unsupported OS")
endif()

include_directories(SYSTEM
  ${OO_PS4_TOOLCHAIN}/include
  ${OO_PS4_TOOLCHAIN}/include/c++/v1
)

link_directories(BEFORE
  ${OO_PS4_TOOLCHAIN}/lib
)

add_link_options(-nostartfiles -nodefaultlibs -lc -lc++ -lkernel -fuse-ld=lld${OO_PS4_LINKER_SUFFIX} -Wl,-m,elf_x86_64 -Wl,--eh-frame-hdr "-Wl,--script,${OO_PS4_TOOLCHAIN}/link.x")

add_compile_options(-nostdinc++ -nostdinc -fPIC -funwind-tables -fshort-wchar)

add_compile_definitions(STBI_NO_SIMD=1)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_compile_options(-O0 -g)
else()
  add_compile_options(-O3)
endif()

function(OpenOrbis_AddFSelfCommand TargetProject WorkDir FSelfName TargetSDKVer)
  set_target_properties(${TargetProject} PROPERTIES OUTPUT_NAME "${FSelfName}" SUFFIX ".elf" PREFIX "")

  set(OUT_TYPE "eboot")
  set(OUT_EXT "bin")

  if(NOT ${FSelfName} STREQUAL "eboot")
    set(OUT_TYPE "lib")
    set(OUT_EXT "prx")
  endif()

  math(EXPR OO_FWVER "${TargetSDKVer} * 65536")
  set(OUT_ABS_PATH "${WorkDir}/${FSelfName}.${OUT_EXT}")

  # Create object from generated elf file
  add_custom_command(TARGET ${TargetProject} POST_BUILD COMMAND
    ${CMAKE_COMMAND} -E env "OO_PS4_TOOLCHAIN=${OO_PS4_TOOLCHAIN}"
    ${OO_BINARIES_PATH}/create-fself -in "${WorkDir}/${FSelfName}.elf"
    --out "${WorkDir}/${FSelfName}.oelf" "--${OUT_TYPE}" "${OUT_ABS_PATH}" --paid 0x3800000000000011 --sdkver "${TargetSDKVer}" --fwversion "${OO_FWVER}"
  )

  set_target_properties(${TargetProject} PROPERTIES OO_FSELF_PATH "${OUT_ABS_PATH}")
endfunction()

function(OpenOrbisPackage_PostProject pkg_title_id pkg_fw_version_hex)
  get_target_property(path_bin ${pkg_title_id} RUNTIME_OUTPUT_DIRECTORY)

  # Create eboot.bin from generated elf file
  OpenOrbis_AddFSelfCommand(${pkg_title_id} ${path_bin} "eboot" ${pkg_fw_version_hex})
  get_filename_component(curr_folder ${CMAKE_CURRENT_SOURCE_DIR} NAME)
  set(install_dir "${CMAKE_INSTALL_PREFIX}/${curr_folder}/${pkg_title_id}")
  set_target_properties(${pkg_title_id} PROPERTIES
    OO_PKG_ROOT "${install_dir}"
    OO_PKG_FINALIZED FALSE
  )

  # install
  install(FILES ${path_bin}/eboot.bin
    DESTINATION "${install_dir}"
  )

  install(DIRECTORY
    ${CMAKE_SOURCE_DIR}/app_data/
    DESTINATION "${install_dir}"
  )

  install(DIRECTORY
    ${CMAKE_CURRENT_SOURCE_DIR}/assets
    DESTINATION "${install_dir}"
    OPTIONAL
  )
endfunction()

function(OpenOrbisPackage_FinalizeProject pkg_title_id)
  set(BASE_SCRIPT [=[
    # Create param.sfo
    if(CMAKE_HOST_WIN32)
      set(PatchParamSfoCommand "${prj_root}/patch_param.bat")
      set(PackageCommand "${prj_root}/package.bat")
    elseif(CMAKE_HOST_LINUX)
      set(PatchParamSfoCommand "${prj_root}/patch_param.sh")
      set(PackageCommand "${prj_root}/package.sh")
    else()
      message(FATAL_ERROR "Unsupported OS, can't create param.sfo")
    endif()

    message(STATUS "Creating param.sfo file")
    execute_process(
      COMMAND "${PatchParamSfoCommand}"
      "${pkg_title}"
      "${pkg_appver}"
      "${pkg_title_id}"
      "${pkg_content_id}"
      "${pkg_downsize}"
      "${pkg_fw_version_hex}"
      "${pkg_attribs1}"
      "${pkg_attribs2}"
      "${ShouldBuildArg}"
      WORKING_DIRECTORY "${pkg_root}/sce_sys"
    )

    if(NOT OO_PS4_NOPKG)
      message(STATUS "Creating GP4 file")

      # create pkg
      execute_process(
        COMMAND "${PackageCommand}" "${pkg_content_id}" "${OO_PS4_TOOLCHAIN}"
        WORKING_DIRECTORY "${pkg_root}"
      )
    endif()
  ]=])

  if(OO_PS4_NOPKG)
    set(ShouldBuildArg "nopkg")
  else()
    set(ShouldBuildArg "pkg")
  endif()

  get_target_property(pkg_root ${pkg_title_id} OO_PKG_ROOT)
  get_target_property(pkg_title ${pkg_title_id} OO_PKG_TITLE)
  get_target_property(pkg_content_id ${pkg_title_id} OO_PKG_CONTENTID)
  get_target_property(pkg_appver ${pkg_title_id} OO_PKG_APPVER)
  get_target_property(pkg_attribs1 ${pkg_title_id} OO_PKG_ATTRIBS1)
  get_target_property(pkg_attribs2 ${pkg_title_id} OO_PKG_ATTRIBS2)
  get_target_property(pkg_downsize ${pkg_title_id} OO_PKG_DOWNSIZE)
  string(CONFIGURE [=[
    set(prj_root "${INTEST_SOURCE_ROOT}")
    set(pkg_root "${pkg_root}")
    set(pkg_title "${pkg_title}")
    set(pkg_title_id "${pkg_title_id}")
    set(pkg_content_id "${pkg_content_id}")
    set(pkg_appver "${pkg_appver}")
    set(pkg_attribs1 "${pkg_attribs1}")
    set(pkg_attribs2 "${pkg_attribs2}")
    set(pkg_downsize "${pkg_downsize}")
    set(ShouldBuildArg "${ShouldBuildArg}")
    ${BASE_SCRIPT}
  ]=] install_code)

  install(CODE "${install_code}")

  # message(FATAL_ERROR ${install_code})
endfunction()
