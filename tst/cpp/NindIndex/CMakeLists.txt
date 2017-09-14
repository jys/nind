
include_directories(${CMAKE_SOURCE_DIR}/src/cpp/)

########### next target ###############
SET(NindIndex_litDumpS2_LIB_SRCS
  NindIndex_litDumpS2.h
  NindIndex_litDumpS2.cpp
)

add_library(NindIndex_litDumpS2 SHARED ${NindIndex_litDumpS2_LIB_SRCS})

target_link_libraries(NindIndex_litDumpS2)

set_target_properties(NindIndex_litDumpS2 PROPERTIES VERSION 1.1.0 SOVERSION 1)
install(TARGETS NindIndex_litDumpS2 DESTINATION lib)

########### next target ###############
SET(NindIndexTest_LIB_SRCS
  NindDate.h
  NindDate.cpp
  NindFichiers.h
  NindFichiers.cpp
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
SET(NindIndex_testEcrivain_SRCS
   NindIndex_testEcrivain.cpp
)

add_executable(NindIndex_testEcrivain ${NindIndex_testEcrivain_SRCS})

target_link_libraries(NindIndex_testEcrivain NindIndex_indexe NindIndex_litDumpS2 NindIndexTest NindIndex NindRetrolexicon NindLexicon NindBasics)

install(TARGETS NindIndex_testEcrivain DESTINATION bin)

########### next target ###############
SET(NindIndex_testLecteur_SRCS
   NindIndex_testLecteur.cpp
)

add_executable(NindIndex_testLecteur ${NindIndex_testLecteur_SRCS})

target_link_libraries(NindIndex_testLecteur NindIndex_indexe NindIndex_litDumpS2 NindIndexTest NindIndex NindRetrolexicon NindLexicon NindBasics)

install(TARGETS NindIndex_testLecteur DESTINATION bin)

########### next target ###############
SET(Nind_testLateconNumber_SRCS
   Nind_testLateconNumber.cpp
)

add_executable(Nind_testLateconNumber ${Nind_testLateconNumber_SRCS})

target_link_libraries(Nind_testLateconNumber NindBasics)

install(TARGETS Nind_testLateconNumber DESTINATION bin)

########### next target ###############
SET(NindIndex_testSynchro_SRCS
   NindIndex_testSynchro.cpp
)

add_executable(NindIndex_testSynchro ${NindIndex_testSynchro_SRCS})

target_link_libraries(NindIndex_testSynchro NindIndex_indexe NindIndex_litDumpS2 NindIndexTest NindIndex NindRetrolexicon NindLexicon NindBasics)

install(TARGETS NindIndex_testSynchro DESTINATION bin)

########### next target ###############

