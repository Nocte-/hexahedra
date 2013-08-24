# - Locate Luabind library
# This module defines
#  LUABIND_LIBRARY, the library to link against
#  LUABIND_FOUND, if false, do not try to link to LUABIND
#  LUABIND_INCLUDE_DIR, where to find headers.

IF(LUABIND_LIBRARY AND LUABIND_INCLUDE_DIR)
  # in cache already
  SET(LUABIND_FIND_QUIETLY TRUE)
ENDIF(LUABIND_LIBRARY AND LUABIND_INCLUDE_DIR)

FIND_PATH(LUABIND_INCLUDE_DIR luabind/luabind.hpp
  "${CMAKE_SOURCE_DIR}/../luabind-0.9.1"  
  "${CMAKE_SOURCE_DIR}/../luabind"  
  $ENV{LUABIND_DIR}/include
  /usr/local/include
  /usr/include
  /sw/include
  /opt/local/include
  /opt/csw/include
  /opt/include
)

FIND_LIBRARY(LUABIND_LIBRARY luabind
  "${CMAKE_SOURCE_DIR}/../luabind-0.9.1/lib"  
  $ENV{LUABIND_DIR}/lib
  /usr/local/lib
  /usr/lib
  /usr/local/X11R6/lib
  /usr/X11R6/lib
  /sw/lib
  /opt/local/lib
  /opt/csw/lib
  /opt/lib
  /usr/freeware/lib64
)

IF(LUABIND_LIBRARY AND LUABIND_INCLUDE_DIR)
  SET(LUABIND_FOUND "YES")
  IF(NOT LUABIND_FIND_QUIETLY)
    MESSAGE(STATUS "Found Luabind: ${LUABIND_LIBRARY}")
  ENDIF(NOT LUABIND_FIND_QUIETLY)
ELSE(LUABIND_LIBRARY AND LUABIND_INCLUDE_DIR)
  IF(NOT LUABIND_FIND_QUIETLY)
    MESSAGE(STATUS "Warning: Unable to find Luabind!")
  ENDIF(NOT LUABIND_FIND_QUIETLY)
ENDIF(LUABIND_LIBRARY AND LUABIND_INCLUDE_DIR)
