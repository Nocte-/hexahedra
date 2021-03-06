IF(RHEA_LIBRARY AND RHEA_INCLUDE_DIR)
  # in cache already
  SET(RHEA_FIND_QUIETLY TRUE)
ENDIF(RHEA_LIBRARY AND RHEA_INCLUDE_DIR)

find_path(RHEA_INCLUDE_DIR
  rhea/variable.hpp
  "${CMAKE_SOURCE_DIR}/../rhea/src")

find_library(RHEA_LIBRARY rhea
  "${CMAKE_SOURCE_DIR}/../rhea"
  rhea)


IF(RHEA_LIBRARY AND RHEA_INCLUDE_DIR)
  SET(RHEA_FOUND "YES")
  IF(NOT RHEA_FIND_QUIETLY)
    MESSAGE(STATUS "Found Rhea: ${RHEA_LIBRARY}")
  ENDIF(NOT RHEA_FIND_QUIETLY)
ELSE(RHEA_LIBRARY AND RHEA_INCLUDE_DIR)
  IF(NOT RHEA_FIND_QUIETLY)
    MESSAGE(STATUS "Warning: Unable to find Rhea! Get it from http://github.com/Nocte-/rhea")
  ENDIF(NOT RHEA_FIND_QUIETLY)
ENDIF(RHEA_LIBRARY AND RHEA_INCLUDE_DIR)

