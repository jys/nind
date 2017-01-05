include_directories(${CMAKE_SOURCE_DIR}/src/cpp )

########### next target ###############

SET(NindRetrolexicon_LIB_SRCS
  NindRetrolexicon.h
  NindRetrolexicon.cpp
)

add_library(NindRetrolexicon SHARED ${NindRetrolexicon_LIB_SRCS})

target_link_libraries(NindRetrolexicon)

set_target_properties(NindRetrolexicon PROPERTIES VERSION 1.1.0 SOVERSION 1)
install(TARGETS NindRetrolexicon DESTINATION lib)

########### install files ###############
install(FILES NindRetrolexicon.h DESTINATION include/NindRetrolexicon)
