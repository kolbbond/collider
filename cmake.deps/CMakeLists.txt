# Bundled dependency builder for collider (optional).
#
# By default collider builds against system packages. To build the
# dependencies from source instead, run `make deps` (which configures and
# builds this directory into .deps/usr) and then build collider with
# -DUSE_BUNDLED_DEPS=ON.
#
# Collider only needs Armadillo, which in turn uses BLAS/LAPACK and,
# optionally, SuperLU.
cmake_minimum_required(VERSION 3.22)
project(COLLIDER_DEPS C)

if(POLICY CMP0135)
	cmake_policy(SET CMP0135 NEW)
endif()

include(ProcessorCount)
ProcessorCount(N)

# Options
option(USE_BUNDLED "Use bundled dependencies." ON)
option(USE_BUNDLED_SUPERLU "build bundled superlu" ${USE_BUNDLED})
option(USE_BUNDLED_ARMADILLO "build bundled armadillo" ${USE_BUNDLED})
option(USE_BUNDLED_OPENBLAS "build bundled openblas (large; prefer a system install)" OFF)

set(DEPS_BUILD_COMMAND "${CMAKE_COMMAND}" --build . --parallel ${N})

# default build type
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
endif()
set(DEPS_CMAKE_ARGS "${DEPS_CMAKE_ARGS}" "-D CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")

# custom modules (Deps.cmake, Build*.cmake)
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" "${PROJECT_SOURCE_DIR}/../cmake")

# install / search prefix for the bundled dependencies
if(DEFINED ENV{DEPS_BUILD_DIR})
	set(DEPS_PREFIX "$ENV{DEPS_BUILD_DIR}/../usr" CACHE PATH "Path prefix for finding dependencies")
else()
	set(DEPS_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/../.deps/usr" CACHE PATH "Path prefix for finding dependencies")
endif()
list(INSERT CMAKE_PREFIX_PATH 0 "${DEPS_PREFIX}")

include(CheckCCompilerFlag)
include(ExternalProject)
include(FindPackageHandleStandardArgs)
include(Deps)

set(DEPS_IGNORE_SHA FALSE)

find_package(OpenMP REQUIRED)

# BLAS / LAPACK: use a system OpenBLAS unless asked to build it
if(USE_BUNDLED_OPENBLAS)
	include(BuildOpenBLAS)
else()
	set(BLA_VENDOR OpenBLAS)
	find_package(BLAS REQUIRED)
	find_package(LAPACK REQUIRED)
endif()

# superlu (optional sparse solver used by armadillo)
if(USE_BUNDLED_SUPERLU)
	include(Buildsuperlu)
endif()

# armadillo (the dependency collider actually needs)
if(USE_BUNDLED_ARMADILLO)
	include(BuildArmadillo)
endif()
