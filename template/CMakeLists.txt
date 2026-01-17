project(template_test LANGUAGES CXX)

link_libraries(SceSystemService)

set(SRC_FILES
  code/main.cpp
  code/test.cpp
)

create_pkg("TEMT00350" 3 50 ${SRC_FILES})
create_pkg("TEMT00550" 5 50 ${SRC_FILES})
