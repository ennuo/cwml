project(pctest1 VERSION 1.0.0 LANGUAGES CXX)

add_executable(pctest1 src/main.cpp)
target_include_directories(pctest1 PRIVATE "${PS3_SDK}/target/common/include")
target_compile_definitions(pctest1 PRIVATE WIN32=1)

target_include_directories(pctest1 PUBLIC
    "${PROJECT_SOURCE_DIR}/src"
    "${PROJECT_SOURCE_DIR}/../CWLib/src"
    "${PROJECT_SOURCE_DIR}/../CoreLib/src"
    "${PROJECT_SOURCE_DIR}/../Spuge/src"
    "${PROJECT_SOURCE_DIR}/../Ib/include"
)

target_precompile_headers(pctest1 PUBLIC ../CoreLib/src/types.h)
target_link_libraries(pctest1 CWLib)