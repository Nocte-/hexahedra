cmake_minimum_required(VERSION 2.8.3)

configure_file(${CMAKE_CURRENT_LIST_DIR}/config.hpp.in ${CMAKE_CURRENT_LIST_DIR}/config.hpp)

set(LIBNAME hexacommon)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build, options are: Debug Release." FORCE)
endif()

file(GLOB SOURCE_FILES "*.cpp" "../libs/lz4/lz4.c")

find_package(Boost ${REQUIRED_BOOST_VERSION} REQUIRED COMPONENTS filesystem signals)
include_directories(${Boost_INCLUDE_DIRS} ../libs)

file(GLOB HEADER_FILES "*.hpp")
source_group(include FILES ${HEADER_FILES})
source_group(source  FILES ${SOURCE_FILES})
add_library(${LIBNAME} STATIC ${SOURCE_FILES} ${HEADER_FILES})

target_link_libraries(${LIBNAME} ${Boost_LIBRARIES})

set(LIBS CryptoPP ES LevelDB)
foreach (LIB ${LIBS})
    find_package(${LIB} REQUIRED)
    string(TOUPPER ${LIB} ULIB)
    include_directories(${${ULIB}_INCLUDE_DIR})
    include_directories(${${ULIB}_INCLUDE_DIRS})
    target_link_libraries(${LIBNAME} ${${ULIB}_LIBRARY})
    target_link_libraries(${LIBNAME} ${${ULIB}_LIBRARIES})
endforeach()

if(LINUX)
    target_link_libraries(${LIBNAME} dl)
endif()
