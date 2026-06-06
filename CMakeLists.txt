cmake_minimum_required(VERSION 3.22)
project(collider_project CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# options
option(ENABLE_GUI "build the lightweight SDL GUI" OFF)
option(USE_BUNDLED_DEPS "find dependencies under .deps/usr (built by 'make deps') instead of system packages" OFF)

# no in-source builds
if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
	message(FATAL_ERROR "In-source builds are not allowed. Configure from a separate build directory.")
endif()

# compiler flags
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -march=native")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8 /O2")
endif()

# custom build type: optimized, but keeps asserts enabled
set(CMAKE_CXX_FLAGS_RELWITHASSERTS "${CMAKE_CXX_FLAGS_RELEASE}" CACHE STRING "" FORCE)
string(REPLACE "-DNDEBUG" "" CMAKE_CXX_FLAGS_RELWITHASSERTS "${CMAKE_CXX_FLAGS_RELWITHASSERTS}")
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug;Release;RelWithDebInfo;MinSizeRel;RelWithAsserts")
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "RelWithAsserts" CACHE STRING "Build type" FORCE)
endif()

# output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# optional bundled dependencies, built into .deps/usr by `make deps`
if(USE_BUNDLED_DEPS)
	if(DEFINED ENV{DEPS_BUILD_DIR})
		set(DEPS_PREFIX "$ENV{DEPS_BUILD_DIR}/usr")
	else()
		set(DEPS_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/.deps/usr")
	endif()
	message(STATUS "Using bundled dependencies from ${DEPS_PREFIX}")
	list(INSERT CMAKE_PREFIX_PATH 0 "${DEPS_PREFIX}")
	list(INSERT CMAKE_MODULE_PATH 0 "${DEPS_PREFIX}/lib/cmake")
endif()

# armadillo (required)
find_package(Armadillo 12.8.0 CONFIG REQUIRED)
if(NOT TARGET Arma::Armadillo)
	add_library(Arma::Armadillo INTERFACE IMPORTED)
	set_target_properties(Arma::Armadillo PROPERTIES
		INTERFACE_INCLUDE_DIRECTORIES "${ARMADILLO_INCLUDE_DIRS}"
		INTERFACE_LINK_LIBRARIES "${ARMADILLO_LIBRARY}")

	# optimized builds: turn off Armadillo's debug/conformance checks
	if(CMAKE_BUILD_TYPE STREQUAL Release OR CMAKE_BUILD_TYPE STREQUAL RelWithAsserts)
		message(STATUS "Using Armadillo no-debug flags")
		target_compile_definitions(Arma::Armadillo INTERFACE -DARMA_NO_DEBUG -DARMA_DONT_CHECK_CONFORMANCE -DARMA_WARN_LEVEL=1)
	endif()

	# only enable SuperLU if a compatible version was found
	if(NOT (SUPERLU_FOUND AND SUPERLU_VERSION_MAJOR EQUAL 5))
		target_compile_definitions(Arma::Armadillo INTERFACE -DARMA_DONT_USE_SUPERLU)
	endif()
endif()

# sources
set(source_list
	src/Log.cxx
	src/Piece.cxx
	src/PieceMoves.cxx
	src/Board.cxx
	src/Engine.cxx
	src/UCI.cxx
)

# the SDL GUI is compiled in only when requested
if(ENABLE_GUI)
	list(APPEND source_list src/Display.cxx)
endif()

add_library(collider_lib ${source_list})
target_link_libraries(collider_lib Arma::Armadillo)
target_include_directories(collider_lib PUBLIC include)

if(ENABLE_GUI)
	# lightweight GUI: plain SDL2 only (no SDL2_image, no extra GUI toolkits)
	find_package(SDL2 REQUIRED)
	if(TARGET SDL2::SDL2)
		target_link_libraries(collider_lib SDL2::SDL2)
	else()
		target_include_directories(collider_lib PUBLIC ${SDL2_INCLUDE_DIRS})
		target_link_libraries(collider_lib ${SDL2_LIBRARIES})
	endif()

	# expose the GUI to the code and point it at the image assets
	target_compile_definitions(collider_lib PUBLIC
		ENABLE_GUI
		COLLIDER_RES_DIR="${CMAKE_CURRENT_SOURCE_DIR}/res"
	)
endif()

# executable
add_executable(collider collider.cxx)
target_link_libraries(collider collider_lib)

enable_testing()
add_subdirectory(test)
