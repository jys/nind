
include_directories(${CMAKE_SOURCE_DIR}/src/cpp/ )
#include_directories(/home/jys/myusr/include/ )
#link_directories(/home/jys/myusr/lib/ )

########### next target ###############

SET(NindLexicon_test1_SRCS
   NindLexicon_test1.cpp
)

add_executable(NindLexicon_test1 ${NindLexicon_test1_SRCS})

target_link_libraries(NindLexicon_test1 NindLexicon)

install(TARGETS NindLexicon_test1 DESTINATION bin)

########### next target ###############
SET(NindLexicon_test2_SRCS
   NindLexicon_test2.cpp
)

add_executable(NindLexicon_test2 ${NindLexicon_test2_SRCS})

target_link_libraries(NindLexicon_test2 NindLexicon)

install(TARGETS NindLexicon_test2 DESTINATION bin)

########### next target ###############
SET(NindLexicon_test3_SRCS
   NindLexicon_test3.cpp
)

add_executable(NindLexicon_test3 ${NindLexicon_test3_SRCS})

target_link_libraries(NindLexicon_test3 NindLexicon)

install(TARGETS NindLexicon_test3 DESTINATION bin)

########### next target ###############
SET(NindLexicon_test4_SRCS
   NindLexicon_test4.cpp
)

add_executable(NindLexicon_test4 ${NindLexicon_test4_SRCS})

target_link_libraries(NindLexicon_test4 NindLexicon)

install(TARGETS NindLexicon_test4 DESTINATION bin)

########### next target ###############
