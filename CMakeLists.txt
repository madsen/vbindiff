cmake_minimum_required(VERSION 3.7)
project(vbindiff)

SET(PACKAGE_VERSION 3.0_beta6)
SET(PACKAGE_VERSION_NUM "3,0,200,6")

set(CMAKE_CXX_STANDARD 14)

configure_file(
        "${PROJECT_SOURCE_DIR}/config.h.cmake-in"
        "${PROJECT_BINARY_DIR}/config.h"
)
include_directories(${PROJECT_BINARY_DIR})

set(SOURCE_FILES
        vbindiff.cpp
        tables.h
        GetOpt/GetOpt.cpp
        GetOpt/GetOpt.hpp)

if (WIN32)
    set(SOURCE_FILES ${SOURCE_FILES}
            win32/ConWin.cpp
            win32/ConWin.hpp
            win32/FileIO.hpp)
    include_directories(BEFORE win32)

    configure_file(
            "${PROJECT_SOURCE_DIR}/win32/version.h.cmake-in"
            "${PROJECT_BINARY_DIR}/version.h"
    )

    if (CMAKE_COMPILER_IS_GNUCC)
        set (CMAKE_RC_COMPILER_INIT windres)
        enable_language(RC)
        set (CMAKE_RC_COMPILE_OBJECT
                "<CMAKE_RC_COMPILER> -O coff -I${PROJECT_BINARY_DIR} -I${PROJECT_SOURCE_DIR}/win32/mingw <FLAGS> <DEFINES> <SOURCE> <OBJECT>")
    endif ()
    set (SOURCE_FILES ${SOURCE_FILES} "${PROJECT_SOURCE_DIR}/win32/vbindiff.rc")
else()
    find_package(Curses REQUIRED)
    include_directories(${CURSES_INCLUDE_DIR})
    find_library(CURSES_PANEL_LIBRARY NAMES panel DOC "The curses panel library")
    if (NOT CURSES_PANEL_LIBRARY)
        message(FATAL_ERROR "A required curses' panel library not found.")
    endif ()
    # Add panels to the library path
    set (CURSES_LIBRARIES ${CURSES_LIBRARIES} ${CURSES_PANEL_LIBRARY})

    set(SOURCE_FILES ${SOURCE_FILES}
            curses/ConWin.cpp
            curses/ConWin.hpp
            curses/FileIO.hpp)
    include_directories(BEFORE curses)

    set(PROJECT_FLAGS "-Wall -Wno-narrowing -D_FILE_OFFSET_BITS=64")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${PROJECT_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} ${PROJECT_FLAGS}")
endif()

add_executable(vbindiff ${SOURCE_FILES})

target_link_libraries(vbindiff ${CURSES_LIBRARIES})
