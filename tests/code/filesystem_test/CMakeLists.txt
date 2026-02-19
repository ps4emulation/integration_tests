project(Therapist VERSION 0.0.1)

link_libraries(SceSystemService)

create_pkg(TEST21370 11 00 "code/log.cpp;code/fs_test_tools.cpp;code/fs_test.cpp;code/main.cpp")
set_target_properties(TEST21370 PROPERTIES OO_PKG_TITLE "Therapist")
set_target_properties(TEST21370 PROPERTIES OO_PKG_APPVER "1.35")
finalize_pkg(TEST21370)
