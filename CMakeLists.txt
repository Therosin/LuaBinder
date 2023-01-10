cmake_minimum_required(VERSION 3.14)
project(LuaBinder VERSION 1.0.0)
# set bin output path
set ( EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/LuaBinder )
# set lib output path
set ( LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/LuaBinder )

# lua513
find_package(lua513 QUIET)
if (NOT lua513_FOUND)
    message(STATUS "lua513 package not found - using FetchContent to download and build lua513")
    include(FetchContent)
    FetchContent_Declare(
        lua513
        GIT_REPOSITORY https://github.com/Therosin/lua513.git
        GIT_TAG Lua513
    )
    FetchContent_MakeAvailable(lua513)
endif()
include_directories(${lua513_SOURCE_DIR}/src)

set(SOURCES LuaBinder.cpp Global.h)
add_executable(${PROJECT_NAME} ${SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17)
target_link_libraries(${PROJECT_NAME} PUBLIC liblua)
