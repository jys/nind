include_directories(${CMAKE_SOURCE_DIR}/src/cpp )

########### next target ###############

SET(NindBasics_LIB_SRCS
  NindFile.h
  NindFile.cpp
  NindPadFile.h
  NindPadFile.cpp
  NindSignalCatcher.h
  NindSignalCatcher.cpp
)

add_library(NindBasics SHARED ${NindBasics_LIB_SRCS})

target_link_libraries(NindBasics)

set_target_properties(NindBasics PROPERTIES VERSION 1.1.0 SOVERSION 1)
install(TARGETS NindBasics DESTINATION lib)

########### install files ###############
install(FILES NindFile.h NindPadFile.h NindSignalCatcher.h DESTINATION include/NindBasics)
