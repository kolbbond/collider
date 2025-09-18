# @hey: set toolchain, have user set this earlier
if(WIN32)
    set(CMAKE_TOOLCHAIN_FILE "C:/Users/dkolbbond/work/programs/cpp/vcpkg/scripts/buildsystems/vcpkg.cmake")
endif()

# This is not meant to be included by the top-level.
cmake_minimum_required(VERSION 3.22)
project(GOOSE_DEPS C)

if(POLICY CMP0135)
  cmake_policy(SET CMP0135 NEW)
endif()

include(ProcessorCount)
ProcessorCount(N)

# Options
option(DEPS_ENABLE_TESTING "Enable testing for dependencies." OFF)
option(USE_BUNDLED "Use bundled dependencies." ON)
option(BUILD_GUI "build the gui library" ${USE_BUNDLED})
option(USE_BUNDLED_SMG "Use the bundled smg." ${USE_BUNDLED})
option(ENABLE_CUDA "Enable CUDA support." ON)
option(USE_BUNDLED_SUPERLU "use bundled superlu" ${USE_BUNDLED})
option(USE_BUNDLED_ARMADILLO "use bundled armadillo" ${USE_BUNDLED})
option(USE_BUNDLED_OPENBLAS "use bundled openblas (huge package)" OFF)  # just install this system wide 
option(USE_BUNDLED_TRACY "use bundled tracy" ON) 
option(USE_BUNDLED_CHUG "use bundled chug" ON) 

# get parallel option from makefile?
#set(DEPS_BUILD_COMMAND "${CMAKE_COMMAND} --build . --parallel ${N}")
set(DEPS_BUILD_COMMAND "${CMAKE_COMMAND}";  --build .; --parallel 4)

# Set the default build type if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
endif()

# check build type
if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug" AND NOT CMAKE_BUILD_TYPE STREQUAL "Release")
    message(FATAL_ERROR "CMAKE_BUILD_TYPE must be either 'Debug' or 'Release'.")
    if(NOT CMAKE_BUILD_TYPE STREQUAL "Release")
        message(FATAL_ERROR "CMAKE_BUILD_TYPE 'Release'.")
    endif()
endif()

# set build type as dependency argument
set(DEPS_CMAKE_ARGS "${DEPS_CMAKE_ARGS}" "-D CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")

# Point CMake at any custom modules we may ship
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" "${PROJECT_SOURCE_DIR}/../cmake")

# Prefer our bundled versions of dependencies.
if(DEFINED ENV{DEPS_BUILD_DIR})
  set(DEPS_PREFIX "$ENV{DEPS_BUILD_DIR}/../usr" CACHE PATH "Path prefix for finding dependencies")
