project(memory_test_2 VERSION 0.0.1)

link_libraries(SceSystemService )

set(SRC_FILES
  code/main.cpp
  code/test.cpp
)

create_pkg("MEMT00170" 1 70 ${SRC_FILES})