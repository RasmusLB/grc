INCLUDE(FindPkgConfig)
PKG_CHECK_MODULES(PC_COMPACTER compacter)

FIND_PATH(
    COMPACTER_INCLUDE_DIRS
    NAMES compacter/api.h
    HINTS $ENV{COMPACTER_DIR}/include
        ${PC_COMPACTER_INCLUDEDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/include
          /usr/local/include
          /usr/include
)

FIND_LIBRARY(
    COMPACTER_LIBRARIES
    NAMES gnuradio-compacter
    HINTS $ENV{COMPACTER_DIR}/lib
        ${PC_COMPACTER_LIBDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/lib
          ${CMAKE_INSTALL_PREFIX}/lib64
          /usr/local/lib
          /usr/local/lib64
          /usr/lib
          /usr/lib64
)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(COMPACTER DEFAULT_MSG COMPACTER_LIBRARIES COMPACTER_INCLUDE_DIRS)
MARK_AS_ADVANCED(COMPACTER_LIBRARIES COMPACTER_INCLUDE_DIRS)

