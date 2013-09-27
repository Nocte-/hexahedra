project(hexahedra)

cmake_minimum_required(VERSION 2.8.3)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build (Debug or Release)" FORCE)
endif()

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules/")

set(CPACK_PACKAGE_NAME "Hexahedra")
set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "1")
set(CPACK_PACKAGE_VERSION_PATCH "1")
set(VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
set(COPYRIGHT "Copyright (c) 2013, Nocte <nocte@hippie.nu>")
EXECUTE_PROCESS(COMMAND "date" "-Iseconds" 
                OUTPUT_VARIABLE BUILDTIME 
                OUTPUT_STRIP_TRAILING_WHITESPACE)
set(WEBPAGE "http://hexahedra.net/")

# What to build?
#
set(BUILD_SERVER 1 CACHE BOOL "Build the server")
set(BUILD_CLIENT 1 CACHE BOOL "Build the demo client")
set(BUILD_UNITTESTS 0 CACHE BOOL "Build the unit tests")
set(BUILD_DOCUMENTATION 0 CACHE BOOL "Generate Doxygen documentation")
set(USE_VALGRIND 0 CACHE BOOL "Use workarounds for Valgrind")
set(USE_CALLGRIND 0 CACHE BOOL "Build with -g")
set(USE_OPENCL 0 CACHE BOOL "Use OpenCL")
set(BUILD_STATIC 0 CACHE BOOL "Link executables statically")

if(USE_OPENCL)
  find_package(OpenCL)
endif()

# Set up the compiler
#
set(RES_FILES "")
if(MSVC)
    add_definitions(/D_WIN32_WINNT=0x0501 /DWIN32_LEAN_AND_MEAN /DNOMINMAX /D_CRT_SECURE_NO_WARNINGS /DSFML_STATIC /DGLEW_STATIC)
	set(CMAKE_CXX_FLAGS         "${CMAKE_CXX_FLAGS} /MP /EHsc /wd4244 /wd4996 ")
    set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /DNDEBUG /MT /MP /GS- /Ox /Ob2 /Oi /Oy /arch:SSE /fp:fast /Zi")
    set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} /DEBUG /OPT:REF /SUBSYSTEM:WINDOWS")
    
else() # Most likely gcc or clang
	set(PROFILE  0 CACHE BOOL "Add profiling information (gcc only)")
	set(COVERAGE 0 CACHE BOOL "Add coverage information (gcc only)")
	
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        set(CMAKE_CXX_FLAGS "-std=c++11")
    elseif ("${CMAKE_CXX_COMPILER_VERSION}" VERSION_LESS "4.7")
        set(CMAKE_CXX_FLAGS "-std=c++0x")
    else()
        set(CMAKE_CXX_FLAGS "-std=c++11")
    endif()

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -Wall -Wempty-body")
    if(USE_VALGRIND)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O -g -DLZ4_FORCE_SW_BITCOUNT -DHEXA_FORCE_SW_BITCOUNT")
    elseif (${CMAKE_BUILD_TYPE} STREQUAL Debug)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -msse -mfpmath=sse -DNDEBUG -ffast-math -funsafe-math-optimizations")
    endif()

    if(OPENCL_FOUND)
        add_definitions("-DHAVE_OPENCL")
    endif()

    if(USE_CALLGRIND)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
    endif()

    if(PROFILE)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pg")
    endif()

    if(COVERAGE)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-arcs -ftest-coverage")
    endif()

    if(BUILD_STATIC)
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s")
    endif()
        
    if(MINGW)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mwindows -mthreads -DWIN32 -D_WIN32 -D_WIN32_WINNT=0x0501 -DBOOST_THREAD_USE_LIB -DGLEW_STATIC")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
    endif()

endif()

# Build the individual components
#
set(Boost_USE_STATIC_LIBS    ON) 
set(Boost_USE_STATIC_RUNTIME ON) 
add_definitions(-DBOOST_FILESYSTEM_NO_DEPRECATED)

# Packaging
#
if (UNIX)
    set(SHAREDIR "${CMAKE_INSTALL_PREFIX}/share")
    set(DATADIR  "${SHAREDIR}/hexahedra" CACHE PATH "Common data files")
    set(DBDIR    "server_db" CACHE PATH "Where to store the server databases")    
    set(BINDIR   "${CMAKE_INSTALL_PREFIX}/games" CACHE PATH "Where to put the binaries")
    set(DOCDIR   "${CMAKE_INSTALL_PREFIX}/doc/hexahedra" CACHE PATH "Where to put the documentation")
elseif(WIN32)
    set(SHAREDIR ".")
    set(DATADIR  "data" CACHE PATH "Common data files")
    set(DBDIR    "server_db" CACHE PATH "Where to store the server databases")      
    set(BINDIR   "." CACHE PATH "Where to put the binaries")
    set(DOCDIR   "doc")
endif()

#set(DOC_FILES AUTHORS LICENSE README)
#install(FILES ${DOC_FILES} DESTINATION ${DOCDIR})

if (BUILD_CLIENT)
    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/data/." DESTINATION "${DATADIR}")
    if (UNIX)
        install(FILES "install/hexahedra.png"     DESTINATION "${SHAREDIR}/pixmaps")                
        install(FILES "install/hexahedra.desktop" DESTINATION "${SHAREDIR}/applications")
    endif()
    if (WIN32)
        configure_file(${CMAKE_CURRENT_SOURCE_DIR}/install/hexahedra-client.rc.in ${CMAKE_CURRENT_SOURCE_DIR}/install/hexahedra-client.rc)
        if (MINGW)
            install(FILES "win32/lib/LuaJIT-2.0.2/src/lua51.dll"
                          DESTINATION "${BINDIR}")
        endif()
    endif()
