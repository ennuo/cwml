project(evanspoker VERSION 1.0.0 LANGUAGES CXX)

add_executable(evanspoker src/entry.cpp src/Evanspoker.cpp)
target_include_directories(evanspoker PUBLIC
    "${PROJECT_SOURCE_DIR}/src"
    "${PROJECT_SOURCE_DIR}/../CWLib/src"
    "${PROJECT_SOURCE_DIR}/../CoreLib/src"
    "${PROJECT_SOURCE_DIR}/../Ib/include"
)

target_precompile_headers(evanspoker PUBLIC ../CoreLib/src/types.h)
target_link_libraries(evanspoker CWLib sysmodule_stub)

include(cell/sprx/evanspoker)