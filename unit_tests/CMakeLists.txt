project (unit-tests)
cmake_minimum_required (VERSION 2.8.3)
set(EXE unit_tests)

ADD_DEFINITIONS(-DBOOST_TEST_DYN_LINK)

file(GLOB SOURCE_FILES "*.cpp")
file(GLOB HEADER_FILES "*.hpp")

add_executable(${EXE} ${SOURCE_FILES} ${HEADER_FILES})
include_directories(.. ../es ../rhea ../libs)

find_package(Boost ${REQUIRED_BOOST_VERSION} REQUIRED COMPONENTS filesystem signals system thread iostreams unit_test_framework)
include_directories(${Boost_INCLUDE_DIRS})

target_link_libraries(${EXE} hexaserver hexacommon hexanoise-s dl ${Boost_LIBRARIES})

file(GLOB TESTFILES "${CMAKE_CURRENT_SOURCE_DIR}/terrain_test_*.json")
file(COPY ${TESTFILES} DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")

