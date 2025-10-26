cmake_minimum_required(VERSION 3.24)

set(PKG_TITLE "PS4 Unit Test (FW 1.00)")
set(PKG_VERSION "1.0")
set(PKG_TITLE_ID "MEMT00001")
set(PKG_CONTENT_ID "IV0000-${PKG_TITLE_ID}_00-PS4SUBSYS0000000")
set(PKG_DOWNLOADSIZE 256)
set(PKG_SYSVER 0x1000051)
set(PKG_ATTRIBS1 0)
set(PKG_ATTRIBS2 0)

OpenOrbisPackage_PreProject()

project(${PKG_TITLE_ID} VERSION 0.0.1)

add_subdirectory(code)

add_executable(${PKG_TITLE_ID}
  $<TARGET_OBJECTS:${PKG_TITLE_ID}_obj>
  ${OO_PS4_TOOLCHAIN}/lib/crt1.o
)

target_link_libraries(${PKG_TITLE_ID} PRIVATE SceSystemService)
add_dependencies(${PKG_TITLE_ID} CppUTest)

OpenOrbisPackage_PostProject()
