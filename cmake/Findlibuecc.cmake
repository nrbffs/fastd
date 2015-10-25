# Defines the following variables:
#  LIBUECC_FOUND
#  LIBUECC_INCLUDE_DIR
#  LIBUECC_LIBRARIES
#  LIBUECC_CFLAGS_OTHER
#  LIBUECC_LDFLAGS_OTHER


if(ANDROID)
  find_host_package(PkgConfig REQUIRED QUIET)
else(ANDROID)
  find_package(PkgConfig REQUIRED QUIET)
endif(ANDROID)

pkg_check_modules(_LIBUECC libuecc>=6)

find_path(LIBUECC_INCLUDE_DIR NAMES libuecc/ecc.h HINTS ${_LIBUECC_INCLUDE_DIRS})
find_library(LIBUECC_LIBRARIES NAMES uecc HINTS ${_LIBUECC_LIBRARY_DIRS})

set(LIBUECC_CFLAGS_OTHER "${_LIBUECC_CFLAGS_OTHER}" CACHE STRING "Additional compiler flags for libuecc")
set(LIBUECC_LDFLAGS_OTHER "${_LIBUECC_LDFLAGS_OTHER}" CACHE STRING "Additional linker flags for libuecc")

find_package_handle_standard_args(LIBUECC REQUIRED_VARS LIBUECC_LIBRARIES LIBUECC_INCLUDE_DIR)
mark_as_advanced(LIBUECC_INCLUDE_DIR LIBUECC_LIBRARIES LIBUECC_CFLAGS_OTHER LIBUECC_LDFLAGS_OTHER)
