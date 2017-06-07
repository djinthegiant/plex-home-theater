#.rst:
# FindRKMPP
# ----------
# Finds the Rockchip MPP library
#
# This will will define the following variables::
#
# RKMPP_FOUND - system has RKMPP
# RKMPP_DEFINITIONS - the RKMPP definitions

if(PKG_CONFIG_FOUND)
  pkg_check_modules(PC_RKMPP rockchip_mpp QUIET)
endif()

find_library(RKMPP_LIBRARY NAMES rockchip_mpp
                           PATHS ${PC_RKMPP_LIBDIR})

set(RKMPP_VERSION ${PC_RKMPP_VERSION})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(RKMPP
                                  REQUIRED_VARS RKMPP_LIBRARY
                                  VERSION_VAR RKMPP_VERSION)

if(RKMPP_FOUND)
  set(RKMPP_DEFINITIONS -DHAS_RKMPP=1)
endif()

mark_as_advanced(RKMPP_LIBRARY)
