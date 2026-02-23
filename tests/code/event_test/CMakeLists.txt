project(event_test VERSION 0.0.1)

link_libraries(SceSystemService SceVideoOut SceGnmDriver)

create_pkg(EVNT00100 1 00 "code/main.cpp;code/test.cpp")
set_target_properties(EVNT00100 PROPERTIES OO_PKG_TITLE "PS4 event queues test")
set_target_properties(EVNT00100 PROPERTIES OO_PKG_APPVER "1.0")
finalize_pkg(EVNT00100)