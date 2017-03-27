#.rst:
# FindRK
# -------
# Finds the Rockchip MPP
#
# This will will define the following variables::
#
# RK_FOUND - system has RK
# RK_LIBRARIES - the RK libraries
# RK_DEFINITIONS - the RK definitions

if(PKG_CONFIG_FOUND)
  pkg_check_modules(RK rockchip_mpp QUIET)
endif()

find_library(RKMPP_LIBRARY NAMES rockchip_mpp
                           PATHS ${PC_RK_LIBDIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(RK
                                  REQUIRED_VARS RKMPP_LIBRARY)

if(RK_FOUND)
  set(RK_LIBRARIES ${RKMPP_LIBRARY})
  set(RK_DEFINITIONS -DHAS_RKMPP=1)
endif()

mark_as_advanced(RKMPP_LIBRARY)
