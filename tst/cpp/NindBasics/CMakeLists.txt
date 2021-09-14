include_directories(${CMAKE_SOURCE_DIR}/src/cpp/)
include_directories(${CMAKE_SOURCE_DIR}/tst/cpp/)

########### next target ###############
SET(NindBasics_testLecture_SRCS
   NindBasics_testLecture.cpp
)

add_executable(NindBasics_testLecture ${NindBasics_testLecture_SRCS})

target_link_libraries(NindBasics)

install(TARGETS NindBasics_testLecture DESTINATION bin)
