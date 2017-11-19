cmake_minimum_required(VERSION 3.7)
project(vbindiff)

SET(PACKAGE_VERSION 3.0_beta6)

set(CMAKE_CXX_STANDARD 14)

configure_file(
        "${PROJECT_SOURCE_DIR}/config.h.cmake-in"
        "${PROJECT_BINARY_DIR}/config.h"
)
include_directories(${PROJECT_BINARY_DIR})

find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})
find_library(CURSES_PANEL_LIBRARY NAMES panel DOC "The curses panel library")
if (NOT CURSES_PANEL_LIBRARY)
    message(FATAL_ERROR "A required curses' panel library not found.")
endif ()
# Add panels to the library path
set (CURSES_LIBRARIES ${CURSES_LIBRARIES} ${CURSES_PANEL_LIBRARY})

set(SOURCE_FILES
        vbindiff.cpp
        tables.h
        curses/ConWin.cpp
        curses/ConWin.hpp
        curses/FileIO.hpp
        GetOpt/GetOpt.cpp
        GetOpt/GetOpt.hpp)

include_directories(BEFORE curses)

add_executable(vbindiff ${SOURCE_FILES})

target_link_libraries(vbindiff ${CURSES_LIBRARIES})
