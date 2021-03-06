cmake_minimum_required(VERSION 3.1)

# set the project name and version
project(QSsh VERSION 1.0)

if(MSVC)
# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4005")
set(BOTAN_LIB ${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib/botan.lib)
else(MSVC)
# specify the C++ standard
set(CMAKE_CXX_STANDARD 14)
include_directories("/usr/local/include/botan-2")
include_directories("/usr/include/botan-2")
set(BOTAN_LIB "-lbotan-2")
endif(MSVC)

set(CMAKE_CXX_STANDARD_REQUIRED True)


find_package(Qt5 COMPONENTS Core Widgets Network  REQUIRED)

# qt
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

add_subdirectory(src)
add_subdirectory(examples)

# INSTALL RULES
install(EXPORT QSsh-targets DESTINATION lib)
install(EXPORT QSsh-targets
  FILE QSshTargets.cmake
  NAMESPACE QSsh::
  DESTINATION share/QSsh/cmake
)


install(FILES "${CMAKE_SOURCE_DIR}/cmakeFindModules/QSshConfig.cmake"
        DESTINATION share/QSsh/cmake)
