project(Enderman VERSION 0.0.1)

link_libraries(SceSystemService)

create_pkg(TEST12345 11 00 "code/log.cpp;code/fs_test_tools.cpp;code/fs_test.cpp;code/main.cpp")
set_target_properties(TEST12345 PROPERTIES OO_PKG_TITLE "Enderman")
set_target_properties(TEST12345 PROPERTIES OO_PKG_APPVER "1.05")
finalize_pkg(TEST12345)