endif()
if (BUILD_SERVER)
    if (WIN32)
        configure_file(${CMAKE_CURRENT_SOURCE_DIR}/install/hexahedra-server.rc.in ${CMAKE_CURRENT_SOURCE_DIR}/install/hexahedra-server.rc)
    endif()
endif()

# Build executables
#
add_subdirectory(hexa)
if(BUILD_SERVER)
    add_subdirectory(hexa/server)
endif()
if(BUILD_CLIENT)
  add_subdirectory(hexa/client)
endif()
if(BUILD_UNITTESTS)
  add_subdirectory(unit_tests)
endif()


# Doxygen documentation
#
if(BUILD_DOCUMENTATION)
  find_package(Doxygen)
  if (DOXYGEN_FOUND STREQUAL "YES")
      configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
      add_custom_target(doxygen ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
      set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES api-doc)
      get_target_property(DOC_TARGET doc TYPE)
      if (NOT DOC_TARGET)
          add_custom_target(doc)
      endif()
      add_dependencies(doc doxygen)
  
      #install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/api-doc/html DESTINATION ${DOCDIR})
  endif()
endif()

#set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Collection of voxel games")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/doc/project_description.txt")
file(READ ${CPACK_PACKAGE_DESCRIPTION_FILE} CPACK_PACKAGE_DESCRIPTION_SUMMARY)
set(CPACK_PACKAGE_VENDOR "Nocte")
set(CPACK_PACKAGE_CONTACT "Nocte <hexahedra.maintainer@gmail.com>")
set(CPACK_SOURCE_IGNORE_FILES "CMakefiles;Makefile;CMakeCache.txt;_CPack_Packages;/.git/;")
set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}_${VERSION}")
set(CPACK_STRIP_FILES TRUE)
 
if(UNIX)
    macro(GZIP_FILE INFILE)
        add_custom_command(OUTPUT ${INFILE}.gz
                           COMMAND gzip -f -9 -c ${INFILE} > ${INFILE}.gz
                           DEPENDS ${INFILE}
                           COMMENT "Compress '${INFILE}'")
    endmacro(GZIP_FILE)

    GZIP_FILE("${CMAKE_CURRENT_SOURCE_DIR}/doc/man/hexahedra.6")
    GZIP_FILE("${CMAKE_CURRENT_SOURCE_DIR}/doc/man/hexahedra-server.6")
    GZIP_FILE("${CMAKE_CURRENT_SOURCE_DIR}/changelog")
    add_custom_target(run ALL DEPENDS doc/man/hexahedra.6.gz doc/man/hexahedra-server.6.gz changelog.gz)

    install(FILES "doc/man/hexahedra.6.gz" "doc/man/hexahedra-server.6.gz" DESTINATION "${CMAKE_INSTALL_PREFIX}/share/man/man6/")
    install(FILES "changelog.gz" "debian/copyright" DESTINATION "${CMAKE_INSTALL_PREFIX}/share/doc/${PROJECT_NAME}/")

    set(CPACK_GENERATOR "DEB")
    add_custom_target(dist COMMAND ${CMAKE_MAKE_PROGRAM} package_source)
    
    set(CPACK_DEBIAN_PACKAGE_SECTION "games")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.0), libenet1a, libjpeg62, libgcc1 (>= 1:4.1.1), libglew1.8, libglu1-mesa | libglu1, libluajit-5.1-2, libstdc++6 (>= 4.0), zlib1g (>= 1:1.1.4), libsqlite3-0, libluabind0.9.1")
    add_custom_target(deb dpkg-buildpackage)
    add_dependencies(deb dist)
    
    set(CPACK_RPM_PACKAGE_GROUP "Amusements/Games")    
    add_custom_target(rpm rpmbuild -ta ${CPACK_PACKAGE_FILE_NAME}.tar.gz)
    add_dependencies(rpm dist)

elseif(WIN32)
    set(CPACK_PACKAGE_INSTALL_DIRECTORY "Hexahedra")
    set(CPACK_NSIS_EXECUTABLES_DIRECTORY "${BINDIR}")
    set(CPACK_NSIS_DISPLAY_NAME "Hexahedra")
    set(CPACK_NSIS_PACKAGE_INSTALL_REGISTRY_KEY "hexahedra")
    set(CPACK_NSIS_URL_INFO_ABOUT ${WEBPAGE})
    set(CPACK_NSIS_MODIFY_PATH "OFF")
    set(CPACK_PACKAGE_EXECUTABLES "hexahedra;Hexahedra")
	
	set(CPACK_NSIS_MUI_ICON "${CMAKE_CURRENT_SOURCE_DIR}/install/hexahedra.ico")
	set(CPACK_NSIS_MUI_HEADERIMAGE_BITMAP "${CMAKE_CURRENT_SOURCE_DIR}/install/nsis-header.bmp")
	set(CPACK_NSIS_MUI_WELCOMEFINISHPAGE_BITMAP "${CMAKE_CURRENT_SOURCE_DIR}/install/nsis-welcome.bmp")

	string(REGEX REPLACE "/" "\\\\\\\\" CPACK_NSIS_MUI_HEADERIMAGE_BITMAP ${CPACK_NSIS_MUI_HEADERIMAGE_BITMAP})
	string(REGEX REPLACE "/" "\\\\\\\\" CPACK_NSIS_MUI_WELCOMEFINISHPAGE_BITMAP ${CPACK_NSIS_MUI_WELCOMEFINISHPAGE_BITMAP})
	
    set(CPACK_GENERATOR "NSIS")
endif()

include(CPack)

