cmake_minimum_required(VERSION 3.16)

project(ProductDatabase LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

# A failed configure may leave a stale Qt6_DIR in the cache, which would stop
# find_package() from searching again.
if(Qt6_DIR AND NOT EXISTS "${Qt6_DIR}/Qt6Config.cmake")
    unset(Qt6_DIR CACHE)
endif()

# Editors (CLion, Qt Creator) do not always pass CMAKE_PREFIX_PATH, so look for a
# locally installed Qt 6 when nothing was specified.
if(NOT CMAKE_PREFIX_PATH)
    foreach(candidate IN ITEMS "$ENV{QTDIR}" "D:/Qt/6.6.0/mingw_64" "C:/Qt/6.6.0/mingw_64")
        if(EXISTS "${candidate}/lib/cmake/Qt6/Qt6Config.cmake")
            list(APPEND CMAKE_PREFIX_PATH "${candidate}")
        endif()
    endforeach()
endif()

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(ProductDatabase WIN32
    src/main.cpp
    src/db/Record.cpp
    src/db/Record.h
    src/db/BinaryIO.h
    src/db/DbException.h
    src/db/HashIndex.cpp
    src/db/HashIndex.h
    src/db/DatabaseManager.cpp
    src/db/DatabaseManager.h
    src/gui/RecordDialog.cpp
    src/gui/RecordDialog.h
    src/gui/DeleteDialog.cpp
    src/gui/DeleteDialog.h
    src/gui/SearchDialog.cpp
    src/gui/SearchDialog.h
    src/gui/MainFrame.cpp
    src/gui/MainFrame.h
)

target_include_directories(ProductDatabase PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_link_libraries(ProductDatabase PRIVATE Qt6::Widgets)
