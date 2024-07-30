cmake_minimum_required(VERSION 3.22.1 FATAL_ERROR)
project(zcore VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra")
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Check if the operating system is Arch Linux
if(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux" OR NOT EXISTS "/etc/arch-release")
    message(FATAL_ERROR "This project requires Arch Linux to build.")
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "14.1.1")
        message(FATAL_ERROR "GCC version must be at least 14.1.1!")
    endif()
endif()

include(CheckCXXCompilerFlag)

function(add_compiler_flag_if_supported flag)
    string(REPLACE "-" "_" flag_var "${flag}")
    set(CXX_FLAG_SUPPORTED_VAR "CXX_FLAG_${flag_var}_SUPPORTED")
    check_cxx_compiler_flag("${flag}" ${CXX_FLAG_SUPPORTED_VAR})

    if(${${CXX_FLAG_SUPPORTED_VAR}})
        add_compile_options("${flag}")
    endif()
endfunction()

add_compiler_flag_if_supported("-Wextra-semi-stmt")
add_compiler_flag_if_supported("-Wfloat-conversion")
add_compiler_flag_if_supported("-Wformat=2")
add_compiler_flag_if_supported("-Wlogical-op")
add_compiler_flag_if_supported("-Wpointer-arith")
add_compiler_flag_if_supported("-Wshadow")
add_compiler_flag_if_supported("-Wshorten-64-to-32")
add_compiler_flag_if_supported("-Wsign-compare")
add_compiler_flag_if_supported("-Wstring-conversion")
add_compiler_flag_if_supported("-Wtype-limits")
add_compiler_flag_if_supported("-Wuninitialized")
add_compiler_flag_if_supported("-Wunreachable-code-aggressive")
add_compiler_flag_if_supported("-Wunused")
add_compiler_flag_if_supported("-Wvla")
add_compiler_flag_if_supported("-Wextra-semi")

if(EXISTS "/etc/lsb-release")
    add_compile_options(
        $<$<CONFIG:DEBUG>:-Wall>
        $<$<CONFIG:DEBUG>:-Wextra>
        $<$<CONFIG:RELEASE>:-O2>
        $<$<CONFIG:RELEASE>:-march=x86-64-v3>
        $<$<CONFIG:RELEASE>:-pipe>
        $<$<CONFIG:RELEASE>:-DNDEBUG>
        $<$<CONFIG:RELEASE>:-flto=auto>
        $<$<CONFIG:DEBUG>:-fcoroutines>
        $<$<CONFIG:RELEASE>:-fcoroutines>
    )
endif()

set(ENV{PKG_CONFIG_PATH} "/usr/lib/pkgconfig/")
find_package(PkgConfig REQUIRED)
find_package(LibSSH REQUIRED)
find_package(glog 0.7.0 REQUIRED)
find_package(CLI11 REQUIRED)
pkg_check_modules(CLI11 REQUIRED IMPORTED_TARGET "CLI11>=2.4.1")
pkg_check_modules(ssh REQUIRED IMPORTED_TARGET "libssh>=0.10.6")

LINK_DIRECTORIES("/usr/lib")
INCLUDE_DIRECTORIES("/usr/include")
include_directories(${JSONCPP_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/head)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src SOURCES)

add_executable(zcore ${SOURCES})
target_link_libraries(zcore PkgConfig::CLI11 PkgConfig::ssh glog::glog)