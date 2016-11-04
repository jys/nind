include_directories(${CMAKE_SOURCE_DIR}/src/cpp )

########### next target ###############

SET(NindLexicon_LIB_SRCS
  NindLexicon.h
  NindLexicon.cpp
  NindLexiconFile.h
  NindLexiconFile.cpp
)

add_library(NindLexicon SHARED ${NindLexicon_LIB_SRCS})

target_link_libraries(NindLexicon)

set_target_properties(NindLexicon PROPERTIES VERSION 1.1.0 SOVERSION 1)
install(TARGETS NindLexicon DESTINATION lib)

########### install files ###############
install(FILES NindLexicon.h NindLexiconFile.h DESTINATION include/NindLexicon)

