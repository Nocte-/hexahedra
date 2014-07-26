cmake_minimum_required (VERSION 2.8.3)
set(EXE hexahedra)
set(LIBNAME hexaclient)

file(GLOB SOURCE_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cpp")
list(REMOVE_ITEM SOURCE_FILES main.cpp)
file(GLOB HEADER_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.hpp")

source_group(include FILES ${HEADER_FILES})
source_group(source  FILES ${SOURCE_FILES})

add_library(${LIBNAME} ${SOURCE_FILES} ${HEADER_FILES})

include_directories(../.. ../../libs)
link_directories(..)

set(BOOST_THREAD_LIBNAME thread)
set(RC_OBJ_FILE "")
if(WIN32)
  set(ADDITIONAL_BOOST_LIBS regex)
  set(SFML_STATIC_LIBRARIES)
  set(ADDITIONAL_SFML_LIBS main)
  set(RC_FILE ${CMAKE_CURRENT_SOURCE_DIR}/../../install/hexahedra-client.rc)
  if(MINGW)
      set(BOOST_THREAD_LIBNAME thread_win32)
      set(RC_OBJ_FILE ${CMAKE_CURRENT_BINARY_DIR}/client_win_rc.o)
      add_custom_command(OUTPUT ${RC_OBJ_FILE}
                         COMMAND ${CMAKE_RC_COMPILER} -I{CMAKE_CURRENT_SOURCE_DIR}
                         -i${RC_FILE} -o${RC_OBJ_FILE}
                         WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
  else()
     set(RC_OBJ_FILE ${RC_FILE})
  endif()
endif()

add_executable(${EXE} WIN32 main.cpp ${RC_OBJ_FILE} ${HEADER_FILES})

find_package(Boost ${REQUIRED_BOOST_VERSION} REQUIRED COMPONENTS chrono date_time iostreams program_options filesystem signals system ${BOOST_THREAD_LIBNAME} ${ADDITIONAL_BOOST_LIBS})
find_package(SFML 2 REQUIRED COMPONENTS graphics window system ${ADDITIONAL_SFML_LIBS})

include_directories(${Boost_INCLUDE_DIRS} ${SFML_INCLUDE_DIR} ${LEVELDB_INCLUDE_DIR})
target_link_libraries(${LIBNAME} ${Boost_LIBRARIES} ${SFML_LIBRARIES})

set(NEED_GLEW "")
if(NOT SFML_STATIC_LIBRARIES)
    set(NEED_GLEW "GLEW")
else()
    include_directories(${GLEW_INCLUDE_DIR})
endif()

foreach (LIB ${NEED_GLEW} OpenGL CryptoPP ENet ES)
    find_package(${LIB} REQUIRED)
    string(TOUPPER ${LIB} ULIB)
    include_directories(${${ULIB}_INCLUDE_DIR})
    target_link_libraries(${LIBNAME} ${${ULIB}_LIBRARY})
    target_link_libraries(${LIBNAME} ${${ULIB}_LIBRARIES})
endforeach()

target_link_libraries(${LIBNAME} cppnetlib-client-connections cppnetlib-uri)


# OpenGL uses a different convention:
target_link_libraries(${LIBNAME} ${OPENGL_gl_LIBRARY})
target_link_libraries(${LIBNAME} ${OPENGL_glu_LIBRARY})

if(WIN32)
  target_link_libraries(${LIBNAME} ws2_32 winmm)
endif()

if(SFML_STATIC_LIBRARIES)
  if(NOT WIN32)
    target_link_libraries(${LIBNAME} freetype jpeg png z bz2 Xrandr)
  endif()
endif()

target_link_libraries(${LIBNAME} hexacommon)
target_link_libraries(${EXE} ${LIBNAME})

# Installation
install(TARGETS ${EXE} DESTINATION "${BINDIR}")

