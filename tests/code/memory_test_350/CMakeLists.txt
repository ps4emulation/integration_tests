project(memory_test_6 VERSION 0.0.1)

link_libraries(SceSystemService )

set(SRC_FILES
  code/main.cpp
  code/test.cpp
)

create_pkg("MEMT00350" 3 50 ${SRC_FILES})