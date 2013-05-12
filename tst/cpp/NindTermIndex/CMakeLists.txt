
include_directories(${CMAKE_SOURCE_DIR}/src/cpp/)
########### next target ###############
SET(NindTermIndex_testEcrivain_SRCS
   NindTermIndex_testEcrivain.cpp
)

add_executable(NindTermIndex_testEcrivain ${NindTermIndex_testEcrivain_SRCS})

target_link_libraries(NindTermIndex_testEcrivain NindTermIndex NindLexicon NindBasics)

install(TARGETS NindTermIndex_testEcrivain DESTINATION bin)

########### next target ###############

