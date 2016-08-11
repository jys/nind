
include_directories(${CMAKE_SOURCE_DIR}/src/cpp/)


########### next target ###############
SET(NindIndexTest_LIB_SRCS
  NindIndexTest.h
  NindIndexTest.cpp
  NindDate.h
  NindDate.cpp
)

add_library(NindIndexTest SHARED ${NindIndexTest_LIB_SRCS})

target_link_libraries(NindIndexTest)

set_target_properties(NindIndexTest PROPERTIES VERSION 1.1.0 SOVERSION 1)
install(TARGETS NindIndexTest DESTINATION lib)

########### next target ###############
SET(NindIndex_indexe_LIB_SRCS
  NindIndex_indexe.h
  NindIndex_indexe.cpp
)

add_library(NindIndex_indexe SHARED ${NindIndex_indexe_LIB_SRCS})

target_link_libraries(NindIndex_indexe)

set_target_properties(NindIndex_indexe PROPERTIES VERSION 1.1.0 SOVERSION 1)
install(TARGETS NindIndex_indexe DESTINATION lib)

########### next target ###############
SET(NindTermIndex_testEcrivain_SRCS
   NindTermIndex_testEcrivain.cpp
)

add_executable(NindTermIndex_testEcrivain ${NindTermIndex_testEcrivain_SRCS})

target_link_libraries(NindTermIndex_testEcrivain NindIndexTest NindIndex NindLexicon NindBasics)

install(TARGETS NindTermIndex_testEcrivain DESTINATION bin)

########### next target ###############
SET(NindIndex_testEcrivain_SRCS
   NindIndex_testEcrivain.cpp
)

add_executable(NindIndex_testEcrivain ${NindIndex_testEcrivain_SRCS})

target_link_libraries(NindIndex_testEcrivain NindIndex_indexe NindIndexTest NindIndex NindLexicon NindBasics)

install(TARGETS NindIndex_testEcrivain DESTINATION bin)

########### next target ###############
SET(NindIndex_testEcrivainB_SRCS
   NindIndex_testEcrivainB.cpp
)

add_executable(NindIndex_testEcrivainB ${NindIndex_testEcrivainB_SRCS})

target_link_libraries(NindIndex_testEcrivainB NindIndexTest NindIndex NindLexicon NindBasics)

install(TARGETS NindIndex_testEcrivainB DESTINATION bin)

########### next target ###############
SET(NindTermIndex_testLecteur_SRCS
   NindTermIndex_testLecteur.cpp
)

add_executable(NindTermIndex_testLecteur ${NindTermIndex_testLecteur_SRCS})

target_link_libraries(NindTermIndex_testLecteur NindIndexTest NindIndex NindLexicon NindBasics)

install(TARGETS NindTermIndex_testLecteur DESTINATION bin)

########### next target ###############
SET(NindIndex_testLecteur_SRCS
   NindIndex_testLecteur.cpp
)

add_executable(NindIndex_testLecteur ${NindIndex_testLecteur_SRCS})

target_link_libraries(NindIndex_testLecteur NindIndexTest NindIndex NindLexicon NindBasics)

install(TARGETS NindIndex_testLecteur DESTINATION bin)

########### next target ###############
SET(Nind_testLateconNumber_SRCS
   Nind_testLateconNumber.cpp
)

add_executable(Nind_testLateconNumber ${Nind_testLateconNumber_SRCS})

target_link_libraries(Nind_testLateconNumber NindBasics)

install(TARGETS Nind_testLateconNumber DESTINATION bin)

########### next target ###############
SET(Nind_testNumbersInLocalIndexes_SRCS
   Nind_testNumbersInLocalIndexes.cpp
)

add_executable(Nind_testNumbersInLocalIndexes ${Nind_testNumbersInLocalIndexes_SRCS})

target_link_libraries(Nind_testNumbersInLocalIndexes NindIndexTest NindLexicon NindBasics)

install(TARGETS Nind_testNumbersInLocalIndexes DESTINATION bin)

########### next target ###############
SET(NindLexiconIndex_testEcrivain_SRCS
   NindLexiconIndex_testEcrivain.cpp
)

add_executable(NindLexiconIndex_testEcrivain ${NindLexiconIndex_testEcrivain_SRCS})

target_link_libraries(NindLexiconIndex_testEcrivain NindIndexTest NindIndex NindBasics)

install(TARGETS NindLexiconIndex_testEcrivain DESTINATION bin)

########### next target ###############
SET(NindLexiconIndex_testLecteur_SRCS
   NindLexiconIndex_testLecteur.cpp
)

add_executable(NindLexiconIndex_testLecteur ${NindLexiconIndex_testLecteur_SRCS})

target_link_libraries(NindLexiconIndex_testLecteur NindIndexTest NindIndex NindBasics)

install(TARGETS NindLexiconIndex_testLecteur DESTINATION bin)

########### next target ###############

