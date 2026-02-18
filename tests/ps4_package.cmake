# description:
# This function creates an OpenOrbis prx library with specified parameters.
#
# params:
# work_lib_name - Name for the library target in CMake project
# fw_version - Firmware version of the library, it is recommended to use the same version as pkg version to avoid linking problems
# pkg_title_id - Package CMake target where to install this library
# inst_path - Relative to project install directory path where to install the library, i.e. "sce_module" to put it in /app0/sce_module
# out_lib_name - Final library name, the one it will be installed with to the `inst_path`
# reuse_existing - Boolean argument, if set to true it will not error on CMake target name collision, already compiled lib will be installed to the specified directory
#
# result:
# This function sets ${prx_first_occur} variable in parent scope to TRUE if library with specified parameters was just built the first time.
# The function always sets this variable to TRUE if reuse_existing set to FALSE.
function(create_lib work_lib_name fw_version pkg_title_id inst_path out_lib_name reuse_existing)
  list(LENGTH ARGN source_count)

  if(source_count LESS 1)
    message(FATAL_ERROR "No source files were provided to the create_lib call")
  endif()

  if(NOT ${reuse_existing} AND TARGET ${work_lib_name})
    message(FATAL_ERROR "Library name collision detected: ${work_lib_name}.")
  endif()

  if(NOT TARGET ${pkg_title_id})
    message(FATAL_ERROR "Specified target (${pkg_title_id}) does not exist, I don't know where to install the library.")
  endif()

  get_target_property(install_dir ${pkg_title_id} OO_PKG_ROOT)

  if(TARGET ${work_lib_name})
    get_target_property(prx_file ${work_lib_name} OO_FSELF_PATH)
    install(FILES ${prx_file}
      DESTINATION "${install_dir}/${inst_path}"
      RENAME "${out_lib_name}"
    )
    set(prx_first_occur FALSE PARENT_SCOPE)
  else()
    add_library(${work_lib_name} SHARED
      ${OO_PS4_TOOLCHAIN}/lib/crtlib.o
      ${ARGN}
    )
    OpenOrbis_AddFSelfCommand(${work_lib_name} ${CMAKE_CURRENT_BINARY_DIR} ${work_lib_name} ${fw_version})

    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${work_lib_name}.prx
      DESTINATION "${install_dir}/${inst_path}"
      RENAME "${out_lib_name}"
    )
    set(prx_first_occur TRUE PARENT_SCOPE)
  endif()
endfunction()

function(internal_create_stub_libs pkg_title_id fw_version)
  # Generate libc.prx stub
  create_lib("c${fw_version}" ${fw_version} ${pkg_title_id} "sce_module" "libc.prx" TRUE "${OO_PS4_TOOLCHAIN}/src/modules/libc/libc/lib.c")

  # Generate libSceFios2.prx
  create_lib("fios${fw_version}" ${fw_version} ${pkg_title_id} "sce_module" "libSceFios2.prx" TRUE "${OO_PS4_TOOLCHAIN}/src/modules/libSceFios2/libSceFios2/lib.c")

  # Generate right.sprx
  create_lib("right${fw_version}" ${fw_version} ${pkg_title_id} "sce_sys/about" "right.sprx" TRUE "${OO_PS4_TOOLCHAIN}/src/modules/right/right/lib.c")

  if(${prx_first_occur}) # No need to re-set this option every time
    target_link_options("right${fw_version}" PRIVATE "-Wl,--version-script=${OO_PS4_TOOLCHAIN}/src/modules/right/right/right.version")
  endif()
endfunction()

function(create_pkg title_id fw_major fw_minor)
  list(LENGTH ARGN source_count)

  if(source_count LESS 1)
    message(FATAL_ERROR "No source files were provided to the create_pkg call")
  endif()

  set(FW_MAJOR_PADDED 0)
  set(FW_MINOR_PADDED 0)

  # Pad FW_MAJOR to 2 digits.
  string(LENGTH ${fw_major} FW_MAJOR_STRLEN)

  if(${FW_MAJOR_STRLEN} EQUAL 1)
    set(FW_MAJOR_PADDED "0${fw_major}")
  elseif(${FW_MAJOR_STRLEN} EQUAL 2)
    set(FW_MAJOR_PADDED "${fw_major}")
  else()
    message(FATAL_ERROR "create_pkg: major SDK version either empty or too long")
  endif()

  # Pad FW_MINOR to 2 digits
  string(LENGTH ${fw_minor} FW_MINOR_STRLEN)

  if(${FW_MINOR_STRLEN} EQUAL 1)
    set(FW_MINOR_PADDED "0${fw_minor}")
  elseif(${FW_MINOR_STRLEN} EQUAL 2)
    set(FW_MINOR_PADDED "${fw_minor}")
  else()
    message(FATAL_ERROR "create_pkg: minor SDK version either empty or too long")
  endif()

  # SDK version cheat sheet:
  # MMmmmppp
  # M - Major
  # m - Minor
  # p - Patch
  # We do not set the third nibble of minor version since we don't really
  # need it for SDK version specification. This variable should be changed
  # if we actually need third nibble of the the minor component of the
  # version in the future. The condition for checking minor version length
  # should be adjusted accordingly too.
  math(EXPR fw_version_hex "0x${FW_MAJOR_PADDED}${FW_MINOR_PADDED}0000")

  # Print debug info if needed
  message(STATUS "Creating package id:${title_id} fw:${fw_version_hex}")

  add_executable(${title_id}
    ${OO_PS4_TOOLCHAIN}/lib/crt1.o
    ${ARGN}
  )

  string(SUBSTRING "${title_id}" 0 4 default_title)
  set_target_properties(${title_id} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${title_id}"
    OO_PKG_TITLE "PS4 ${default_title} (FW ${fw_major}.${fw_minor})"
    OO_PKG_CONTENTID "IV0000-${title_id}_00-PS4INTEST0000000"
    OO_PKG_APPVER "1.0"
    OO_PKG_DOWNSIZE 0x100
    OO_PKG_ATTRIBS1 0
    OO_PKG_ATTRIBS2 0
  )

  target_compile_definitions(${title_id}
    PRIVATE FW_VER_MAJOR=${fw_major} FW_VER_MINOR=${fw_minor} FW_VER="${fw_version_hex}u"
  )

  target_link_options(${title_id} PRIVATE -pie)

  add_dependencies(${title_id} CppUTest)

  OpenOrbisPackage_PostProject(${title_id} ${fw_version_hex})

  internal_create_stub_libs(${title_id} ${fw_version_hex})
endfunction(create_pkg)

function(finalize_pkg pkg_title_id)
  OpenOrbisPackage_FinalizeProject(${pkg_title_id})
endfunction(finalize_pkg)
