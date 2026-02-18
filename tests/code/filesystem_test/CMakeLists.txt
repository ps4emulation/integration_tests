project(Therapist VERSION 0.0.1)

link_libraries(SceSystemService)

set(SRC_FILES
  code/log.h
  code/fs_test.h
  code/main.cpp
  code/log.cpp
  code/fs_test_tools.cpp
  code/fs_test.cpp
)

create_pkg(TEST21370 5 50 ${SRC_FILES})
set_target_properties(TEST21370 PROPERTIES OO_PKG_TITLE "Therapist")
set_target_properties(TEST21370 PROPERTIES OO_PKG_APPVER "1.32")
finalize_pkg(TEST21370)
