include_directories(${CMAKE_SOURCE_DIR}/src/cpp/ )

########### next target ###############

SET(Nind_test1_SRCS
   Nind_test1.cpp
)

add_executable(Nind_test1 ${Nind_test1_SRCS})

target_link_libraries(Nind_test1 NindLexicon)

install(TARGETS Nind_test1 DESTINATION bin)

########### next target ###############

SET(Nind_testSynchro_SRCS
   Nind_testSynchro.cpp
)

add_executable(Nind_testSynchro ${Nind_testSynchro_SRCS})

target_link_libraries(Nind_testSynchro NindBasics)

install(TARGETS Nind_testSynchro DESTINATION bin)
########### next target ###############

