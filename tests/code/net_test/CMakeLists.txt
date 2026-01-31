project(net_test LANGUAGES CXX)

link_libraries(SceSystemService SceNet)

create_pkg("TNET00100" 1 00 "code/main.cpp;code/test.cpp;code/nbio_stream_logger.cpp")
