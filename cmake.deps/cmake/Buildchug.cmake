
# prefix and cmake args
list(APPEND _chug_prefix_paths "${CMAKE_PREFIX_PATH}") # propagate prefix paths
list(JOIN _chug_prefix_paths ";" _armadillo_prefix_path_string)
string(REPLACE ";" "|" _chug_prefix_path_fixed "${_armadillo_prefix_path_string}")

# chug options
# from bash script
set(chug_CMAKE_ARGS
    -D CMAKE_PREFIX_PATH=${_chug_prefix_path_fixed}
    -D BUILD_EXAMPLES=OFF
  )

  #get_externalproject_options(chug ${DEPS_IGNORE_SHA})
get_externalproject_options(chug local)
ExternalProject_Add(chug
    GIT_REPOSITORY ${CHUG_URL}
    GIT_TAG master
    DOWNLOAD_DIR ${DEPS_DOWNLOAD_DIR}/chug
    SOURCE_DIR ${DEPS_BUILD_DIR}/src/chug

    LIST_SEPARATOR | # alternate list separator
    CMAKE_ARGS ${DEPS_CMAKE_ARGS} ${chug_CMAKE_ARGS}
    ${EXTERNALPROJECT_OPTIONS}
  )

add_dependencies(chug armadillo smg)


