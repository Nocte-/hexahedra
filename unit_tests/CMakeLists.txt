project (unit-tests)
cmake_minimum_required (VERSION 2.8.3)
set(EXE unit_tests)

file(GLOB SOURCE_FILES "*.cpp")
file(GLOB HEADER_FILES "*.hpp")

add_executable(${EXE} ${SOURCE_FILES} ${HEADER_FILES})

link_directories()
include_directories(.. ../es ../rhea ../libs)


find_package(Boost 1.46 REQUIRED COMPONENTS filesystem signals system thread iostreams unit_test_framework)

include_directories(${Boost_INCLUDE_DIRS})

target_link_libraries(${EXE} hexacommon dl ${Boost_LIBRARIES})

