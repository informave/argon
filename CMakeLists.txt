#
# Project file for ARGON
#

cmake_minimum_required(VERSION 2.6.1)

# project name
project(argon)

# versions
set(${PROJECT_NAME}_MAJOR_VERSION 0)
set(${PROJECT_NAME}_MINOR_VERSION 1)
set(${PROJECT_NAME}_PATCH_LEVEL 0)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)


set(MEMORYCHECK_COMMAND_OPTIONS "--leak-check=full --show-possibly-lost=yes
-show-reachable=no")


if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR AND NOT MSVC_IDE)
  message(FATAL_ERROR "In-source builds are not allowed.
  Please create a directory and run cmake from there, passing the path
  to this source directory as the last argument.
  This process created the file `CMakeCache.txt' and the directory
  `CMakeFiles'.
  Please delete them.")
endif()


# library Unix style versioning
set(${PROJECT_NAME}_SOVERSION
	"${${PROJECT_NAME}_MAJOR_VERSION}")
set(${PROJECT_NAME}_VERSION
	"${${PROJECT_NAME}_MAJOR_VERSION}.${${PROJECT_NAME}_MINOR_VERSION}.${${PROJECT_NAME}_PATCH_LEVEL}")

#set(CMAKE_VERBOSE_MAKEFILE true)


option(ARGON_ENABLE_PEDANTIC "Compile with pedantic enabled." OFF)
option(ARGON_ENABLE_WERROR "Fail and stop if a warning is triggered." OFF)

if(MSVC)
   option(ARGON_ENABLE_WARNINGS "Enable compiler warnings." OFF)
else(MSVC)
   option(ARGON_ENABLE_WARNINGS "Enable compiler warnings." ON)
endif()



option(ARGON_CXX98_COMPATIBILITY
	"Use C++98 header and features instead of the current standard
	version" ON)



set(ARGON_DEFAULT_CHARSET "UTF-8" CACHE STRING "This charset is used for input 1-byte chars when converting to the internal wide charset")

set(ARGON_INTERNAL_CHARTYPE "2" CACHE STRING "This char type is
internally used (only change if you really know what you are doing)")


option(ARGON_WITH_TESTS
        "Compile and run tests" OFF)



#select plattform
if(WIN32)
  if(CYGWIN)
    set(ARGON_ON_WIN32 0)
    set(ARGON_ON_UNIX 1)
  else(CYGWIN)
    set(ARGON_ON_WIN32 1)
    set(ARGON_ON_UNIX 0)
  endif(CYGWIN)
  set(LTDL_SHLIB_EXT ".dll")
  set(EXEEXT ".exe")
else(WIN32)
  if(UNIX)
    set(ARGON_ON_WIN32 0)
    set(ARGON_ON_UNIX 1)
    if(APPLE)
      set(LTDL_SHLIB_EXT ".dylib")
    else(APPLE)
      set(LTDL_SHLIB_EXT ".so")
    endif(APPLE)
    set(EXEEXT "")
  else(UNIX)
    MESSAGE(SEND_ERROR "Unable to determine platform")
  endif(UNIX)
endif(WIN32)


#special options for MSVC
if(MSVC)
  # List of valid CRTs for MSVC
  set(MSVC_CRT
      MD
      MDd
      MT
      MTd)

  set(ARGON_USE_CRT "" CACHE STRING "Specify VC++ CRT to use for debug/release configurations.")

  add_definitions("-D_CRT_SECURE_NO_DEPRECATE -D_CRT_SECURE_NO_WARNINGS")
  add_definitions("-D_SCL_SECURE_NO_WARNINGS -DCRT_NONSTDC_NO_WARNINGS")
  add_definitions("-D_SCL_SECURE_NO_DEPRECATE")
  add_definitions("-wd4309 -wd4251 -wd4099 -wd4800")

  if(NOT ${ARGON_USE_CRT} STREQUAL "")
    list(FIND MSVC_CRT ${ARGON_USE_CRT} idx)
    if(idx LESS 0)
      message(FATAL_ERROR "Invalid value for ARGON_USE_CRT: ${ARGON_USE_CRT}. Valid options are one of: ${MSVC_CRT}")
    endif(idx LESS 0)
    add_definitions("/${ARGON_USE_CRT}")
    message(STATUS "Using VC++ CRT: ${ARGON_USE_CRT}")
  endif(NOT ${ARGON_USE_CRT} STREQUAL "")

  # Enable warnings
  if(ARGON_ENABLE_WARNINGS)
    add_definitions("/W4 /Wall")
    if(ARGON_ENABLE_PEDANTIC)
      # No MSVC equivalent available
    endif(ARGON_ENABLE_PEDANTIC)
  endif(ARGON_ENABLE_WARNINGS)
  if(ARGON_ENABLE_WERROR)
    add_definitions("/WX")
  endif(ARGON_ENABLE_WERROR)

elseif(CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-non-virtual-dtor -Wno-deprecated")
  if (ARGON_ENABLE_WARNINGS)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -W -Wextra -Wno-unused-parameter  -Wwrite-strings  -Wno-unused -Wno-switch-enum")
    if(ARGON_ENABLE_PEDANTIC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -Wno-long-long -Weffc++ -Wno-non-virtual-dtor")
    endif(ARGON_ENABLE_PEDANTIC)
  endif(ARGON_ENABLE_WARNINGS)
  if(ARGON_ENABLE_WERROR)
    add_definitions("-Werror")
  endif(ARGON_ENABLE_WERROR)
endif(MSVC)



#set(PACKAGE_NAME argon)
#set(PACKAGE_VERSION 0.0.1)
#set(PACKAGE_STRING "${PACKAGE_NAME} ${PACKAGE_VERSION}")
#set(PACKAGE_BUGREPORT "bugs@informave.org")

# Directories
set(ARGON_MAIN_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(ARGON_MAIN_DOC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/doc)
set(ARGON_MAIN_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(ARGON_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})

# Include directories
include_directories( ${ARGON_MAIN_INCLUDE_DIR} )
include_directories( ${ARGON_BINARY_DIR} )


if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release CACHE STRING
		"Choose the type of build, options are: None Debug Release."
		FORCE)
endif(NOT CMAKE_BUILD_TYPE)

string(TOUPPER "${CMAKE_BUILD_TYPE}" uppercase_CMAKE_BUILD_TYPE)


if(ARGON_USE_LOCAL_HEADERS)
	set(ARGON_LOCAL_HEADERS_INCLUDE_DIR
	"${ARGON_MAIN_INCLUDE_DIR}/headers")
        include_directories(${ARGON_MAIN_INCLUDE_DIR}/headers)
endif()



include(CheckIncludeFiles)


check_include_files("stdint.h" ARGON_HAVE_STDINT_H)







configure_file(
  "${ARGON_MAIN_INCLUDE_DIR}/argon/argon_config.hh.in"
  "${PROJECT_BINARY_DIR}/argon/argon_config.hh"
  )
  
# Version info resource
if(MSVC)  
	configure_file(
 		"${ARGON_MAIN_INCLUDE_DIR}/version.rc.in"
  		"${PROJECT_BINARY_DIR}/version.rc"
)  
endif(MSVC)




if(NOT MSVC)
	add_subdirectory(doc)
endif()
  

#include Debug information
if(CMAKE_COMPILER_IS_GNUCXX AND (uppercase_CMAKE_BUILD_TYPE STREQUAL "DEBUG"))
	add_definitions("-O1 -g")
endif()


# Lemon parser
add_executable(lemon ${CMAKE_CURRENT_SOURCE_DIR}/lemon/lemon.c)


add_custom_command(
	OUTPUT ${ARGON_MAIN_SRC_DIR}/parser.cc
	COMMAND lemon -p -s T=${CMAKE_CURRENT_SOURCE_DIR}/lemon/lempar.c ${ARGON_MAIN_SRC_DIR}/parser.y
	DEPENDS lemon ${ARGON_MAIN_SRC_DIR}/parser.y
	${CMAKE_CURRENT_SOURCE_DIR}/lemon/lempar.c
	${CMAKE_CURRENT_SOURCE_DIR}/include/argon/ast.hh # depend workaround cmake
	

)




# Source files
set(argon_srcs
	${ARGON_MAIN_SRC_DIR}/parser.cc # generated by lemon
	${ARGON_MAIN_SRC_DIR}/token.cc
	${ARGON_MAIN_SRC_DIR}/ast.cc
	${ARGON_MAIN_SRC_DIR}/elements.cc
	${ARGON_MAIN_SRC_DIR}/symboltable.cc
	${ARGON_MAIN_SRC_DIR}/typetable.cc
	${ARGON_MAIN_SRC_DIR}/pass1visitor.cc
	${ARGON_MAIN_SRC_DIR}/pass2visitor.cc
	${ARGON_MAIN_SRC_DIR}/task.cc
	${ARGON_MAIN_SRC_DIR}/voidtask.cc
	${ARGON_MAIN_SRC_DIR}/storetask.cc
	${ARGON_MAIN_SRC_DIR}/visitors.cc
	${ARGON_MAIN_SRC_DIR}/logcmd.cc
	${ARGON_MAIN_SRC_DIR}/sqlexec.cc
	${ARGON_MAIN_SRC_DIR}/connection.cc
	${ARGON_MAIN_SRC_DIR}/genrange.cc
	${ARGON_MAIN_SRC_DIR}/expand.cc
	${ARGON_MAIN_SRC_DIR}/compact.cc
	${ARGON_MAIN_SRC_DIR}/table.cc
	${ARGON_MAIN_SRC_DIR}/table_sqlite.cc
	${ARGON_MAIN_SRC_DIR}/table_firebird.cc
	${ARGON_MAIN_SRC_DIR}/sql.cc
	${ARGON_MAIN_SRC_DIR}/transfertask.cc
	${ARGON_MAIN_SRC_DIR}/fetchtask.cc
	${ARGON_MAIN_SRC_DIR}/semantic.cc
	${ARGON_MAIN_SRC_DIR}/processor.cc
	${ARGON_MAIN_SRC_DIR}/dtsengine.cc
	${ARGON_MAIN_SRC_DIR}/exceptions.cc
	${ARGON_MAIN_SRC_DIR}/value.cc
	${ARGON_MAIN_SRC_DIR}/rtti.cc
	${ARGON_MAIN_SRC_DIR}/function.cc
	${ARGON_MAIN_SRC_DIR}/globalcontext.cc
	${ARGON_MAIN_SRC_DIR}/lambda.cc
	${ARGON_MAIN_SRC_DIR}/sequence.cc
	${ARGON_MAIN_SRC_DIR}/builtin/string.cc
	${ARGON_MAIN_SRC_DIR}/builtin/sys.cc
	${ARGON_MAIN_SRC_DIR}/builtin/debug.cc
	${ARGON_MAIN_SRC_DIR}/builtin/sql.cc
	${ARGON_MAIN_SRC_DIR}/builtin/seq.cc
	)


if(MSVC)  
	set(argon_srcs
		"${PROJECT_BINARY_DIR}/version.rc"
  		${argon_srcs})
endif(MSVC)


# library
add_library(argon SHARED ${argon_srcs})



# ICU Library
set(DBWTL_FIND_REQUIRED true)
set(DBWTL_FIND_QUIETLY false)


include(FindDBWTL)


include_directories(${DBWTL_INCLUDE_DIRS})
target_link_libraries(argon ${DBWTL_LIBRARIES})


#target_link_libraries(argon dbwtl)


#SET_TARGET_PROPERTIES(argon PROPERTIES LINK_FLAGS "-L/home/cytrinox/bin/usr/lib")

if(UNIX)
SET (CMAKE_SHARED_LINKER_FLAGS ${CMAKE_SHARED_LINKER_FLAGS_INIT}
"-L/home/cytrinox/bin/usr/lib")

SET (CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS_INIT}
"-L/home/cytrinox/bin/usr/lib")
endif()

# CLI
set(argoncli_srcs
	${ARGON_MAIN_SRC_DIR}/cli/climain.cc
)

add_executable(argoncli ${argoncli_srcs})


target_link_libraries(argoncli argon)


if(UNIX)
	target_link_libraries(argon)
endif()


set_target_properties(argon PROPERTIES
	VERSION ${${PROJECT_NAME}_VERSION}
	SOVERSION ${${PROJECT_NAME}_SOVERSION}
)


set(WITH_DOXYFILE_PDF false)

if(NOT MSVC)
	include(UseDoxygen)
endif()



if(UNIX)
	install(TARGETS argon LIBRARY DESTINATION lib)
endif(UNIX)

if(NOT UNIX)
	install(TARGETS argon DESTINATION lib)
endif(NOT UNIX)

install(DIRECTORY ${ARGON_MAIN_INCLUDE_DIR}/argon
  DESTINATION include
  FILES_MATCHING
  PATTERN "*.hh"
  REGEX "^[^\\.]*$"
  PATTERN ".git*" EXCLUDE
)

install(FILES "${PROJECT_BINARY_DIR}/argon/argon_config.hh"
	DESTINATION include/argon)


#include Tests
if(ARGON_WITH_TESTS)
	include(CTest)
	add_subdirectory(tests)
endif(ARGON_WITH_TESTS)

