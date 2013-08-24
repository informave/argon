# Finds the DBWTL library
#
#  DBWTL_FOUND          - True if DBWTL found.
#  DBWTL_INCLUDE_DIRS   - Directory to include to get dbwtl headers
#  DBWTL_LIBRARIES      - Libraries to link against for the dbwtl

# Look for the header file.
find_path(
  DBWTL_INCLUDE_DIR 
  NAMES dbwtl/dbobjects
  DOC "Include directory for the DBWTL library")
mark_as_advanced(DBWTL_INCLUDE_DIR)

# Look for the library.
find_library(
  DBWTL_LIBRARY
  NAMES dbwtl
  DOC "Libraries to link against DBWTL")
mark_as_advanced(DBWTL_LIBRARY)

# Copy the results to the output variables.
if(DBWTL_INCLUDE_DIR AND DBWTL_LIBRARY)
  set(DBWTL_FOUND 1)
  set(DBWTL_LIBRARIES ${DBWTL_LIBRARY})
  set(DBWTL_INCLUDE_DIRS ${DBWTL_INCLUDE_DIR})

else(DBWTL_INCLUDE_DIR AND DBWTL_LIBRARY)
  set(DBWTL_FOUND 0)
  set(DBWTL_LIBRARIES)
  set(DBWTL_INCLUDE_DIRS)
endif(DBWTL_INCLUDE_DIR AND DBWTL_LIBRARY)

IF(DBWTL_FOUND)
  IF( NOT DBWTL_FIND_QUIETLY )
    MESSAGE( STATUS "Found DBWTL header files in ${DBWTL_INCLUDE_DIRS}")
    MESSAGE( STATUS "Found DBWTL libraries: ${DBWTL_LIBRARIES}")
  ENDIF( NOT DBWTL_FIND_QUIETLY )
ELSE(DBWTL_FOUND)
	IF(DBWTL_FIND_REQUIRED)
		MESSAGE( FATAL_ERROR "Could not find DBWTL" )
	ELSE(DBWTL_FIND_REQUIRED)
		MESSAGE( STATUS "Optional package DBWTL was not found" )
	ENDIF(DBWTL_FIND_REQUIRED)
ENDIF(DBWTL_FOUND)

