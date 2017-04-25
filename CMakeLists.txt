project(nind)
cmake_minimum_required(VERSION 2.8)

include(SystemSpecificInformations.cmake)

option(WITH_ASAN "Enable address sanitizer" OFF)
message("WITH_ASAN=${WITH_ASAN}")
option(WITH_ARCH "Enable architecture optimizations" OFF)
message("WITH_ARCH=${WITH_ARCH}")

if (NOT (${CMAKE_SYSTEM_NAME} STREQUAL "Windows"))
    if ("x${CMAKE_BUILD_TYPE}" STREQUAL "xRelease")
      if (WITH_ARCH)
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -march=native -O2")
      else()
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -mtune=generic -march=x86-64")
      endif()
    endif()
endif()


add_definitions(
    -std=c++11
)

if (NOT (${CMAKE_SYSTEM_NAME} STREQUAL "Windows"))
    message("Linux flags")

    # Flags needed for the LIMA plugins mechanism to work: our libs are dynamically loaded by
    # factories, thus their symbols must be known even if not used by the binary
    set(CMAKE_EXE_LINKER_FLAGS "-Wl,-z,defs,--no-as-needed")
    set(CMAKE_SHARED_LINKER_FLAGS "-Wl,-z,defs,--no-as-needed")
    set(CMAKE_MODULE_LINKER_FLAGS "-Wl,-z,defs,--no-as-needed")

    include(CheckCXXCompilerFlag)
    CHECK_CXX_COMPILER_FLAG(-std=c++0x HAVE_STDCPP0X)
    CHECK_CXX_COMPILER_FLAG(-std=c++11 HAVE_STDCPP11)
    CHECK_CXX_COMPILER_FLAG(-fsanitize=undefined HAVE_SANITIZE_UNDEFINED)
    CHECK_CXX_COMPILER_FLAG(-fno-omit-frame-pointer HAVE_NO_OMIT_FRAME_POINTER)
    if (HAVE_NO_OMIT_FRAME_POINTER)
      set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer")
    endif()

    if (HAVE_STDCPP0X)
      message("C++0x supported")
      set(CMAKE_CXX_FLAGS "-std=c++0x -DBOOST_NO_HASH ${CMAKE_CXX_FLAGS}")
    else (HAVE_STDCPP0X)
      message("C++0x NOT supported")
      set(CMAKE_CXX_FLAGS "-DNO_STDCPP0X ${CMAKE_CXX_FLAGS}")
    endif (HAVE_STDCPP0X)
    if (HAVE_STDCPP11)
      message("C++11 supported")
      set(CMAKE_CXX_FLAGS "-std=c++11 -DBOOST_NO_HASH ${CMAKE_CXX_FLAGS}")
    else (HAVE_STDCPP11)
      message("C++11 NOT supported")
      set(CMAKE_CXX_FLAGS "-DNO_STDCPP0X ${CMAKE_CXX_FLAGS}")
    endif (HAVE_STDCPP11)
    set(CMAKE_CXX_FLAGS "-W -Wall ${CMAKE_CXX_FLAGS}")

    if (WITH_ASAN)
      # NOTE: check -fsanitize=address flag failed if this flag is not set also
      # for the linker (use CMAKE_REQUIRED_FLAGS to set linker flag for 
      # CHECK_CXX_COMPILER_FLAG macro).
      set(SAFE_CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS}")
      set(CMAKE_REQUIRED_FLAGS "-fsanitize=address")
      CHECK_CXX_COMPILER_FLAG(-fsanitize=address HAVE_SANITIZE_ADDRESS)
      set(CMAKE_REQUIRED_FLAGS "${SAFE_CMAKE_REQUIRED_FLAGS}")
      if (HAVE_SANITIZE_ADDRESS)
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address")
      endif()
      CHECK_CXX_COMPILER_FLAG(-fsanitize=undefined HAVE_SANITIZE_UNDEFINED)
      if (HAVE_SANITIZE_UNDEFINED)
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=undefined")
      endif()
      CHECK_CXX_COMPILER_FLAG(-fsanitize=leak HAVE_SANITIZE_LEAK)
      if (HAVE_SANITIZE_LEAK)
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=leak")
      endif()

    endif ()

    set(LIB_INSTALL_DIR "lib")
else ()
  message("Windows flags")
  add_definitions(-D WIN32)
  
  # By default, do not warn when built on machines using only VS Express:
  if(NOT DEFINED CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_NO_WARNINGS)
    set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_NO_WARNINGS ON)
  endif()
  set(CMAKE_C_FLAGS "/Zc:wchar_t- /EHsc /GR ${CMAKE_C_FLAGS}")
  set(CMAKE_CXX_FLAGS "/Zc:wchar_t- /EHsc /GR /W4 /MP /FAu ${CMAKE_CXX_FLAGS}")

  set(LIB_INSTALL_DIR "bin")
endif ()


add_subdirectory(src)
add_subdirectory(tst)

install(FILES NindConfig.cmake DESTINATION share/apps/nind/cmake/Modules)

########### packaging ###############
include(InstallRequiredSystemLibraries)

set(CPACK_MONOLITHIC_INSTALL 1)
set(CPACK_PACKAGE_NAME "nind")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Nind Inverted file index")
set(CPACK_PACKAGE_VENDOR "Latécon")
set(CPACK_PACKAGE_CONTACT "Jean-Yves Sage <jean-yves.sage@orange.fr>")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENCE.md")
set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "1")
set(CPACK_PACKAGE_VERSION_PATCH "0")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "nind-${AMOSE_VERSION_MAJOR}.${AMOSE_VERSION_MINOR}")
set(CPACK_STRIP_FILES "")
set(CPACK_SOURCE_STRIP_FILES "")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
set(CPACK_GENERATOR ${SPECIFIC_SYSTEM_PREFERED_CPACK_GENERATOR})

include(CPack)