else()
  set(DEPS_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/../.deps/usr" CACHE PATH "Path prefix for finding dependencies")
endif()

include(CheckCCompilerFlag)
include(ExternalProject)
include(FindPackageHandleStandardArgs)

# neovim utility functions
include(Deps)
if(WIN32)
    message(STATUS "this is a Windows machine")

    # need to add the Visual Studio environment variables
    # @hey: hardcoded vcpkg 
    # add a vcpkg path variable
    list(INSERT CMAKE_MODULE_PATH 0 "${CMAKE_CURRENT_SOURCE_DIR}/../../vcpkg/installed/x64-windows/share/")
    list(INSERT CMAKE_MODULE_PATH 0 "${CMAKE_CURRENT_SOURCE_DIR}/../../vcpkg/installed/x64-windows/share/freetype")
    list(INSERT CMAKE_MODULE_PATH 0 "${CMAKE_CURRENT_SOURCE_DIR}/../../vcpkg/installed/x64-windows/lib/pkgconfig")
    #list(INSERT CMAKE_MODULE_PATH 0 "${CMAKE_CURRENT_SOURCE_DIR}/../../vcpkg/ports/lapack-reference")
    #list(INSERT CMAKE_MODULE_PATH 0 "${CMAKE_CURRENT_SOURCE_DIR}/../../vcpkg/ports/openblas")
    #list(INSERT CMAKE_MODULE_PATH 0 "${CMAKE_CURRENT_SOURCE_DIR}/../../vcpkg/ports/clapack")

    #list(INSERT CMAKE_PREFIX_PATH 0 "${CMAKE_CURRENT_SOURCE_DIR}/../../vcpkg/ports/lapack-reference")
    list(INSERT CMAKE_PREFIX_PATH 0 "${CMAKE_CURRENT_SOURCE_DIR}/../../vcpkg/installed/x64-windows/debug/bin/")

    list(INSERT CMAKE_PREFIX_PATH 0 "${CMAKE_CURRENT_SOURCE_DIR}/../../vcpkg/installed/x64-windows/lib/")
    list(INSERT CMAKE_PREFIX_PATH 0 "${CMAKE_CURRENT_SOURCE_DIR}/../../vcpkg/installed/x64-windows/include/")
    list(INSERT CMAKE_PREFIX_PATH 0 "${CMAKE_CURRENT_SOURCE_DIR}/../../vcpkg/installed/x64-windows/share/")
    list(INSERT CMAKE_PREFIX_PATH 0 "${CMAKE_CURRENT_SOURCE_DIR}/../../vcpkg/installed/x64-windows/share/freetype")
    list(INSERT CMAKE_PREFIX_PATH 0 "${CMAKE_CURRENT_SOURCE_DIR}/../../vcpkg/installed/x64-windows/lib/cmake/SDL2")
    list(INSERT CMAKE_PREFIX_PATH 0 "${CMAKE_CURRENT_SOURCE_DIR}/../../vcpkg/installed/x64-windows/lib/cmake")
    list(INSERT CMAKE_PREFIX_PATH 0 "${CMAKE_CURRENT_SOURCE_DIR}/../../vcpkg/installed/x64-windows/include/freetype")
    list(INSERT CMAKE_PREFIX_PATH 0 "${CMAKE_CURRENT_SOURCE_DIR}/../../vcpkg/installed/x64-windows/lib/pkgconfig")

    # @hey: add a runtime output for all libraries
    #list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" "${PROJECT_SOURCE_DIR}/../cmake")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/bin CACHE PATH "" FORCE)

elseif(UNIX)

# check openmp
    list(INSERT CMAKE_PREFIX_PATH 0 "/usr/lib/x86_64-linux-gnu/openmpi/")
    list(INSERT CMAKE_PREFIX_PATH 0 "/usr/lib/x86_64-linux-gnu/")

endif()

if(UNIX)
  message(STATUS "this is a linux machine")
    list(INSERT CMAKE_PREFIX_PATH 0 "/usr/lib/x86_64-linux-gnu/") # for opengl and sdl
    list(INSERT CMAKE_PREFIX_PATH 0 "/usr/local/cuda")
    list(INSERT CMAKE_PREFIX_PATH 0 "/usr/local/cuda/bin")
    list(INSERT CMAKE_PREFIX_PATH 0 "/usr/local/cuda/include")
# additional prefix paths
endif(UNIX)

# if enable cuda
# @hey: set this earlier as the user? or in presets
if(WIN32)
    set(CUDA_TOOLKIT_ROOT_DIR "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v12.9/bin")
    set(CMAKE_CUDA_COMPILER "${CUDA_TOOLKIT_ROOT_DIR}/nvcc.exe")
else()
    list(INSERT CMAKE_PREFIX_PATH 0 "/usr/local/cuda-12")
    list(INSERT CMAKE_PREFIX_PATH 0 "/usr/local/cuda-12/bin")
    list(INSERT CMAKE_PREFIX_PATH 0 "/usr/local/cuda-12/include")
    set(CUDA_TOOLKIT_ROOT_DIR "/usr/local/cuda")
    set(CMAKE_CUDA_COMPILER "${CUDA_TOOLKIT_ROOT_DIR}/bin/nvcc")
endif()


# additional prefix paths
list(INSERT CMAKE_PREFIX_PATH 0 "${DEPS_PREFIX}")
list(INSERT CMAKE_PREFIX_PATH 0 "${DEPS_PREFIX}/bin/")
list(INSERT CMAKE_PREFIX_PATH 0 "${DEPS_PREFIX}/include/")
list(INSERT CMAKE_PREFIX_PATH 0 "${DEPS_PREFIX}/lib/")
list(INSERT CMAKE_PREFIX_PATH 0 "${DEPS_PREFIX}/share/")
list(INSERT CMAKE_PREFIX_PATH 0 "${DEPS_PREFIX}/share/cmake/Corrade")

# if enable cuda
# @hey: set this earlier as the user? or in presets
if(WIN32)
    set(CUDA_TOOLKIT_ROOT_DIR "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v12.9/bin")
    set(CMAKE_CUDA_COMPILER "${CUDA_TOOLKIT_ROOT_DIR}/nvcc.exe")
else()
    set(CUDA_TOOLKIT_ROOT_DIR "/usr/local/cuda")
    set(CMAKE_CUDA_COMPILER "${CUDA_TOOLKIT_ROOT_DIR}/bin/nvcc")
endif()

find_package(OpenMP REQUIRED)
message(STATUS "OpenMP_LiBRARIES: ${OpenMP_LIBRARIES}")

## Add this block to check what was found
if(OpenMP_FOUND)
  message(STATUS "OpenMP FOUND")
  message(STATUS "  > CXX Flags: ${OpenMP_CXX_FLAGS}")
  message(STATUS "  > CXX Libraries: ${OpenMP_CXX_LIBRARIES}")
  message(STATUS "  > C Flags: ${OpenMP_C_FLAGS}")
  message(STATUS "  > C Libraries: ${OpenMP_C_LIBRARIES}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
  #set(DEPS_CMAKE_ARGS "${DEPS_CMAKE_ARGS}" "${OpenMP_CXX_FLAGS}")
  #set(DEPS_CMAKE_ARGS "${DEPS_CMAKE_ARGS}" "${OpenMP_C_FLAGS}")
else()
  message(WARNING "OpenMP NOT FOUND")
endif()

#-------------------------------------------------------------------------------
# User settings
#-------------------------------------------------------------------------------

set(DEPS_IGNORE_SHA FALSE)

if(USE_BUNDLED AND MSVC)
    #option(USE_BUNDLED_GETTEXT "Use the bundled version of gettext." ON)
else()
    #option(USE_BUNDLED_GETTEXT "Use the bundled version of gettext." OFF)
endif()

# @hey: add boost
# boost-math, boost-sort, boost-filesystem, boost-iostreams

# build openblas
if(USE_BUNDLED_OPENBLAS)
    include(BuildOpenBLAS)
else()
# find openblas
# @NOTE: let's just confirm we have it
# @hey: add option for intel MKL
#find_package(OpenBLAS CONFIG REQUIRED)
#message(WARNING "OpenBLAS found: ${OpenBLAS_VERSION} at ${OpenBLAS_INCLUDE_DIRS} and ${OpenBLAS_LIBRARIES}")
# find openblas
# @hey: maybe we just confirm we have openblas prior

    message(STATUS "CMAKE_MODULE_PATH: ${CMAKE_MODULE_PATH}")

    # if one api @hey: run based on environment variable
    #set(MKL_ROOT_DIR "C:/Program Files (x86)/Intel/oneAPI/mkl/2025.2")
    #list(INSERT CMAKE_PREFIX_PATH 0 "C:/Program Files (x86)/Intel/oneAPI/mkl/2025.2/bin")
    #list(INSERT CMAKE_PREFIX_PATH 0 "C:/Program Files (x86)/Intel/oneAPI/mkl/2025.2/lib")
    #list(INSERT CMAKE_MODULE_PATH 0 "${MKL_ROOT_DIR}/lib/cmake/mkl")

    # @hey: allow vendor settings
    set(BLA_VENDOR OpenBLAS)
    #set(BLA_VENDOR Intel10_64ilp)
    #find_package(MKL CONFIG REQUIRED)
    #set(BLA_STATIC ON)
    find_package(OpenBLAS CONFIG REQUIRED)
    find_package(BLAS REQUIRED)
    message(WARNING "BLAS found: ${BLAS_VERSION} at ${BLAS_INCLUDE_DIRS}")

    find_package(LAPACK REQUIRED)
    message(WARNING "LAPACK library: ${LAPACK_LIBRARIES}")
endif()


# build superlu
if(USE_BUNDLED_SUPERLU)
    include(Buildsuperlu)
endif()

# build armadillo
if(USE_BUNDLED_ARMADILLO)
    include(BuildArmadillo)
endif()

# build jsoncpp
if(WIN32)
else()
    include(Buildjsoncpp)
endif()

# build vtk
if(USE_BUNDLED_VTK)
    include(BuildVTK)
endif()

# for gui
if(USE_BUNDLED_SMG)
    message(STATUS "prefix path: ${CMAKE_PREFIX_PATH}")
    add_subdirectory(smg)
endif()

# if using python
if(USE_BUNDLED_PYBIND11)
    include(Buildpybind11)
endif()

if(USE_BUNDLED_TRACY)
    include(BuildTracy)
endif()

if(USE_BUNDLED_CHUG)
    include(Buildchug)
endif()

