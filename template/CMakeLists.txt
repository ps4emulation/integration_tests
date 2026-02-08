project(template_test LANGUAGES CXX)

link_libraries(SceSystemService)

set(SRC_FILES
  code/main.cpp
  code/test.cpp
)

create_pkg(TEMT00350 3 50 ${SRC_FILES})
set_target_properties(TEMT00350 PROPERTIES OO_PKG_TITLE "PS4 test template for SDK 3.50")
finalize_pkg(TEMT00350)

create_pkg(TEMT00550 5 50 ${SRC_FILES})
set_target_properties(TEMT00550 PROPERTIES OO_PKG_TITLE "PS4 test template for SDK 5.50")
set_target_properties(TEMT00550 PROPERTIES OO_PKG_APPVER "1.1")
finalize_pkg(TEMT00550)
