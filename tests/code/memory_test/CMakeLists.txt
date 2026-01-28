project(memory_test_1 VERSION 0.0.1)

link_libraries(SceSystemService)

create_pkg("MEMT00100" 1 00 "code/main.cpp;code/test_100.cpp")
create_pkg("MEMT00170" 1 70 "code/main.cpp;code/test_170.cpp")
create_pkg("MEMT00200" 2 00 "code/main.cpp;code/test_200.cpp")
create_pkg("MEMT00250" 2 50 "code/main.cpp;code/test_250.cpp")
create_pkg("MEMT00300" 3 00 "code/main.cpp;code/test_300.cpp")
create_pkg("MEMT00350" 3 50 "code/main.cpp;code/test_350.cpp")
create_pkg("MEMT00550" 5 50 "code/main.cpp;code/test_550.cpp")
