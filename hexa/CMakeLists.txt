cmake_minimum_required(VERSION 2.8.3)

configure_file(${CMAKE_CURRENT_LIST_DIR}/config.hpp.in ${CMAKE_CURRENT_LIST_DIR}/config.hpp)

set(LIBNAME hexacommon)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build, options are: Debug Release." FORCE)
endif()

find_package(Sqlite3)
if (SQLITE3_FOUND)
  file(GLOB SOURCE_FILES "*.cpp" "../libs/lz4/lz4.c")
else()
  file(GLOB SOURCE_FILES "*.cpp" "../libs/sqlite/sqlite3.c" "../libs/lz4/lz4.c")
endif()

find_package(Boost 1.52 REQUIRED COMPONENTS filesystem signals)
include_directories(${Boost_INCLUDE_DIRS} ${SQLITE3_INCLUDE_DIR} ../libs)

file(GLOB HEADER_FILES "*.hpp" "physics/*.hpp")
source_group(include FILES ${HEADER_FILES})
source_group(source  FILES ${SOURCE_FILES})
add_library(${LIBNAME} STATIC ${SOURCE_FILES} ${HEADER_FILES})

target_link_libraries(${LIBNAME} ${Boost_LIBRARIES} ${SQLITE3_LIBRARY})

if(NOT MSYS)
    target_link_libraries(${LIBNAME} dl)
endif()

foreach (LIB CryptoPP ES)
    find_package(${LIB} REQUIRED)
    string(TOUPPER ${LIB} ULIB)
    include_directories(${${ULIB}_INCLUDE_DIR})
    target_link_libraries(${LIBNAME} ${${ULIB}_LIBRARY})
    target_link_libraries(${LIBNAME} ${${ULIB}_LIBRARIES})
endforeach()

