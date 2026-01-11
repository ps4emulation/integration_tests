project(controller_test VERSION 0.0.1)

#link_libraries(SceSystemService)

set(SRC_FILES
  code/main.cpp
  code/test.cpp
)

create_pkg("CONT00350" 3 50 ${SRC_FILES})
create_pkg("CONT00550" 5 50 ${SRC_FILES})
