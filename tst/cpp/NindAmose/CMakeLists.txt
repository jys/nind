include_directories(${CMAKE_SOURCE_DIR}/src/cpp/)

########### next target ###############

SET(NindAmose_indexeCorpus_SRCS
   NindAmose_indexeCorpus.cpp
)

add_executable(NindAmose_indexeCorpus ${NindAmose_indexeCorpus_SRCS})

target_link_libraries(NindAmose_indexeCorpus NindIndex NindBasics)

install(TARGETS NindAmose_indexeCorpus DESTINATION bin)

########### next target ###############