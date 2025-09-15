# Make sure we've loaded the SDK
include("${CMAKE_CURRENT_LIST_DIR}/sdk.cmake")

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_VERSION 1)

set(TOOL_OS_SUFFIX "")
if (CMAKE_HOST_WIN32)
	set(TOOL_OS_SUFFIX ".exe")
endif()

set(CMAKE_SYSTEM_PROCESSOR powerpc64)

set(GCC_PPU_PREFIX "${PS3_SDK}/host-win32/ppu/bin/ppu-lv2-")
set(CMAKE_C_COMPILER "${GCC_PPU_PREFIX}gcc${TOOL_OS_SUFFIX}" CACHE PATH "C compiler")
set(CMAKE_CXX_COMPILER "${GCC_PPU_PREFIX}g++${TOOL_OS_SUFFIX}" CACHE PATH "C++ compiler")
set(CMAKE_ASM_COMPILER "${GCC_PPU_PREFIX}as${TOOL_OS_SUFFIX}" CACHE PATH "assembler")
set(CMAKE_LINKER "${GCC_PPU_PREFIX}ld${TOOL_OS_SUFFIX}" CACHE PATH "linker")

# Some common tools that might be used
set(PS3_MAKE_FSELF "${PS3_SDK}/host-win32/bin/make_fself.exe")
set(PS3_PRX_STRIP "${PS3_SDK}/host-win32/bin/ppu-lv2-prx-strip.exe")

set(PS3 True)
set(__PS3__ True)
set(SN_TARGET_PS3 True)
set(CELL True)
set(BUILD_CELL True)
set(BUILD_PS3 True)

set(CMAKE_FIND_ROOT_PATH "${PS3_SDK}/host-win32/ppu/bin" "${PS3_SDK}/target/ppu" "${PS3_SDK}/target/common")
set(CMAKE_SYSTEM_PREFIX_PATH ${CMAKE_FIND_ROOT_PATH})
set(CMAKE_INSTALL_PREFIX "${PS3_SDK}/target/ppu")

# link_directories("${PS3_SDK}/target/ppu/lib/fno-exceptions/fno-rtti" "${PS3_SDK}/target/ppu/lib/fno-exceptions" "${PS3_SDK}/target/ppu/lib")

set(CMAKE_C_STANDARD_LIBRARIES "${PS3_SDK}/target/ppu/lib/fno-exceptions/fno-rtti/libc.a ${PS3_SDK}/target/ppu/lib/fno-exceptions/fno-rtti/libm.a ${PS3_SDK}/target/ppu/lib/liblv2_stub.a")
set(CMAKE_C_LINK_EXECUTABLE "${CMAKE_C_COMPILER} <FLAGS> <CMAKE_C_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")
set(CMAKE_C_INCLUDE_PATH "${PS3_SDK}/target/common/include" "${PS3_SDK}/target/ppu/include")

set(CMAKE_CXX_STANDARD_LIBRARIES "${CMAKE_C_STANDARD_LIBRARIES} ${PS3_SDK}/target/ppu/lib/fno-exceptions/fno-rtti/libstdc++.a")
set(CMAKE_CXX_LINK_EXECUTABLE "${CMAKE_CXX_COMPILER} <FLAGS> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")
set(CMAKE_CXX_INCLUDE_PATH "${PS3_SDK}/target/common/include" "${PS3_SDK}/target/ppu/include")

set(CMAKE_ASM_COMPILE_OBJECT "<CMAKE_ASM_COMPILER> <DEFINES> <INCLUDES> <FLAGS> -o <OBJECT> <SOURCE>")


set(CMAKE_CXX_FLAGS "-fno-exceptions -fno-rtti -fno-strict-aliasing -fmodulo-sched" CACHE STRING "c++ flags")
set(CMAKE_C_FLAGS "-fno-exceptions -fno-strict-aliasing -fmodulo-sched" CACHE STRING "c flags")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG" CACHE STRING "c++ release flags")
set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG" CACHE STRING "c release flags")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -DDEBUG -D_DEBUG" CACHE STRING "c++ debug flags")
set(CMAKE_C_FLAGS_DEBUG "-O0 -g -DDEBUG -D_DEBUG" CACHE STRING "c debug flags")
set(CMAKE_SHARED_LINKER_FLAGS "" CACHE STRING "shared linker flags")
set(CMAKE_MODULE_LINKER_FLAGS "" CACHE STRING "module linker flags")
set(CMAKE_EXE_LINKER_FLAGS "" CACHE STRING "executable linker flags")

#set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE_INIT "-O3 -DNDEBUG")
set(CMAKE_ASM_FLAGS_RELEASE "")
set(CMAKE_ASM_FLAGS_RELEASE_INIT "")


set(CMAKE_STATIC_LIBRARY_SUFFIX ".a")
set(CMAKE_STATIC_LIBRARY_SUFFIX_C ".a")
set(CMAKE_STATIC_LIBRARY_SUFFIX_CXX ".a")

set(CMAKE_EXECUTABLE_SUFFIX ".prx")
set(CMAKE_EXECUTABLE_SUFFIX_C ".prx")
set(CMAKE_EXECUTABLE_SUFFIX_CXX ".prx")

set(CMAKE_C_STANDARD_COMPUTED_DEFAULT 99)
set(CMAKE_CXX_STANDARD_COMPUTED_DEFAULT 98)

# CMake can't scan outside the PS3 SDK root
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

#set(CMAKE_C_COMPILER_FORCED 1)
#set(CMAKE_CXX_COMPILER_FORCED 1)
#set(CMAKE_ASM_COMPILER_FORCED 1)

add_compile_definitions(__PPU=1 __BIG_ENDIAN__=1 PS3=1)