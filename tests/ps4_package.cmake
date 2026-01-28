function(create_pkg pkg_title_id fw_major fw_minor src_files)
  set(FW_MAJOR_PADDED 0 PARENT_SCOPE)
  set(FW_MINOR_PADDED 0 PARENT_SCOPE)

  # Pad FW_MAJOR to 2 digits.
  string(LENGTH ${fw_major} FW_MAJOR_STRLEN)

  if(${FW_MAJOR_STRLEN} LESS 2)
    set(FW_MAJOR_PADDED "0${fw_major}")
  else()
    set(FW_MAJOR_PADDED "${fw_major}")
  endif()

  # Pad FW_MINOR to 2 digits
  string(LENGTH ${fw_minor} FW_MINOR_STRLEN)

  if(${FW_MINOR_STRLEN} LESS 2)
    set(FW_MINOR_PADDED "0${fw_minor}")
  else()
    set(FW_MINOR_PADDED "${fw_minor}")
  endif()

  set(FW_VERSION_HEX_STR "0x${FW_MAJOR_PADDED}${FW_MINOR_PADDED}0000")
  math(EXPR FW_VERSION_HEX "${FW_VERSION_HEX_STR}")

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

  add_custom_command(TARGET ${pkg_title_id} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_SOURCE_DIR}/app_data/param.sfo
    $<TARGET_FILE_DIR:${pkg_title_id}>/param.sfo
  )

  target_compile_definitions(${pkg_title_id}
    PRIVATE FW_VER_MAJOR=${fw_major} FW_VER_MINOR=${fw_minor}
  )

  add_dependencies(${pkg_title_id} CppUTest)

  OpenOrbisPackage_PostProject(${out_dir})
endfunction(create_pkg)
