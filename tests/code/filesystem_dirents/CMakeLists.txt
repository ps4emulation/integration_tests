project(Enderman VERSION 0.0.1)

link_libraries(SceSystemService)

set(SRC_FILES
  code/log.h
  code/fs_test.h
  code/main.cpp
  code/log.cpp
  code/fs_test_tools.cpp
  code/fs_test.cpp
)

create_pkg(TEST12345 11 00 ${SRC_FILES})
set_target_properties(TEST12345 PROPERTIES OO_PKG_TITLE "Enderman")
set_target_properties(TEST12345 PROPERTIES OO_PKG_APPVER "1.03")
finalize_pkg(TEST12345)
