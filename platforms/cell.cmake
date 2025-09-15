set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 98)
set(CMAKE_C_STANDARD 99)

set(CMAKE_BUILD_TYPE "Debug")


# set(CWML_HOT_RELOAD TRUE)

file(GLOB_RECURSE SRC_FILES "${PROJECT_SOURCE_DIR}/code/**.cpp")
file(GLOB_RECURSE HEADER_FILES "${PROJECT_SOURCE_DIR}/code/**.h")
file(GLOB_RECURSE ASSEMBLY_FILES "${PROJECT_SOURCE_DIR}/code/**.s")

add_executable(${CMAKE_PROJECT_NAME} ${SRC_FILES} ${ASSEMBLY_FILES} ${HEADER_FILES})

target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC
    ${PROJECT_SOURCE_DIR}/code/cwml/include
    ${PROJECT_SOURCE_DIR}/code/cwml/src
    ${PROJECT_SOURCE_DIR}/code/CWLib/src
    ${PROJECT_SOURCE_DIR}/code/CoreLib/src
    ${PROJECT_SOURCE_DIR}/code/PrxLib/src
)

target_compile_options(${CMAKE_PROJECT_NAME} PUBLIC
    $<$<COMPILE_LANGUAGE:C,CXX>:
        #-Wall
        #-Wextra
        -O3
        -fno-exceptions
        -fno-strict-aliasing

        -fmodulo-sched
        #-v
    >

    $<$<COMPILE_LANGUAGE:CXX>:
        -fno-rtti
    >
)

target_link_libraries(${CMAKE_PROJECT_NAME} fs_stub audio_stub gcm_cmd gcm_sys_stub http_stub http_util_stub net_stub sysutil_np_stub sysutil_game_stub rtc_stub)
target_compile_definitions(${CMAKE_PROJECT_NAME} PUBLIC 
    __GCC__=1
    _NO_EX=1
    _HAS_EXCEPTIONS=0
)

if (CWML_HOT_RELOAD)
    target_compile_definitions(${CMAKE_PROJECT_NAME} PUBLIC CWML_HOT_RELOAD=1)
endif()

target_precompile_headers(${CMAKE_PROJECT_NAME} PUBLIC code/CoreLib/src/types.h code/PrxLib/src/printf.h code/PrxLib/src/hook.h)


add_subdirectory(plugins/CrossPlay)
add_subdirectory(plugins/RemoteDebugger)