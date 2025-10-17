cmake_minimum_required(VERSION 3.24)

set(PKG_TITLE "Insertq stress test")
set(PKG_VERSION "1.0")
set(PKG_TITLE_ID "IQST00001")
set(PKG_CONTENT_ID "IV0000-${PKG_TITLE_ID}_00-PS4SUBSYS0000000")

get_filename_component(CURRENT_FOLDER_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)

if(TARGET ${PKG_TITLE_ID})
	message(FATAL_ERROR "Test name collision detected: ${PKG_TITLE_ID}.")
endif()

project(${PKG_TITLE_ID} VERSION 0.0.1)
set(PRJ_ADD_LIBS SceSystemService)

add_subdirectory(code)

add_executable(${PKG_TITLE_ID}
  $<TARGET_OBJECTS:${PKG_TITLE_ID}_obj>
  ${OO_PS4_TOOLCHAIN}/lib/crt1.o
)

set_target_properties(${PKG_TITLE_ID} PROPERTIES OUTPUT_NAME "eboot" SUFFIX ".elf" PREFIX "")
target_link_libraries(${PKG_TITLE_ID} PRIVATE ${PRJ_ADD_LIBS})
add_dependencies(${PKG_TITLE_ID} CppUTest)

if(CMAKE_HOST_WIN32)
  set(ORBIS_BIN_PATH ${OO_PS4_TOOLCHAIN}/bin/windows)
elseif(CMAKE_HOST_LINUX)
  set(ORBIS_BIN_PATH ${OO_PS4_TOOLCHAIN}/bin/linux)
else()
  message(FATAL_ERROR "Unsupported OS")
endif()

# Create eboot.bin from generated elf file
add_custom_command(TARGET ${PKG_TITLE_ID} POST_BUILD COMMAND
  ${CMAKE_COMMAND} -E env "OO_PS4_TOOLCHAIN=${OO_PS4_TOOLCHAIN}"
  ${ORBIS_BIN_PATH}/create-fself -in "${CMAKE_CURRENT_BINARY_DIR}/eboot.elf"
  --out "${CMAKE_CURRENT_BINARY_DIR}/eboot.oelf" --eboot "${CMAKE_CURRENT_SOURCE_DIR}/eboot.bin" --paid 0x3800000000000011
)

if(OO_PS4_NOPKG)
	set(PKG_SHOULDBUILD "nopkg")
	else()
	set(PKG_SHOULDBUILD "pkg")
endif()

# Create param.sfo and pkg file
if(CMAKE_HOST_WIN32)
  add_custom_command(TARGET ${PKG_TITLE_ID} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E env "OO_PS4_TOOLCHAIN=${OO_PS4_TOOLCHAIN}"
    cmd /c ${INTEST_SOURCE_ROOT}/package.bat
    "${PKG_TITLE}"
    "${PKG_VERSION}"
    "${PKG_TITLE_ID}"
    "${PKG_CONTENT_ID}"
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
    "${PKG_SHOULDBUILD}"
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
    COMMENT "Running package.sh..."
  )
else()
  message(FATAL_ERROR "Unsupported OS")
endif()

# install
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/eboot.bin
  DESTINATION ${CMAKE_INSTALL_PREFIX}/${CURRENT_FOLDER_NAME})

if(NOT OO_PS4_NOPKG)
	install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/${PKG_CONTENT_ID}.pkg
		DESTINATION ${CMAKE_INSTALL_PREFIX}/${CURRENT_FOLDER_NAME})
endif()

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
