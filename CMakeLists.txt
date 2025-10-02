cmake_minimum_required(VERSION 3.22)
project(collider)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# enables
option(ENABLE_GUI "build with smg" ON)

# ensure out of core build
if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
  message(
    FATAL_ERROR
      "In-source builds not allowed (for your own safety).\nPlease make a new directory (called a build directory) and run CMake from there.\n"
  )
endif()

# if bundled
# Prefer our bundled versions of dependencies.
if(DEFINED ENV{DEPS_BUILD_DIR})
  set(DEPS_PREFIX "$ENV{DEPS_BUILD_DIR}/usr" CACHE PATH "Path prefix for finding dependencies")
else()
  set(DEPS_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/.deps/usr" CACHE PATH "Path prefix for finding dependencies")
endif()

# set the goose directory
set(COLLIDER_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/..")

# if enable cuda
# @hey: can we move this to makefile also?
if(WIN32)
    set(CUDA_TOOLKIT_ROOT_DIR "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v12.9/")
    set(CMAKE_CUDA_COMPILER "${CUDA_TOOLKIT_ROOT_DIR}/bin/nvcc.exe")
else()
    set(CUDA_TOOLKIT_ROOT_DIR "/usr/local/cuda")
    set(CMAKE_CUDA_COMPILER "${CUDA_TOOLKIT_ROOT_DIR}/bin/nvcc")
endif()

# compiler options
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	message(STATUS "using GCC settings")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -march=native -O3")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
	message(STATUS "using Clang settings")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -march=native -O3 -funroll-loops")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	message(STATUS "using MSVC settings")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /O2")
    string( REPLACE "/DNDEBUG" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}") # enable asserts
else()
	message(STATUS "Compiler not recognized")
endif()

# output paths
if(NOT DEFINED CMAKE_RUNTIME_OUTPUT_DIRECTORY AND NOT DEFINED CMAKE_LIBRARY_OUTPUT_DIRECTORY AND NOT DEFINED CMAKE_ARCHIVE_OUTPUT_DIRECTORY)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin CACHE PATH "" FORCE)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib CACHE PATH "" FORCE)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib CACHE PATH "" FORCE)
endif()

if(WIN32)
    message(STATUS "this is a Windows machine")

    # need to add the Visual Studio environment variables
    # @hey: hardcoded vcpkg 
    # add a vcpkg path variable
    list(INSERT CMAKE_MODULE_PATH 0 "${CMAKE_CURRENT_SOURCE_DIR}/../vcpkg/installed/x64-windows/share/")
    list(INSERT CMAKE_MODULE_PATH 0 "${CMAKE_CURRENT_SOURCE_DIR}/../vcpkg/installed/x64-windows/share/freetype")
    list(INSERT CMAKE_MODULE_PATH 0 "${CMAKE_CURRENT_SOURCE_DIR}/../vcpkg/installed/x64-windows/lib/pkgconfig")

    list(INSERT CMAKE_PREFIX_PATH 0 "${CMAKE_CURRENT_SOURCE_DIR}/../vcpkg/installed/x64-windows/lib/")
    list(INSERT CMAKE_PREFIX_PATH 0 "${CMAKE_CURRENT_SOURCE_DIR}/../vcpkg/installed/x64-windows/include/")
    list(INSERT CMAKE_PREFIX_PATH 0 "${CMAKE_CURRENT_SOURCE_DIR}/../vcpkg/installed/x64-windows/share/")
    list(INSERT CMAKE_PREFIX_PATH 0 "${CMAKE_CURRENT_SOURCE_DIR}/../vcpkg/installed/x64-windows/share/freetype")
    list(INSERT CMAKE_PREFIX_PATH 0 "${CMAKE_CURRENT_SOURCE_DIR}/../vcpkg/installed/x64-windows/lib/cmake/SDL2")
    list(INSERT CMAKE_PREFIX_PATH 0 "${CMAKE_CURRENT_SOURCE_DIR}/../vcpkg/installed/x64-windows/lib/cmake")
    list(INSERT CMAKE_PREFIX_PATH 0 "${CMAKE_CURRENT_SOURCE_DIR}/../vcpkg/installed/x64-windows/include/freetype")
    list(INSERT CMAKE_PREFIX_PATH 0 "${CMAKE_CURRENT_SOURCE_DIR}/../vcpkg/installed/x64-windows/lib/pkgconfig")

    # @hey: add a runtime output for all libraries
    #list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" "${PROJECT_SOURCE_DIR}/../cmake")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/bin CACHE PATH "" FORCE)

endif(WIN32)
if(UNIX)
  message(STATUS "this is a linux machine")
    list(INSERT CMAKE_PREFIX_PATH 0 "/usr/lib/x86_64-linux-gnu/") # for opengl and sdl
# additional prefix paths
endif(UNIX)

# insert into lists at the beginning to search there first
list(INSERT CMAKE_MODULE_PATH 0 "${DEPS_PREFIX}/lib/cmake/")
list(INSERT CMAKE_MODULE_PATH 0 "${DEPS_PREFIX}/cmake/")
list(INSERT CMAKE_MODULE_PATH 0 "${DEPS_PREFIX}/share/cmake/Corrade")
list(INSERT CMAKE_MODULE_PATH 0 "${DEPS_PREFIX}/share/pkgconfig")

