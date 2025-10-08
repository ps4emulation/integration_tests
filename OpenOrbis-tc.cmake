message(STATUS "tt ${OO_PS4_TOOLCHAIN}")

if(NOT DEFINED ENV{OO_PS4_TOOLCHAIN})
  if(DEFINED OO_PS4_TOOLCHAIN)
    set(ENV{OO_PS4_TOOLCHAIN} ${OO_PS4_TOOLCHAIN})
  else()
    message(FATAL_ERROR "Missing OpenOrbis toolchain environment variable!")
  endif()
endif()

STRING(REGEX REPLACE "\\\\" "/" OO_PS4_TOOLCHAIN "$ENV{OO_PS4_TOOLCHAIN}")

message(STATUS "weasd ${OO_PS4_TOOLCHAIN}")
set(CMAKE_SYSTEM_NAME FreeBSD CACHE STRING "" FORCE)
set(CMAKE_C_COMPILER_TARGET "x86_64-pc-freebsd12-elf" CACHE STRING "" FORCE)
set(CMAKE_CXX_COMPILER_TARGET "${CMAKE_C_COMPILER_TARGET}" CACHE STRING "" FORCE)
set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY" CACHE STRING "" FORCE)
set(CMAKE_SYSROOT "${OO_PS4_TOOLCHAIN}" CACHE PATH "" FORCE)

message(STATUS "SYSROOT ${CMAKE_SYSROOT}")

include_directories(SYSTEM
  ${OO_PS4_TOOLCHAIN}/include
  ${OO_PS4_TOOLCHAIN}/include/c++/v1
)

link_directories(BEFORE
  ${OO_PS4_TOOLCHAIN}/lib
)

add_link_options(-pie -nostartfiles -nodefaultlibs -lc -lc++ -lkernel -fuse-ld=lld -Wl,-m,elf_x86_64 -Wl,--eh-frame-hdr "-Wl,--script,${OO_PS4_TOOLCHAIN}/link.x")

add_compile_options(-nostdinc++ -nostdinc -fPIC -funwind-tables -fshort-wchar)

add_compile_definitions(STBI_NO_SIMD=1)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_compile_options(-O0 -g)
else()
  add_compile_options(-O3)
endif()
