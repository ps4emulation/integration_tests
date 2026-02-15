project(highfw_test VERSION 0.0.1)

link_libraries(SceSystemService SceUserService)

create_pkg(HIFW00001 11 00 "code/main.cpp")
set_target_properties(HIFW00001 PROPERTIES OO_PKG_TITLE "High firmware version test")
set_target_properties(HIFW00001 PROPERTIES OO_PKG_APPVER "1.1")
finalize_pkg(HIFW00001)
