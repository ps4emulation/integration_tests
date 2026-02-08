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

  # Create object from generated elf file
  add_custom_command(TARGET ${TargetProject} POST_BUILD COMMAND
    ${CMAKE_COMMAND} -E env "OO_PS4_TOOLCHAIN=${OO_PS4_TOOLCHAIN}"
    ${OO_BINARIES_PATH}/create-fself -in "${WorkDir}/${FSelfName}.elf"
    --out "${WorkDir}/${FSelfName}.oelf" "--${OUT_TYPE}" "${WorkDir}/${FSelfName}.${OUT_EXT}" --paid 0x3800000000000011 --sdkver "${TargetSDKVer}" --fwversion "${OO_FWVER}"
  )

  unset(OUT_TYPE)
  unset(OUT_EXT)
  unset(OO_FWVER)
endfunction()

function(OpenOrbisPackage_PreProject)
  if(TARGET ${PKG_TITLE_ID})
    message(FATAL_ERROR "Test name collision detected: ${PKG_TITLE_ID}.")
  endif()
endfunction()

function(OpenOrbisPackage_PostProject path_bin)
  if(OO_PS4_NOPKG)
    set(PKG_SHOULDBUILD "nopkg")
  else()
    set(PKG_SHOULDBUILD "pkg")
  endif()

  # Create eboot.bin from generated elf file
  OpenOrbis_AddFSelfCommand(${PKG_TITLE_ID} ${path_bin} "eboot" ${PKG_SYSVER})
  get_filename_component(CURRENT_FOLDER_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
  set(install_dir "${CMAKE_INSTALL_PREFIX}/${CURRENT_FOLDER_NAME}/${PKG_TITLE_ID}")

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
  )

  install(FILES ${path_bin}/param.sfo
    DESTINATION "${install_dir}/sce_sys"
  )

  # Create param.sfo
  if(CMAKE_HOST_WIN32)
    string(REPLACE "^" "^^" ESCAPED_PKG_TITLE "${PKG_TITLE}")
    string(REPLACE "&" "^&" ESCAPED_PKG_TITLE "${ESCAPED_PKG_TITLE}")
    string(REPLACE "|" "^|" ESCAPED_PKG_TITLE "${ESCAPED_PKG_TITLE}")
    string(REPLACE "<" "^<" ESCAPED_PKG_TITLE "${ESCAPED_PKG_TITLE}")
    string(REPLACE ">" "^>" ESCAPED_PKG_TITLE "${ESCAPED_PKG_TITLE}")
    string(REPLACE "(" "^(" ESCAPED_PKG_TITLE "${ESCAPED_PKG_TITLE}")
    string(REPLACE ")" "^)" ESCAPED_PKG_TITLE "${ESCAPED_PKG_TITLE}")
    string(REPLACE "%" "%%" ESCAPED_PKG_TITLE "${ESCAPED_PKG_TITLE}")
    string(REPLACE "\"" "^\"" ESCAPED_PKG_TITLE "${ESCAPED_PKG_TITLE}")
    string(REPLACE "!" "^!" ESCAPED_PKG_TITLE "${ESCAPED_PKG_TITLE}")
    string(REPLACE ";" "^;" ESCAPED_PKG_TITLE "${ESCAPED_PKG_TITLE}")
    string(REPLACE "=" "^=" ESCAPED_PKG_TITLE "${ESCAPED_PKG_TITLE}")
    string(REPLACE "\n" "^n" ESCAPED_PKG_TITLE "${ESCAPED_PKG_TITLE}")
    string(REPLACE " " "^ " ESCAPED_PKG_TITLE "${ESCAPED_PKG_TITLE}")

    add_custom_command(TARGET ${PKG_TITLE_ID} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E env "OO_PS4_TOOLCHAIN=${OO_PS4_TOOLCHAIN}"
      cmd /c ${INTEST_SOURCE_ROOT}/patch_param.bat
      "${ESCAPED_PKG_TITLE}"
      "${PKG_VERSION}"
      "${PKG_TITLE_ID}"
      "${PKG_CONTENT_ID}"
      "${PKG_DOWNLOADSIZE}"
      "${PKG_SYSVER}"
      "${PKG_ATTRIBS1}"
      "${PKG_ATTRIBS2}"
      "${PKG_SHOULDBUILD}"
      WORKING_DIRECTORY "${path_bin}"
      COMMENT "Patching param.sfo..."
    )

    if(NOT OO_PS4_NOPKG)
      # create pkg
      install(CODE "
          execute_process(
              COMMAND \"${INTEST_SOURCE_ROOT}/package.bat\" \"${PKG_CONTENT_ID}\" \"${OO_PS4_TOOLCHAIN}\"
              WORKING_DIRECTORY \"${install_dir}\"
          )
          message(STATUS \"Creating GP4 file\")
      ")
    endif()
  elseif(CMAKE_HOST_LINUX)
    string(REPLACE "\t" "\\t" ESCAPED_PKG_TITLE "${PKG_TITLE}")
    string(REPLACE "\"" "\\\"" ESCAPED_PKG_TITLE "${ESCAPED_PKG_TITLE}")
    string(REPLACE "'" "\\'" ESCAPED_PKG_TITLE "${ESCAPED_PKG_TITLE}")
    string(REPLACE "\\" "\\\\" ESCAPED_PKG_TITLE "${ESCAPED_PKG_TITLE}")
    string(REPLACE "|" "\\|" ESCAPED_PKG_TITLE "${ESCAPED_PKG_TITLE}")
    string(REPLACE "&" "\\&" ESCAPED_PKG_TITLE "${ESCAPED_PKG_TITLE}")
    string(REPLACE ";" "\\;" ESCAPED_PKG_TITLE "${ESCAPED_PKG_TITLE}")
    string(REPLACE "(" "\\(" ESCAPED_PKG_TITLE "${ESCAPED_PKG_TITLE}")
    string(REPLACE ")" "\\)" ESCAPED_PKG_TITLE "${ESCAPED_PKG_TITLE}")
    string(REPLACE "<" "\\<" ESCAPED_PKG_TITLE "${ESCAPED_PKG_TITLE}")
    string(REPLACE ">" "\\>" ESCAPED_PKG_TITLE "${ESCAPED_PKG_TITLE}")
    string(REPLACE "*" "\\*" ESCAPED_PKG_TITLE "${ESCAPED_PKG_TITLE}")
    string(REPLACE "?" "\\?" ESCAPED_PKG_TITLE "${ESCAPED_PKG_TITLE}")
    string(REPLACE "[" "\\[" ESCAPED_PKG_TITLE "${ESCAPED_PKG_TITLE}")
    string(REPLACE "]" "\\]" ESCAPED_PKG_TITLE "${ESCAPED_PKG_TITLE}")
    string(REPLACE "{" "\\{" ESCAPED_PKG_TITLE "${ESCAPED_PKG_TITLE}")
    string(REPLACE "}" "\\}" ESCAPED_PKG_TITLE "${ESCAPED_PKG_TITLE}")
    string(REPLACE "#" "\\#" ESCAPED_PKG_TITLE "${ESCAPED_PKG_TITLE}")
    string(REPLACE "!" "\\!" ESCAPED_PKG_TITLE "${ESCAPED_PKG_TITLE}")
    string(REPLACE "$" "\\$" ESCAPED_PKG_TITLE "${ESCAPED_PKG_TITLE}")
    string(REPLACE "`" "\\`" ESCAPED_PKG_TITLE "${ESCAPED_PKG_TITLE}")
    string(REPLACE "~" "\\~" ESCAPED_PKG_TITLE "${ESCAPED_PKG_TITLE}")
    string(REPLACE "\n" "\\n" ESCAPED_PKG_TITLE "${ESCAPED_PKG_TITLE}")

    add_custom_command(TARGET ${PKG_TITLE_ID} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E env "OO_PS4_TOOLCHAIN=${OO_PS4_TOOLCHAIN}"
      "${INTEST_SOURCE_ROOT}/patch_param.sh"
      "${ESCAPED_PKG_TITLE}"
      "${PKG_VERSION}"
      "${PKG_TITLE_ID}"
      "${PKG_CONTENT_ID}"
      "${PKG_DOWNLOADSIZE}"
      "${PKG_SYSVER}"
      "${PKG_ATTRIBS1}"
      "${PKG_ATTRIBS2}"
      "${PKG_SHOULDBUILD}"
      WORKING_DIRECTORY "${path_bin}"
      COMMENT "Patching param.sfo..."
    )

    if(NOT OO_PS4_NOPKG)
      # create pkg
      install(CODE "
          execute_process(
              COMMAND \"${INTEST_SOURCE_ROOT}/package.sh\" \"${PKG_CONTENT_ID}\" \"${OO_PS4_TOOLCHAIN}\"
              WORKING_DIRECTORY \"${install_dir}\"
          )
          message(STATUS \"Creating GP4 file\")
      ")
    endif()
  else()
    message(FATAL_ERROR "Unsupported OS")
  endif()

  unset(install_dir)
endfunction()
