# Try to find Nind headers and library.
#
# Variables defined by this module:
#
#  Nind_FOUND               System has Nind library/headers.
#  Nind_LIBRARIES           The Nind library.
#  Nind_INCLUDE_DIRS        The location of Nind headers.

find_path(Nind_ROOT_DIR
  NAMES include/NindCommonExport.h
)

find_path(Nind_INCLUDE_DIR
  NAMES NindCommonExport.h
  HINTS ${Nind_ROOT_DIR}/include
)

find_library(Nind_AMOSE_LIBRARY
  NAMES NindAmose
  HINTS ${Nind_ROOT_DIR}/lib
)

find_library(Nind_BASICS_LIBRARY
  NAMES NindBasics
  HINTS ${Nind_ROOT_DIR}/lib
)

find_library(Nind_INDEX_INDEXE_LIBRARY
  NAMES NindIndex_indexe
  HINTS ${Nind_ROOT_DIR}/lib
)

find_library(Nind_INDEX_LIBRARY
  NAMES NindIndex
  HINTS ${Nind_ROOT_DIR}/lib
)

find_library(Nind_INDEX_TEST_LIBRARY
  NAMES NindIndexTest
  HINTS ${Nind_ROOT_DIR}/lib
)

find_library(Nind_RETROLEXICON_LIBRARY
  NAMES NindRetrolexicon
  HINTS ${Nind_ROOT_DIR}/lib
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Nind DEFAULT_MSG
  Nind_INCLUDE_DIR
  Nind_AMOSE_LIBRARY
  Nind_BASICS_LIBRARY
  Nind_INDEX_INDEXE_LIBRARY
  Nind_INDEX_LIBRARY
  Nind_INDEX_TEST_LIBRARY
  Nind_RETROLEXICON_LIBRARY
)

mark_as_advanced(
  Nind_INCLUDE_DIR
  Nind_AMOSE_LIBRARY
  Nind_BASICS_LIBRARY
  Nind_INDEX_INDEXE_LIBRARY
  Nind_INDEX_LIBRARY
  Nind_INDEX_TEST_LIBRARY
  Nind_RETROLEXICON_LIBRARY
)

set(Nind_INCLUDE_DIRS ${Nind_INCLUDE_DIR})
set(Nind_LIBRARIES
  ${Nind_AMOSE_LIBRARY}
  ${Nind_BASICS_LIBRARY}
  ${Nind_INDEX_INDEXE_LIBRARY}
  ${Nind_INDEX_LIBRARY}
  ${Nind_INDEX_TEST_LIBRARY}
  ${Nind_RETROLEXICON_LIBRARY}
)

