include_directories(${CMAKE_SOURCE_DIR}/src/cpp/)
include_directories(${CMAKE_SOURCE_DIR}/tst/cpp/)

########### next target ###############
SET(NindAmose_litTexteAnalysej_LIB_SRCS
  NindAmose_litTexteAnalysej.h
  NindAmose_litTexteAnalysej.cpp
)

add_library(NindAmose_litTexteAnalysej SHARED ${NindAmose_litTexteAnalysej_LIB_SRCS})

target_link_libraries(NindAmose_litTexteAnalysej)

set_target_properties(NindAmose_litTexteAnalysej PROPERTIES VERSION 1.1.0 SOVERSION 1)
install(TARGETS NindAmose_litTexteAnalysej DESTINATION lib)

########### next target ###############

SET(NindAmose_indexeCorpus_SRCS
   NindAmose_indexeCorpus.cpp
)

add_executable(NindAmose_indexeCorpus ${NindAmose_indexeCorpus_SRCS})

target_link_libraries(NindAmose_indexeCorpus NindAmose_litTexteAnalysej NindIndexTest NindIndex NindRetrolexicon NindBasics NindAmose)

install(TARGETS NindAmose_indexeCorpus DESTINATION bin)

########### next target ###############

SET(Nind_indexeCorpus_SRCS
   Nind_indexeCorpus.cpp
)

add_executable(Nind_indexeCorpus ${Nind_indexeCorpus_SRCS})

target_link_libraries(Nind_indexeCorpus NindAmose_litTexteAnalysej NindIndexTest NindIndex NindRetrolexicon NindBasics NindAmose)

install(TARGETS Nind_indexeCorpus DESTINATION bin)

########### next target ###############

SET(NindAmose_effaceDocument_SRCS
   NindAmose_effaceDocument.cpp
)

add_executable(NindAmose_effaceDocument ${NindAmose_effaceDocument_SRCS})

target_link_libraries(NindAmose_effaceDocument NindIndex NindRetrolexicon NindBasics NindAmose)

install(TARGETS NindAmose_effaceDocument DESTINATION bin)

########### next target ###############

SET(NindAmose_testLecteur_SRCS
   NindAmose_testLecteur.cpp
)

add_executable(NindAmose_testLecteur ${NindAmose_testLecteur_SRCS})

target_link_libraries(NindAmose_testLecteur NindAmose_litTexteAnalysej NindIndexTest NindIndex NindRetrolexicon NindBasics NindAmose)

install(TARGETS NindAmose_testLecteur DESTINATION bin)

########### next target ###############

SET(NindAmose_testSynchro_SRCS
   NindAmose_testSynchro.cpp
)

add_executable(NindAmose_testSynchro ${NindAmose_testSynchro_SRCS})

target_link_libraries(NindAmose_testSynchro NindAmose_litTexteAnalysej NindIndexTest NindIndex NindRetrolexicon NindBasics NindAmose)

install(TARGETS NindAmose_testSynchro DESTINATION bin)

########### next target ###############