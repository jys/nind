
include_directories(${CMAKE_SOURCE_DIR}/src/cpp/)
include_directories(${CMAKE_SOURCE_DIR}/tst/cpp/)
########### next target ###############
SET(NindLexicon_testEcrivain_SRCS
   NindLexicon_testEcrivain.cpp
)

add_executable(NindLexicon_testEcrivain ${NindLexicon_testEcrivain_SRCS})

target_link_libraries(NindLexicon_testEcrivain NindIndex_litDumpS2 NindIndexTest NindLexicon NindBasics)

install(TARGETS NindLexicon_testEcrivain DESTINATION bin)

########### next target ###############
SET(NindLexicon_testLecteur_SRCS
   NindLexicon_testLecteur.cpp
)

add_executable(NindLexicon_testLecteur ${NindLexicon_testLecteur_SRCS})

target_link_libraries(NindLexicon_testLecteur NindLexicon NindBasics)

install(TARGETS NindLexicon_testLecteur DESTINATION bin)

########### next target ###############
SET(NindLexicon_testInter_SRCS
   NindLexicon_testInter.cpp
)

add_executable(NindLexicon_testInter ${NindLexicon_testInter_SRCS})

target_link_libraries(NindLexicon_testInter NindLexicon NindBasics)

install(TARGETS NindLexicon_testInter DESTINATION bin)

########### next target ###############
SET(NindLexicon_testLimites_SRCS
   NindLexicon_testLimites.cpp
)

add_executable(NindLexicon_testLimites ${NindLexicon_testLimites_SRCS})

target_link_libraries(NindLexicon_testLimites NindLexicon NindBasics)

install(TARGETS NindLexicon_testLimites DESTINATION bin)

########### next target ###############
SET(NindLexicon_testLimitesB_SRCS
   NindLexicon_testLimitesB.cpp
)

add_executable(NindLexicon_testLimitesB ${NindLexicon_testLimitesB_SRCS})

target_link_libraries(NindLexicon_testLimitesB NindLexicon NindBasics)

install(TARGETS NindLexicon_testLimitesB DESTINATION bin)

########### next target ###############
SET(NindLexicon_testLimitesC_SRCS
   NindLexicon_testLimitesC.cpp
)

add_executable(NindLexicon_testLimitesC ${NindLexicon_testLimitesC_SRCS})

target_link_libraries(NindLexicon_testLimitesC NindLexicon NindBasics)

install(TARGETS NindLexicon_testLimitesC DESTINATION bin)

########### next target ###############
SET(NindLexicon_testMemoire_SRCS
   NindLexicon_testMemoire.cpp
)

add_executable(NindLexicon_testMemoire ${NindLexicon_testMemoire_SRCS})

target_link_libraries(NindLexicon_testMemoire NindLexicon NindBasics)

install(TARGETS NindLexicon_testMemoire DESTINATION bin)

########### next target ###############
