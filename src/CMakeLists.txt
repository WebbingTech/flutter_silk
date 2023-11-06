# The Flutter tooling requires that developers have CMake 3.10 or later
# installed. You should not increase this version, as doing so will cause
# the plugin to fail to compile for some customers of the plugin.
cmake_minimum_required(VERSION 3.10)

#project(flutter_silk_library VERSION 0.0.1 LANGUAGES C)
project(flutter_silk_library VERSION 0.0.1)

if(WIN32)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lame/Dll)
endif()

add_definitions(-DHAVE_STDINT_H)
add_definitions(-DHAVE_MPGLIB)
add_definitions(-DDECODE_ON_THE_FLY)
add_definitions(-DUSE_FAST_LOG)
add_definitions(-DTAKEHIRO_IEEE754_HACK)

if((${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC))
    add_definitions(-DUSE_LAYER_2)
    add_definitions(-DHAVE_CONFIG_H)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-D_CRT_SECURE_NO_DEPRECATED)
    configure_file(lame/configMS.h lame/config.h @ONLY)
else()
    add_definitions(-DSTDC_HEADERS)
    if(NOT ieee754_float32_t)
        add_definitions(-Dieee754_float32_t=float)
    endif()
endif()


include_directories(
        silk/interface/
        silk/src/
        lame/include/
        lame/
        lame/libmp3lame/
        lame/mpglib/
)


file(GLOB SILK_SRC
        silk/src/*.c
        )

file(GLOB lAME_SRC
        lame/*.c
        lame/libmp3lame/*.c
        lame/libmp3lame/vector/*.c
        lame/mpglib/*.c
        )

add_library(flutter_silk SHARED
        "flutter_silk.c"
        ${lAME_SRC}
        ${SILK_SRC}
        )


set_target_properties(flutter_silk PROPERTIES
        PUBLIC_HEADER flutter_silk.h
        OUTPUT_NAME "flutter_silk"
        )

target_compile_definitions(flutter_silk PUBLIC DART_SHARED_LIB)