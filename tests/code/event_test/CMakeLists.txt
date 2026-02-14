project(event_test LANGUAGES CXX)

link_libraries(SceSystemService SceVideoOut)

create_pkg("EVNT00100" 1 00 "code/main.cpp;code/test.cpp")
