cmake_minimum_required(VERSION 3.30)
project(collider)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# find SDL2
find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)

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
    SDL2
    SDL2_image
    Arma::Armadillo
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
