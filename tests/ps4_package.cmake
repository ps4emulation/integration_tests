function(create_pkg pkg_title_id fw_major fw_minor src_files)
  # Pad FW_MINOR to 2 digits
  if(${fw_minor} LESS 10)
    set(FW_MINOR_PADDED "0${fw_minor}")
  else()
    set(FW_MINOR_PADDED "${fw_minor}")
  endif()

  set(FW_VERSION_HEX_STR "0x${fw_major}${FW_MINOR_PADDED}00000")
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
  message(STATUS "Creating package ${PKG_TITLE} with id: ${PKG_TITLE_ID}")

  OpenOrbisPackage_PreProject()

  add_executable(${pkg_title_id}
    ${src_files}
    ${OO_PS4_TOOLCHAIN}/lib/crt1.o
  )

  set_target_properties(${pkg_title_id} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${pkg_title_id}
  )

  add_dependencies(${pkg_title_id} CppUTest)

  OpenOrbisPackage_PostProject()
endfunction()
# Example usage:
# create_pkg("MEMT00550" 0x5500000)
