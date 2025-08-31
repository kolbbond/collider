cmake_minimum_required(VERSION 3.30)
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

# output paths
if(NOT DEFINED CMAKE_RUNTIME_OUTPUT_DIRECTORY AND NOT DEFINED CMAKE_LIBRARY_OUTPUT_DIRECTORY AND NOT DEFINED CMAKE_ARCHIVE_OUTPUT_DIRECTORY)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/bin CACHE PATH "" FORCE)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/lib CACHE PATH "" FORCE)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/lib CACHE PATH "" FORCE)
endif()


# find SDL2
find_package(chug REQUIRED)
find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)

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
    src/Board.cpp
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
