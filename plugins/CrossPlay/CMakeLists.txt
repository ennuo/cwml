cmake_minimum_required(VERSION 3.20)
project(CrossPlay VERSION 1.0.0 LANGUAGES CXX ASM)

file(GLOB_RECURSE SRC_FILES "${PROJECT_SOURCE_DIR}/src/**.cpp")
add_executable(${PROJECT_NAME} ${SRC_FILES})

target_include_directories(${PROJECT_NAME} PUBLIC
    ${PROJECT_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/code/cwml/include
    ${CMAKE_SOURCE_DIR}/code/CWLib/src
    ${CMAKE_SOURCE_DIR}/code/CoreLib/src
    ${CMAKE_SOURCE_DIR}/code/Ib/include
)

# yoinked from rpcs3
option(WOLFSSL_TLS13 "Enable wolfSSL TLS v1.3 (default: enabled)" OFF)
set(WOLFSSL_SHA3 ON CACHE STRING "Enable wolfSSL SHA-3 support (default: enabled on x86_64/aarch64)")
set(WOLFSSL_SHAKE256 ON CACHE STRING "Enable wolfSSL SHAKE256 support (default: enabled on x86_64/aarch64)")
option(WOLFSSL_BASE64_ENCODE "Enable Base64 encoding (default: enabled on x86_64)" OFF)
option(WOLFSSL_DES3 "Enable DES3 (default: disabled)" OFF)
option(WOLFSSL_PWDBASED "Enable PWDBASED (default: disabled)" N)
option(WOLFSSL_FAST_MATH "Enable fast math ops (default: disabled)" ON)
option(WOLFSSL_EXAMPLES "Enable examples (default: enabled)" OFF)
option(WOLFSSL_CRYPT_TESTS "Enable Crypt Bench/Test (default: enabled)" OFF)
option(WOLFSSL_ASYNC_THREADS "Enable Asynchronous Threading (default: enabled)" OFF)
option(WOLFSSL_BUILD_OUT_OF_TREE "Don't generate files in the source tree (default: yes)" ON)
option(WOLFSSL_SNI "Enable SNI (default: disabled)" ON)
option(WOLFSSL_OPENSSLEXTRA "Enable extra OpenSSL API, size+ (default: disabled)" OFF)
option(WOLFSSL_HARDEN "Enable Hardened build, Enables Timing Resistance and Blinding (default: enabled)" OFF)
option(WOLFSSL_ALT_CERT_CHAINS "Enable support for Alternate certification chains (default: disabled)" ON)
option(BUILD_SHARED_LIBS "Build shared libraries (.dll/.so) instead of static ones (.lib/.a)" OFF)
set(WOLFSSL_SINGLE_THREADED ON CACHE STRING "")

add_subdirectory(sdk/wolfssl EXCLUDE_FROM_ALL)
target_compile_definitions(wolfssl PUBLIC 
    WOLFSSL_DES_ECB=1
    HAVE_WRITE_DUP=1
    FP_MAX_BITS=8192 
    WOLFSSL_NO_OPTIONS_H=1
    NO_WOLFSSL_DIR=1
    NO_BIO=1
    NO_ASN_TIME=1
    NO_TIMEVAL=1
    NO_FILESYSTEM=1
    NO_STDIO_FILESYSTEM=1
    OPENSSL_EXTRA_NO_ASN1=1
    WC_NO_STATIC_ASSERT=1
    XMALLOC_USER=1
    BIG_ENDIAN_ORDER=1
    NO_WOLFSSL_SERVER=1
    NO_ERROR_STRINGS=1
    NO_MULTIBYTE_PRINT=1
    NO_DEV_URANDOM=1
)

target_link_libraries(${PROJECT_NAME} CWLib wolfssl)
target_precompile_headers(${PROJECT_NAME} PUBLIC "${CMAKE_SOURCE_DIR}/code/CoreLib/src/types.h")
include(cell/sprx/crossplay)