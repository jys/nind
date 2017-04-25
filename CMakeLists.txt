cmake_minimum_required(VERSION 2.8)
ADD_DEFINITIONS(
    -std=c++11
)
PROJECT(nind)
add_subdirectory(src)
add_subdirectory(tst)

install(FILES NindConfig.cmake DESTINATION share/apps/nind/cmake/Modules)

