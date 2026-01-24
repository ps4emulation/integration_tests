function(create_pkg pkg_title_id fw_major fw_minor src_files)
  # Create fw version
  set(fw_major_hex "0x${fw_major}")
  set(fw_minor_hex "0x${fw_minor}")

  math(EXPR FW_VERSION_INT "(${fw_major_hex} << 24) | ${fw_minor_hex} << 16")

  execute_process(
    COMMAND printf "0x%08X" ${FW_VERSION_INT}
    OUTPUT_VARIABLE FW_VERSION_INT_HEX
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )

  # Set variables for the package
  string(SUBSTRING "${pkg_title_id}" 0 4 title)
  set(PKG_TITLE "PS4 ${title} (FW ${fw_major}.${fw_minor})")

  set(PKG_TITLE_ID "${pkg_title_id}")
  set(PKG_VERSION "1.0")
  set(PKG_CONTENT_ID "IV0000-${pkg_title_id}_00-PS4SUBSYS0000000")
  set(PKG_DOWNLOADSIZE 0x100)
  set(PKG_SYSVER ${FW_VERSION_INT_HEX})
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
