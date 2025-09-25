cmake_minimum_required(VERSION 3.20)

set(GAME_PLATFORM "cell")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY $<1:${CMAKE_BINARY_DIR}/bin> CACHE INTERNAL "")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

if (NOT GAME_PLATFORM MATCHES "win32")
	set(GAME_CONFIG "lbpib")
	include(LoadConfigSymbols)

	if (GAME_PLATFORM MATCHES "psp2")
		if (DEFINED ENV{VITASDK})
			set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
		else()
			message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
		endif()
	elseif (GAME_PLATFORM MATCHES "cell")
		set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cell/ppu.cmake")
		add_link_options(-mprx)
	endif()
else()
	# For now, on Windows, just assume we're using LBP1,
	# this is only used for testing reimplementations anyway.
	add_compile_definitions(LBP1=1)
	set(WIN32 True)
	add_compile_definitions(WIN32=1 VC_EXTRALEAN=1 WIN32_LEAN_AND_MEAN=1 NOMINMAX=1)
	add_compile_definitions(RESOURCE_SYSTEM_REIMPLEMENTATION=1)
	
	# Still include the PS3 SDK so we can use the Vectormath types
	include(cell/sdk)
endif()

project(cwml_root VERSION 1.0.0 LANGUAGES CXX ASM)

add_subdirectory(sdk/zlib)
add_subdirectory(code/Ib)
add_subdirectory(code/CoreLib)
add_subdirectory(code/CWLib)

if (GAME_PLATFORM MATCHES "win32")
	add_subdirectory(code/pctest1)
else()
	if (NOT GAME_CONFIG MATCHES "lbpib")
		add_subdirectory(code/cwml)
	endif()

	if (GAME_CONFIG MATCHES "lbp2")
		add_subdirectory(plugins/Crossplay)
		add_subdirectory(plugins/Patchwork)
	endif()

	if (GAME_CONFIG MATCHES "lbpib")
		add_subdirectory(code/evanspoker)
	endif()
endif()