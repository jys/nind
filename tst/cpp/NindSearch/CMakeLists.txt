include_directories(${CMAKE_SOURCE_DIR}/src/cpp/)
include_directories(${CMAKE_SOURCE_DIR}/tst/cpp/)

########### next target ###############
SET(NindSearch_SRCS
   NindSearch.cpp
)

add_executable(NindSearch ${NindSearch_SRCS})

target_link_libraries(NindSearch NindIndexTest NindIndex NindBasics)

install(TARGETS NindSearch DESTINATION bin)

########### next target ###############
