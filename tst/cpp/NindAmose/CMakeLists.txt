include_directories(${CMAKE_SOURCE_DIR}/src/cpp/)

########### next target ###############

SET(NindAmose_indexeCorpus_SRCS
   NindAmose_indexeCorpus.cpp
)

add_executable(NindAmose_indexeCorpus ${NindAmose_indexeCorpus_SRCS})

target_link_libraries(NindAmose_indexeCorpus NindIndex NindBasics NindAmose)

install(TARGETS NindAmose_indexeCorpus DESTINATION bin)

########### next target ###############

SET(NindAmose_effaceDocument_SRCS
   NindAmose_effaceDocument.cpp
)

add_executable(NindAmose_effaceDocument ${NindAmose_effaceDocument_SRCS})

target_link_libraries(NindAmose_effaceDocument NindIndex NindBasics NindAmose)

install(TARGETS NindAmose_effaceDocument DESTINATION bin)

########### next target ###############

SET(NindAmose_testLecteur_SRCS
   NindAmose_testLecteur.cpp
)

add_executable(NindAmose_testLecteur ${NindAmose_testLecteur_SRCS})

target_link_libraries(NindAmose_testLecteur NindIndex NindBasics NindAmose)

install(TARGETS NindAmose_testLecteur DESTINATION bin)

########### next target ###############