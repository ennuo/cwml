project(cwml VERSION 1.0.0 LANGUAGES CXX)

file(GLOB_RECURSE VITA_SOURCE_FILES "${PROJECT_SOURCE_DIR}/src/psp2/**.cpp")
file(GLOB_RECURSE PS3_SOURCE_FILES "${PROJECT_SOURCE_DIR}/src/cell/**.cpp")

add_executable(cwml
    src/main_ps3.cpp
    src/cwml.cpp
    src/cwml/Context.cpp
)

if (VITA)
	target_sources(cwml PRIVATE ${VITA_SOURCE_FILES})
	target_compile_definitions(cwml PRIVATE VITA=1)
    target_include_directories(cwml PUBLIC /mnt/e/usr/local/psp2/target/include_common)
elseif(PS3)
	target_sources(cwml PRIVATE ${PS3_SOURCE_FILES})
	target_compile_definitions(cwml PRIVATE PS3=1)
endif()

target_include_directories(cwml PUBLIC
    "${PROJECT_SOURCE_DIR}/src"
    "${PROJECT_SOURCE_DIR}/include"
    "${PROJECT_SOURCE_DIR}/../CoreLib/src"
    "${PROJECT_SOURCE_DIR}/../CWLib/src"
    "${PROJECT_SOURCE_DIR}/../Ib/include"
)

target_precompile_headers(cwml PUBLIC ../CoreLib/src/types.h)
target_link_libraries(cwml CWLib)
include(cell/sprx)