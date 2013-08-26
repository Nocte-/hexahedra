cmake_minimum_required (VERSION 2.8.3)
set(EXE hexahedra-server)
set(LIBNAME hexaserver)

file(GLOB SOURCE_FILES "*.cpp" "../../libs/luabind/*.cpp")
file(GLOB HEADER_FILES "*.hpp")

source_group(include FILES ${HEADER_FILES})
source_group(source  FILES ${SOURCE_FILES})

add_library(${LIBNAME} ${SOURCE_FILES} ${HEADER_FILES})
add_executable(${EXE} ${HEADER_FILES})

include_directories(../.. ../../libs)
link_directories(..)

set(ADD_DL dl)
set(BOOST_THREAD_SUFFIX "")
if(WIN32)
  set(ADD_DL "")
  set(ADDITIONAL_BOOST_LIBS date_time regex)
  if(MSYS)
      set(BOOST_THREAD_SUFFIX "_win32")
  endif()
endif()

find_package(Boost 1.52 REQUIRED COMPONENTS chrono iostreams program_options filesystem system signals thread${BOOST_THREAD_SUFFIX} ${ADDITIONAL_BOOST_LIBS})
include_directories(${Boost_INCLUDE_DIRS})
target_link_libraries(${LIBNAME} ${Boost_LIBRARIES})

find_package(LuaJIT)
if(LUAJIT_FOUND)
  include_directories(${LUAJIT_INCLUDE_DIR})
  target_link_libraries(${LIBNAME} ${LUAJIT_LIBRARIES})
else(LUAJIT_FOUND)
  find_package(Lua51)
  if(LUA51_FOUND)
    message("LuaJIT not found, using Lua instead.")
    include_directories(${LUA_INCLUDE_DIR})
    target_link_libraries(${LIBNAME} ${LUA_LIBRARY})
  else(LUA51_FOUND)
    message(SEND_ERROR "The server requires either LuaJIT or Lua")
  endif(LUA51_FOUND)
endif(LUAJIT_FOUND)

foreach (LIB ENet ES)
    find_package(${LIB} REQUIRED)
    string(TOUPPER ${LIB} ULIB)
    include_directories(${${ULIB}_INCLUDE_DIR})
    target_link_libraries(${LIBNAME} ${${ULIB}_LIBRARY})
    target_link_libraries(${LIBNAME} ${${ULIB}_LIBRARIES})
endforeach()

target_link_libraries(${LIBNAME} hexacommon z ${ADD_DL})
if(WIN32)
    target_link_libraries(${LIBNAME} ws2_32 winmm)
endif()
target_link_libraries(${EXE} hexaserver hexacommon)

# Installation
install(TARGETS ${EXE} DESTINATION "${BINDIR}")

