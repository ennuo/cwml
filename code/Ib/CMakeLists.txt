project(Ib VERSION 1.0.0 LANGUAGES CXX)

file(GLOB_RECURSE VITA_SOURCE_FILES "${PROJECT_SOURCE_DIR}/src/psp2/**.cpp")
file(GLOB_RECURSE PS3_SOURCE_FILES "${PROJECT_SOURCE_DIR}/src/cell/**.cpp")
file(GLOB_RECURSE WIN32_SOURCE_FILES "${PROJECT_SOURCE_DIR}/src/win32/**.cpp")

add_library(Ib STATIC src/ScopedHook.cpp src/SHA1.cpp)

if (VITA)
	target_sources(Ib PRIVATE ${VITA_SOURCE_FILES})
	target_compile_definitions(Ib PRIVATE VITA=1)
elseif(PS3)
	target_sources(Ib PRIVATE ${PS3_SOURCE_FILES})
	target_compile_definitions(Ib PRIVATE PS3=1)
elseif(WIN32)
	target_sources(Ib PRIVATE ${WIN32_SOURCE_FILES})
	target_compile_definitions(Ib PRIVATE WIN32=1)
endif()

target_include_directories(Ib PUBLIC "${PROJECT_SOURCE_DIR}/include")
