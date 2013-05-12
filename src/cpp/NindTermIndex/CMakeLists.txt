include_directories(${CMAKE_SOURCE_DIR}/src/cpp )

########### next target ###############

SET(NindTermIndex_LIB_SRCS
  NindTermIndex.h
  NindTermIndex.cpp
  NindTermIndexFile.h
  NindTermIndexFile.cpp
  NindTerm.h
  NindTerm.cpp
)

add_library(NindTermIndex SHARED ${NindTermIndex_LIB_SRCS})

target_link_libraries(NindTermIndex)

set_target_properties(NindTermIndex PROPERTIES VERSION 1.1.0 SOVERSION 1)
install(TARGETS NindTermIndex DESTINATION lib)

########### install files ###############
