
include_directories(${CMAKE_SOURCE_DIR}/src/cpp/)
########### next target ###############
SET(NindLexicon_testEcrivain_SRCS
   NindLexicon_testEcrivain.cpp
)

add_executable(NindLexicon_testEcrivain ${NindLexicon_testEcrivain_SRCS})

target_link_libraries(NindLexicon_testEcrivain NindLexicon NindBasics)

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
