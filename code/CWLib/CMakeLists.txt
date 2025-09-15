project(CWLib VERSION 1.0.0 LANGUAGES CXX)

file(GLOB_RECURSE VITA_SOURCE_FILES "${PROJECT_SOURCE_DIR}/src/psp2/**.cpp")
file(GLOB_RECURSE PS3_SOURCE_FILES "${PROJECT_SOURCE_DIR}/src/cell/**.cpp")
file(GLOB_RECURSE WIN32_SOURCE_FILES "${PROJECT_SOURCE_DIR}/src/win32/**.cpp")
file(GLOB_RECURSE NETWORK_SOURCE_FILES "${PROJECT_SOURCE_DIR}/src/network/**.cpp")

add_library(CWLib STATIC
    src/savegame/Quick.cpp
    src/vm/InstanceLayout.cpp
    src/vm/VMTypes.cpp
    src/Fart.cpp
    src/FartLazy.cpp
    src/FartRO.cpp
    src/readini.cpp
    src/Resource.cpp
    src/ResourceDLC.cpp
    src/ResourceGuidSubst.cpp
    src/ResourcePlan.cpp
    src/ResourceScript.cpp
    src/ResourceSystem.cpp
    src/SerialiseCW.cpp
    src/SerialisedResource.cpp
    src/SerialiseVM.cpp
    src/SystemCommon.cpp
    src/thing.cpp
    src/ThingPartContainer.cpp
    src/Variable.cpp
)

if (VITA)
	target_sources(CWLib PRIVATE ${VITA_SOURCE_FILES})
	target_compile_definitions(CWLib PRIVATE VITA=1)
elseif(PS3)
	target_sources(CWLib PRIVATE ${PS3_SOURCE_FILES})
    target_sources(CWLib PRIVATE ${NETWORK_SOURCE_FILES})
	target_compile_definitions(CWLib PRIVATE PS3=1)
    target_link_libraries(CWLib audio_stub gcm_cmd gcm_sys_stub http_stub http_util_stub net_stub sysutil_stub sysutil_np_stub sysutil_game_stub)
elseif(WIN32)
	target_sources(CWLib PRIVATE ${WIN32_SOURCE_FILES})
    target_include_directories(CWLib PRIVATE "${PS3_SDK}/target/common/include")
	target_compile_definitions(CWLib PRIVATE WIN32=1)
endif()

target_include_directories(CWLib PUBLIC
    "${PROJECT_SOURCE_DIR}/src"
    "${PROJECT_SOURCE_DIR}/../CoreLib/src"
    "${PROJECT_SOURCE_DIR}/../Spuge/src"
    "${PROJECT_SOURCE_DIR}/../Ib/include"
)

target_precompile_headers(CWLib PUBLIC ../CoreLib/src/types.h)
target_link_libraries(CWLib CoreLib)