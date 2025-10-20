if(NOT DEFINED ENV{OO_PS4_TOOLCHAIN})
  if(DEFINED OO_PS4_TOOLCHAIN)
    set(ENV{OO_PS4_TOOLCHAIN} ${OO_PS4_TOOLCHAIN})
  else()
    message(FATAL_ERROR "Missing OpenOrbis toolchain environment variable!")
  endif()
endif()

STRING(REGEX REPLACE "\\\\" "/" OO_PS4_TOOLCHAIN "$ENV{OO_PS4_TOOLCHAIN}")

set(CMAKE_SYSTEM_NAME FreeBSD CACHE STRING "" FORCE)
set(CMAKE_C_COMPILER_TARGET "x86_64-pc-freebsd12-elf" CACHE STRING "" FORCE)
set(CMAKE_CXX_COMPILER_TARGET "${CMAKE_C_COMPILER_TARGET}" CACHE STRING "" FORCE)
set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY" CACHE STRING "" FORCE)
set(CMAKE_SYSROOT "${OO_PS4_TOOLCHAIN}" CACHE PATH "" FORCE)

include_directories(SYSTEM
  ${OO_PS4_TOOLCHAIN}/include
  ${OO_PS4_TOOLCHAIN}/include/c++/v1
)

link_directories(BEFORE
  ${OO_PS4_TOOLCHAIN}/lib
)

add_link_options(-pie -nostartfiles -nodefaultlibs -lc -lc++ -lkernel -fuse-ld=lld${OO_PS4_LINKER_SUFFIX} -Wl,-m,elf_x86_64 -Wl,--eh-frame-hdr "-Wl,--script,${OO_PS4_TOOLCHAIN}/link.x")

add_compile_options(-nostdinc++ -nostdinc -fPIC -funwind-tables -fshort-wchar)

add_compile_definitions(STBI_NO_SIMD=1)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_compile_options(-O0 -g)
else()
  add_compile_options(-O3)
endif()

function(OpenOrbisPackage_PreProject)
	if(TARGET ${PKG_TITLE_ID})
		message(FATAL_ERROR "Test name collision detected: ${PKG_TITLE_ID}.")
	endif()
endfunction()

function(OpenOrbisPackage_PostProject)
	set_target_properties(${PKG_TITLE_ID} PROPERTIES OUTPUT_NAME "eboot" SUFFIX ".elf" PREFIX "")

	if(CMAKE_HOST_WIN32)
		set(ORBIS_BIN_PATH ${OO_PS4_TOOLCHAIN}/bin/windows)
	elseif(CMAKE_HOST_LINUX)
		set(ORBIS_BIN_PATH ${OO_PS4_TOOLCHAIN}/bin/linux)
	else()
		message(FATAL_ERROR "Unsupported OS")
	endif()

  if(OO_PS4_NOPKG)
    set(PKG_SHOULDBUILD "nopkg")
  else()
    set(PKG_SHOULDBUILD "pkg")
  endif()

	# Create eboot.bin from generated elf file
	add_custom_command(TARGET ${PKG_TITLE_ID} POST_BUILD COMMAND
		${CMAKE_COMMAND} -E env "OO_PS4_TOOLCHAIN=${OO_PS4_TOOLCHAIN}"
		${ORBIS_BIN_PATH}/create-fself -in "${CMAKE_CURRENT_BINARY_DIR}/eboot.elf"
		--out "${CMAKE_CURRENT_BINARY_DIR}/eboot.oelf" --eboot "${CMAKE_CURRENT_SOURCE_DIR}/eboot.bin" --paid 0x3800000000000011
	)

	# Create param.sfo and pkg file
	if(CMAKE_HOST_WIN32)
		add_custom_command(TARGET ${PKG_TITLE_ID} POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E env "OO_PS4_TOOLCHAIN=${OO_PS4_TOOLCHAIN}"
			cmd /c ${INTEST_SOURCE_ROOT}/package.bat
			"${PKG_TITLE}"
			"${PKG_VERSION}"
			"${PKG_TITLE_ID}"
			"${PKG_CONTENT_ID}"
			"${PKG_DOWNLOADSIZE}"
			"${PKG_SYSVER}"
			"${PKG_ATTRIBS1}"
			"${PKG_ATTRIBS2}"
			"${PKG_SHOULDBUILD}"
			WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
			COMMENT "Running package.bat..."
		)
	elseif(CMAKE_HOST_LINUX)
		add_custom_command(TARGET ${PKG_TITLE_ID} POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E env "OO_PS4_TOOLCHAIN=${OO_PS4_TOOLCHAIN}"
			"${INTEST_SOURCE_ROOT}/package.sh"
			"${PKG_TITLE}"
			"${PKG_VERSION}"
			"${PKG_TITLE_ID}"
			"${PKG_CONTENT_ID}"
			"${PKG_DOWNLOADSIZE}"
			"${PKG_SYSVER}"
			"${PKG_ATTRIBS1}"
			"${PKG_ATTRIBS2}"
			"${PKG_SHOULDBUILD}"
			WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
			COMMENT "Running package.sh..."
		)
	else()
		message(FATAL_ERROR "Unsupported OS")
	endif()

	get_filename_component(CURRENT_FOLDER_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)

	if(NOT OO_PS4_NOPKG)
		install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/${PKG_CONTENT_ID}.pkg
			DESTINATION ${CMAKE_INSTALL_PREFIX}/${CURRENT_FOLDER_NAME})
	endif()

	# install
	install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/eboot.bin
		DESTINATION ${CMAKE_INSTALL_PREFIX}/${CURRENT_FOLDER_NAME})

	install(DIRECTORY
		${CMAKE_CURRENT_SOURCE_DIR}/sce_sys
		DESTINATION ${CMAKE_INSTALL_PREFIX}/${CURRENT_FOLDER_NAME}
	)

	install(DIRECTORY
		${CMAKE_CURRENT_SOURCE_DIR}/sce_module
		DESTINATION ${CMAKE_INSTALL_PREFIX}/${CURRENT_FOLDER_NAME}
	)

	install(DIRECTORY
		${CMAKE_CURRENT_SOURCE_DIR}/assets
		DESTINATION ${CMAKE_INSTALL_PREFIX}/${CURRENT_FOLDER_NAME}
	)
endfunction()
