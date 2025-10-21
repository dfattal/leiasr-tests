#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "srCameras::srCameras" for configuration "Release"
set_property(TARGET srCameras::srCameras APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(srCameras::srCameras PROPERTIES
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/lib/SimulatedRealityCameras.lib"
  
  )

list(APPEND _IMPORT_CHECK_TARGETS srCameras::srCameras )
list(APPEND _IMPORT_CHECK_FILES_FOR_srCameras::srCameras "${_IMPORT_PREFIX}/lib/SimulatedRealityCameras.lib"  )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
