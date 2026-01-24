project(memory_test_4 VERSION 0.0.1)

link_libraries(SceSystemService )

set(SRC_FILES
  code/main.cpp
  code/test.cpp
)

create_pkg("MEMT00250" 2 50 ${SRC_FILES})