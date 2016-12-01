include_directories(${CMAKE_SOURCE_DIR}/src/cpp/)

########### next target ###############

SET(NindAmose_indexeCorpus_SRCS
   NindAmose_indexeCorpus.cpp
)

add_executable(NindAmose_indexeCorpus ${NindAmose_indexeCorpus_SRCS})

target_link_libraries(NindAmose_indexeCorpus NindIndex NindBasics NindAmose)

install(TARGETS NindAmose_indexeCorpus DESTINATION bin)

########### next target ###############

SET(Nind_indexeCorpus_SRCS
   Nind_indexeCorpus.cpp
)

add_executable(Nind_indexeCorpus ${Nind_indexeCorpus_SRCS})

target_link_libraries(Nind_indexeCorpus NindIndex NindBasics NindAmose)

install(TARGETS Nind_indexeCorpus DESTINATION bin)

########### next target ###############

SET(NindAmose_effaceDocument_SRCS
   NindAmose_effaceDocument.cpp
)

add_executable(NindAmose_effaceDocument ${NindAmose_effaceDocument_SRCS})

target_link_libraries(NindAmose_effaceDocument NindIndex NindBasics NindAmose)

install(TARGETS NindAmose_effaceDocument DESTINATION bin)

########### next target ###############