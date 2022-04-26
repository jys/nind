project(nind)
cmake_minimum_required(VERSION 2.8)

option(WITH_ASAN "Enable address sanitizer" OFF)
message("WITH_ASAN=${WITH_ASAN}")
option(WITH_ARCH "Enable architecture optimizations" OFF)
message("WITH_ARCH=${WITH_ARCH}")
option(WITH_DEBUG_MESSAGES "Enable debug messages" OFF)
message("WITH_DEBUG_MESSAGES=${WITH_DEBUG_MESSAGES}")

include(SystemSpecificInformations.cmake)


message("System name is ${CMAKE_SYSTEM_NAME}")
if (NOT (${CMAKE_SYSTEM_NAME} STREQUAL "Windows"))
    message("Linux flags")

    # In debug mode, adds the -DDEBUG_INDEX to the compiler command line.
    # Allows to build with conditionally built debug output
    set(DEBUG_FLAGS "-DDEBUG_NIND")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${DEBUG_FLAGS}" )
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} ${DEBUG_FLAGS}" )
    if (WITH_DEBUG_MESSAGES)
      set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${DEBUG_FLAGS}" )
    endif()

    if (WITH_ARCH)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
    else()
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mtune=generic -msse4.2")
    endif()

    # Flags needed for the plugins mechanism to work: our libs are dynamically loaded by
    # factories, thus their symbols must be known even if not used by the binary
    set(CMAKE_EXE_LINKER_FLAGS "-Wl,-z,defs,--no-as-needed")
    set(CMAKE_SHARED_LINKER_FLAGS "-Wl,-z,defs,--no-as-needed")
    set(CMAKE_MODULE_LINKER_FLAGS "-Wl,-z,defs,--no-as-needed")

    include(CheckCXXCompilerFlag)
    CHECK_CXX_COMPILER_FLAG(-std=c++0x HAVE_STDCPP0X)
    CHECK_CXX_COMPILER_FLAG(-std=c++11 HAVE_STDCPP11)
    CHECK_CXX_COMPILER_FLAG(-std=c++14 HAVE_STDCPP14)
    CHECK_CXX_COMPILER_FLAG(-fsanitize=undefined HAVE_SANITIZE_UNDEFINED)
    CHECK_CXX_COMPILER_FLAG(-fno-omit-frame-pointer HAVE_NO_OMIT_FRAME_POINTER)
    if (HAVE_NO_OMIT_FRAME_POINTER)
      set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer")
    endif()

    if (HAVE_STDCPP14)
      message("C++14 supported ")
      set(CMAKE_CXX_STANDARD 14)
      set(CMAKE_CXX_STANDARD_REQUIRED ON)
      set(CMAKE_CXX_EXTENSIONS OFF)
    elseif (HAVE_STDCPP11)
      message("C++11 supported")
      set(CMAKE_CXX_FLAGS "-std=c++11 -DBOOST_NO_HASH ${CMAKE_CXX_FLAGS}")
      CHECK_CXX_COMPILER_FLAG(-Wsuggest-override HAVE_SUGGEST_OVERRIDE)
      if (HAVE_SUGGEST_OVERRIDE)
#       set(CMAKE_CXX_FLAGS "-Wsuggest-override -Werror=suggest-override ${CMAKE_CXX_FLAGS}")
        set(CMAKE_CXX_FLAGS "-Wsuggest-override ${CMAKE_CXX_FLAGS}")
      endif()
    elseif (HAVE_STDCPP0X)
      message("C++0x supported")
      set(CMAKE_CXX_FLAGS "-std=c++0x -DBOOST_NO_HASH ${CMAKE_CXX_FLAGS}")
    else ()
      message("C++0x NOT supported")
      set(CMAKE_CXX_FLAGS "-DNO_STDCPP0X ${CMAKE_CXX_FLAGS}")
    endif ()
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
  if (NOT DEFINED CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_NO_WARNINGS)
    set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_NO_WARNINGS ON)
  endif ()

  set(CMAKE_C_FLAGS "/Zc:wchar_t- /EHsc /GR ${CMAKE_C_FLAGS}")
  set(CMAKE_CXX_FLAGS "/Zc:wchar_t- /EHsc /GR /W4 /MP /FAu ${CMAKE_CXX_FLAGS}")

  set(LIB_INSTALL_DIR "bin")
endif ()


add_subdirectory(src)
add_subdirectory(tst)

install(FILES NindConfig.cmake DESTINATION share/apps/nind/cmake/Modules)

########### packaging ###############
INCLUDE(InstallRequiredSystemLibraries)

set(CPACK_MONOLITHIC_INSTALL 1)
set(CPACK_PACKAGE_NAME "nind")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Nind Inverted file index")
set(CPACK_PACKAGE_VENDOR "Latejcon")
set(CPACK_PACKAGE_CONTACT "Jean-Yves Sage <jean-yves.sage@orange.fr>")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENCE.md")
set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "1")
set(CPACK_PACKAGE_VERSION_PATCH "0")
SET(CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "nind-${AMOSE_VERSION_MAJOR}.${AMOSE_VERSION_MINOR}")
set(CPACK_STRIP_FILES "")
set(CPACK_SOURCE_STRIP_FILES "")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
set(CPACK_GENERATOR ${SPECIFIC_SYSTEM_PREFERED_CPACK_GENERATOR})


set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}")

# set of commands to define component based packaging
# notify cpack of component all names
set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME "runtime")
SET(CPACK_COMPONENTS_ALL lib devel runtime)
set(CPACK_RPM_COMPONENT_INSTALL ON)
set(CPACK_DEB_COMPONENT_INSTALL ON)

INCLUDE(CPack)
