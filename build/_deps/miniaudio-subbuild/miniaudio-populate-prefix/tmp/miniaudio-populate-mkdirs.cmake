# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION ${CMAKE_VERSION}) # this file comes with cmake

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "C:/Engine/build/_deps/miniaudio-src")
  file(MAKE_DIRECTORY "C:/Engine/build/_deps/miniaudio-src")
endif()
file(MAKE_DIRECTORY
  "C:/Engine/build/_deps/miniaudio-build"
  "C:/Engine/build/_deps/miniaudio-subbuild/miniaudio-populate-prefix"
  "C:/Engine/build/_deps/miniaudio-subbuild/miniaudio-populate-prefix/tmp"
  "C:/Engine/build/_deps/miniaudio-subbuild/miniaudio-populate-prefix/src/miniaudio-populate-stamp"
  "C:/Engine/build/_deps/miniaudio-subbuild/miniaudio-populate-prefix/src"
  "C:/Engine/build/_deps/miniaudio-subbuild/miniaudio-populate-prefix/src/miniaudio-populate-stamp"
)

set(configSubDirs Debug)
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "C:/Engine/build/_deps/miniaudio-subbuild/miniaudio-populate-prefix/src/miniaudio-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "C:/Engine/build/_deps/miniaudio-subbuild/miniaudio-populate-prefix/src/miniaudio-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
