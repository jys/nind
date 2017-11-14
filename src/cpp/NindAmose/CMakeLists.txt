include_directories(${CMAKE_SOURCE_DIR}/src/cpp )

########### next target ###############

SET(NindAmose_LIB_SRCS
  NindLexiconAmose.cpp
  NindTermAmose.h
  NindTermAmose.cpp
  NindLocalAmose.h
  NindLocalAmose.cpp
)

add_library(NindAmose SHARED ${NindAmose_LIB_SRCS})

target_link_libraries(NindAmose NindIndex NindRetrolexicon)

set_target_properties(NindAmose PROPERTIES VERSION 1.1.0 SOVERSION 1)
install(TARGETS NindAmose DESTINATION lib)

########### install files ###############
install(FILES 
    NindTermAmose.h 
    NindLocalAmose.h 
    NindLexiconAmose.h
  DESTINATION include/NindAmose)