# prepend prefix paths @hey: not all of this is necessary
list(INSERT CMAKE_PREFIX_PATH 0 "${DEPS_PREFIX}/share/pkgconfig")
list(INSERT CMAKE_PREFIX_PATH 0 "${DEPS_PREFIX}/include")
list(INSERT CMAKE_PREFIX_PATH 0 "${DEPS_PREFIX}/bin")
list(INSERT CMAKE_PREFIX_PATH 0 "${DEPS_PREFIX}/lib")
list(INSERT CMAKE_PREFIX_PATH 0 "${DEPS_PREFIX}/lib/smg")
list(INSERT CMAKE_PREFIX_PATH 0 "${DEPS_PREFIX}/share")
list(INSERT CMAKE_PREFIX_PATH 0 "${DEPS_PREFIX}/lib/cmake") 
list(INSERT CMAKE_PREFIX_PATH 0 "${DEPS_PREFIX}/lib/cmake/smg/") 
list(INSERT CMAKE_PREFIX_PATH 0 "${DEPS_PREFIX}/share/cmake/Corrade")
list(INSERT CMAKE_PREFIX_PATH 0 "${DEPS_PREFIX}/share/cmake/Magnum")
list(INSERT CMAKE_PREFIX_PATH 0 "${DEPS_PREFIX}/share/cmake/MagnumIntegration")
list(INSERT CMAKE_PREFIX_PATH 0 "${CMAKE_CURRENT_SOURCE_DIR}/build/") 
list(INSERT CMAKE_PREFIX_PATH 0 "${CMAKE_CURRENT_SOURCE_DIR}/build/lib/") 
list(INSERT CMAKE_PREFIX_PATH 0 "${CMAKE_CURRENT_SOURCE_DIR}/build/lib/cmake") 
list(INSERT CMAKE_PREFIX_PATH 0 "${CMAKE_CURRENT_SOURCE_DIR}/build/lib/cmake/goosecommon") 
list(INSERT CMAKE_PREFIX_PATH 0 "${CMAKE_CURRENT_SOURCE_DIR}/build/include/")
list(INSERT CMAKE_PREFIX_PATH 0 "/usr/lib/x86_64-linux-gnu/")
list(INSERT CMAKE_PREFIX_PATH 0 "/usr/include/x86_64-linux-gnu/")
list(INSERT CMAKE_PREFIX_PATH 0 "/usr/lib/x86_64-linux-gnu/cmake")
list(INSERT CMAKE_PREFIX_PATH 0 "/usr/lib/x86_64-linux-gnu/cmake/SDL2")
list(INSERT CMAKE_MODULE_PATH 0 "/usr/lib/x86_64-linux-gnu/cmake")
list(INSERT CMAKE_MODULE_PATH 0 "/usr/lib/x86_64-linux-gnu/cmake/SDL2/")


# find SDL2
find_package(SDL2 REQUIRED)
#find_package(SDL2_image REQUIRED)
find_package(chug REQUIRED)

if(ENABLE_GUI)
    # find smg
    find_package(smg REQUIRED)
endif()

# find armadillo library
find_package(Armadillo 12.8.0 CONFIG REQUIRED)
if(NOT TARGET Arma::Armadillo)
	# create target
	add_library(Arma::Armadillo INTERFACE IMPORTED)
	set_target_properties(Arma::Armadillo PROPERTIES
		INTERFACE_INCLUDE_DIRECTORIES "${ARMADILLO_INCLUDE_DIRS}"
		INTERFACE_LINK_LIBRARIES "${ARMADILLO_LIBRARY}")

	# set (n)debug flags for Armadilo
	if(CMAKE_BUILD_TYPE STREQUAL Release)
		message(STATUS "Using Armadillo no-debug flags (conformance check disabled!)")
		target_compile_definitions(Arma::Armadillo INTERFACE -DNDEBUG -DARMA_NO_DEBUG -DARMA_DONT_CHECK_CONFORMANCE -DARMA_WARN_LEVEL=1)
	else()
		message(STATUS "Using Armadillo in debug mode")
	endif()

	# disable armadillo wrapper (for using custom BLAS)
	if(BLAS_FOUND AND LAPACK_FOUND)
		# report to user
		message(STATUS "Disabling Armadillo blas wrapper")
		target_compile_definitions(Arma::Armadillo INTERFACE -DARMA_DONT_USE_WRAPPER -DARMA_USE_BLAS)
	endif()

	# disable superlu if not available
	if(SUPERLU_FOUND AND SUPERLU_VERSION_MAJOR EQUAL 5)
		message(STATUS "Enabling Superlu")
	else()
		message(STATUS "Disabling Armadillo SuperLU")
		target_compile_definitions(Arma::Armadillo INTERFACE -DARMA_DONT_USE_SUPERLU)
	endif()
endif()

# add sources
set(source_list 
    #src/Extra.cpp
    src/Log.cpp
    src/Piece.cpp
    src/PieceMoves.cpp
    src/Board.cpp
    src/Engine.cpp
    src/UCI.cpp
)

add_library(collider
    ${source_list}
    )

target_link_libraries(collider
    chug::chug
    Arma::Armadillo

    SDL2
    SDL2_image
    smg::smg
)

target_include_directories(collider PUBLIC
include

)

# executable
add_executable(main main.cpp
)

target_link_libraries(main
    collider
)



enable_testing()
add_subdirectory(test)
