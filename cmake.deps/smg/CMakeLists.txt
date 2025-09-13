project(smg-build)
message(STATUS "Building smg")

# check build type
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# Point CMake at any custom modules we may ship
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" "${PROJECT_SOURCE_DIR}/../cmake")

# prefix and cmake args
list(APPEND _smg_prefix_paths "${CMAKE_PREFIX_PATH}") # propagate prefix paths
list(JOIN _smg_prefix_paths ";" _smg_prefix_path_string)
string(REPLACE ";" "|" _smg_prefix_path_fixed "${_smg_prefix_path_string}")
set(smg_CMAKE_ARGS
    -D CMAKE_PREFIX_PATH=${_smg_prefix_path_fixed}
    -D ENABLE_ARMADILLO=OFF
    -D CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
  )

# build rat dependencies
option(USE_BUNDLED_MAGNUM "use bundled magnum here" ${USE_BUNDLED})

# build superlu
if(USE_BUNDLED_MAGNUM)
    include(BuildCorrade)
    include(BuildMagnum)
    include(BuildMagnumPlugins)
    include(BuildMagnumIntegration)
endif()


# download or use local source
option(USE_LOCAL_SMG "Use local smg source directory" OFF)
if(USE_LOCAL_SMG)
    set(SMG_GIT_REPO "")
    set(SMG_DOWNLOAD_DIR "")
    set(SMG_GIT_TAG "")
    set(SMG_LOCAL_SOURCE_DIR "${PROJECT_SOURCE_DIR}/../../../simple-magnum-gui" CACHE PATH "Path to local smg source directory")
    get_externalproject_options(local ${DEPS_IGNORE_SHA})
    
else()
    set(SMG_GIT_REPO "https://github.com/kolbbond/simple-magnum-gui.git" )
    set(SMG_LOCAL_SOURCE_DIR "${DEPS_BUILD_DIR}/src/smg" CACHE PATH "Path to local smg source directory")
    set(SMG_DOWNLOAD_DIR "${DEPS_DOWNLOAD_DIR}/smg" CACHE PATH "Path to smg download directory")
    set(SMG_GIT_TAG "builddependencies")
    set(SMG_PATCH_COMMAND "${CMAKE_COMMAND} -E echo 'Running submodule update...' && git -C <SOURCE_DIR> submodule update --init --recursive")
    get_externalproject_options(smg ${DEPS_IGNORE_SHA})
endif()

# build with external project
ExternalProject_Add(smg
    GIT_REPOSITORY ${SMG_GIT_REPO}
#GIT_TAG e7c47fc1b2aba58d9b5f1a148351cc09b2fb7e67 
    GIT_TAG ${SMG_GIT_TAG} # can be a branch, tag, or commit hash
    DOWNLOAD_DIR ${SMG_DOWNLOAD_DIR}
    SOURCE_DIR ${SMG_LOCAL_SOURCE_DIR}
    #PATCH_COMMAND ${SMG_PATCH_COMMAND}
    PATCH_COMMAND ${CMAKE_COMMAND} -E echo "Running submodule update..."
                   COMMAND git -C <SOURCE_DIR> submodule update --init --recursive
    BUILD_COMMAND ${DEPS_BUILD_COMMAND}

    CMAKE_ARGS ${DEPS_CMAKE_ARGS} ${smg_CMAKE_ARGS}

    LIST_SEPARATOR |
    ${EXTERNALPROJECT_OPTIONS}

    LOG_UPDATE TRUE
    UPDATE_DISCONNECTED FALSE

# hey put these behind an option
#TEST_COMMAND ${CMAKE_CTEST_COMMAND} --test-dir "${DEPS_BUILD_DIR}/src/smg-build"
)

# @hey: add an add_test() command for more control

  # add dependencies to ensure built
add_dependencies(smg
  armadillo
  magnumintegration 
)

