include_directories(${CMAKE_SOURCE_DIR}/src/cpp )

########### next target ###############

SET(NindIndex_LIB_SRCS
  NindIndex.h
  NindIndex.cpp
  NindLexiconIndex.h
  NindLexiconIndex.cpp
  NindRetrolexiconIndex.h
  NindRetrolexiconIndex.cpp
  NindTermIndex.h
  NindTermIndex.cpp
  NindLocalIndex.h
  NindLocalIndex.cpp
)

add_library(NindIndex SHARED ${NindIndex_LIB_SRCS})

target_link_libraries(NindIndex)

set_target_properties(NindIndex PROPERTIES VERSION 1.1.0 SOVERSION 1)
install(TARGETS NindIndex DESTINATION lib)

########### install files ###############
install(FILES NindIndex.h NindLexiconIndex.h NindRetrolexiconIndex.h NindTermIndex.h NindLocalIndex.h DESTINATION include/NindIndex)
