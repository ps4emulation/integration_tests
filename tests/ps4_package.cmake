function(create_lib work_dir work_lib_name src_files fw_version inst_path out_lib_name)
  add_library(${work_lib_name} SHARED ${src_files} ${OO_PS4_TOOLCHAIN}/lib/crtlib.o)
  OpenOrbis_AddFSelfCommand(${work_lib_name} ${work_dir} ${work_lib_name} ${fw_version})
  install(FILES ${work_dir}/${work_lib_name}.prx
    DESTINATION "${inst_path}"
    RENAME "${out_lib_name}"
  )
endfunction()

function(internal_create_stub_libs out_dir fw_version)
  get_filename_component(curr_folder ${CMAKE_CURRENT_SOURCE_DIR} NAME)
  set(install_dir "${CMAKE_INSTALL_PREFIX}/${curr_folder}/${PKG_TITLE_ID}")

  # Generate libc.prx stub
  if(NOT TARGET "c${fw_version}")
    create_lib(${out_dir} "c${fw_version}" "${OO_PS4_TOOLCHAIN}/src/modules/libc/libc/lib.c" ${fw_version} "${install_dir}/sce_module" "libc.prx")
  endif()

  # Generate libSceFios2.prx
  if(NOT TARGET "fios${fw_version}")
    create_lib(${out_dir} "fios${fw_version}" "${OO_PS4_TOOLCHAIN}/src/modules/libSceFios2/libSceFios2/lib.c" ${fw_version} "${install_dir}/sce_module" "libSceFios2.prx")
  endif()

  # Generate right.sprx
  if(NOT TARGET "right${fw_version}")
    create_lib(${out_dir} "right${fw_version}" "${OO_PS4_TOOLCHAIN}/src/modules/right/right/lib.c" ${fw_version} "${install_dir}/sce_sys/about" "right.sprx")
    target_link_options("right${fw_version}" PRIVATE "-Wl,--version-script=${OO_PS4_TOOLCHAIN}/src/modules/right/right/right.version")
  endif()
endfunction()

function(create_pkg pkg_title_id fw_major fw_minor src_files)
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
  math(EXPR FW_VERSION_HEX "0x${FW_MAJOR_PADDED}${FW_MINOR_PADDED}0000")

  # Set variables for the package
  string(SUBSTRING "${pkg_title_id}" 0 4 title)
  set(PKG_TITLE "PS4 ${title} (FW ${fw_major}.${fw_minor})")

  set(PKG_TITLE_ID "${pkg_title_id}")
  set(PKG_VERSION "1.0")
  set(PKG_CONTENT_ID "IV0000-${pkg_title_id}_00-PS4SUBSYS0000000")
  set(PKG_DOWNLOADSIZE 0x100)
  set(PKG_SYSVER ${FW_VERSION_HEX})
  set(PKG_ATTRIBS1 0)
  set(PKG_ATTRIBS2 0)

  # Print debug info if needed
  message(STATUS "Creating package ${PKG_TITLE} id:${PKG_TITLE_ID} fw:${PKG_SYSVER}")

  OpenOrbisPackage_PreProject()

  add_executable(${pkg_title_id}
    ${src_files}
    ${OO_PS4_TOOLCHAIN}/lib/crt1.o
  )

  set(out_dir "${CMAKE_CURRENT_BINARY_DIR}/${pkg_title_id}")
  set_target_properties(${pkg_title_id} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${out_dir}
  )

  target_compile_definitions(${pkg_title_id}
    PRIVATE FW_VER_MAJOR=${fw_major} FW_VER_MINOR=${fw_minor} FW_VER="${PKG_SYSVER}u"
  )

  target_link_options(${pkg_title_id} PRIVATE -pie)

  add_dependencies(${pkg_title_id} CppUTest)

  internal_create_stub_libs(${CMAKE_CURRENT_BINARY_DIR} ${PKG_SYSVER})

  OpenOrbisPackage_PostProject(${out_dir})
endfunction(create_pkg)
