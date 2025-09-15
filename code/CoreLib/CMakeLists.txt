project(CoreLib VERSION 1.0.0 LANGUAGES CXX)

file(GLOB_RECURSE VITA_SOURCE_FILES "${PROJECT_SOURCE_DIR}/src/psp2/**.cpp")
file(GLOB_RECURSE PS3_SOURCE_FILES "${PROJECT_SOURCE_DIR}/src/cell/**.cpp")
file(GLOB_RECURSE WIN32_SOURCE_FILES "${PROJECT_SOURCE_DIR}/src/win32/**.cpp")

add_library(CoreLib STATIC
    src/CalendarTime.cpp
    src/Clock.cpp
    src/Colour.cpp
    src/CompressionJob.cpp
    src/Directory.cpp
    src/GameUpdateStage.cpp
    src/filepath.cpp
    src/GuidHash.cpp
    src/GuidHashMap.cpp
    src/Hash.cpp
    src/JobManager.cpp
    src/refcount.cpp
    src/Serialise.cpp
    src/sha1shared.cpp
    src/StringUtil.cpp
    src/TextStream.cpp
    src/memory/Allocator.cpp
    src/memory/Memory.cpp
)

if (VITA)
	target_sources(CoreLib PRIVATE ${VITA_SOURCE_FILES})
	target_compile_definitions(CoreLib PRIVATE VITA=1)
    target_include_directories(CoreLib PUBLIC /mnt/e/usr/local/psp2/target/include_common)
elseif(PS3)
	target_sources(CoreLib PRIVATE ${PS3_SOURCE_FILES})
	target_compile_definitions(CoreLib PRIVATE PS3=1)
    target_link_libraries(CoreLib fs_stub rtc_stub)
elseif(WIN32)
	target_sources(CoreLib PRIVATE ${WIN32_SOURCE_FILES})
    target_include_directories(CoreLib PRIVATE "${PS3_SDK}/target/common/include")
	target_compile_definitions(CoreLib PRIVATE WIN32=1)
endif()

target_include_directories(CoreLib PUBLIC
    "${PROJECT_SOURCE_DIR}/src"
    "${PROJECT_SOURCE_DIR}/../Spuge/src"
    "${PROJECT_SOURCE_DIR}/../Ib/include"
    "${CMAKE_SOURCE_DIR}/sdk/zlib"
)

target_precompile_headers(CoreLib PUBLIC src/types.h)
target_link_libraries(CoreLib Ib zlibstatic)
