project(highfw_test VERSION 0.0.1)

link_libraries(SceSystemService SceUserService)

create_pkg("HIFW00001" 11 00 "code/main.cpp")
