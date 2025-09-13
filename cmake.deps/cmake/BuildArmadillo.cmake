# prefix and cmake args
list(APPEND _armadillo_prefix_paths "${CMAKE_PREFIX_PATH}") # propagate prefix paths
list(JOIN _armadillo_prefix_paths ";" _armadillo_prefix_path_string)
string(REPLACE ";" "|" _armadillo_prefix_path_fixed "${_armadillo_prefix_path_string}")

set(ARMADILLO_URL https://gitlab.com/conradsnicta/armadillo-code/-/archive/14.6.2/armadillo-code-14.6.2.tar.gz )
set(ARMADILLO_SHA256 c86559b590e757caa2b32bc60602a14b37cecf66f7aff49b1d90916f4d89f105 )

# armadillo options
# from bash script
# cmake -B build -DCMAKE_INSTALL_PREFIX=~/.local -DOPENBLAS_PROVIDES_LAPACK=true -DARMA_USE_SUPERLU=true;
set(armadillo_CMAKE_ARGS
    -D CMAKE_PREFIX_PATH=${_armadillo_prefix_path_fixed}
    -D STATIC_LIB=OFF
    -D ARMA_USE_WRAPPER=false
    -D ARMA_USE_BLAS=true
    -D ARMA_USE_SUPERLU=true
    -D BLA_VENDOR=OpenBLAS
    -D BLAS_LIBRARIES=${BLAS_LIBRARIES}
    -D LAPACK_LIBRARIES=${LAPACK_LIBRARIES}
    -D BLAS_INCLUDES=${BLAS_INCLUDES}
    -D LAPACK_INCLUDES=${LAPACK_INCLUDES}
  )

  # might want to enable this earlier
if(WIN32)
    set(armadillo_CMAKE_ARGS ${armadillo_CMAKE_ARGS} 
# this is true on linux
        -D OPENBLAS_PROVIDES_LAPACK=false
    )
elseif(UNIX)
    set(armadillo_CMAKE_ARGS ${armadillo_CMAKE_ARGS} 
# this is true on linux
        -D OPENBLAS_PROVIDES_LAPACK=true
    )
endif()

  #list(APPEND armadillo_CMAKE_ARGS -D WITH_LUA_ENGINE=${LUA_ENGINE})

  # pull repo

get_externalproject_options(armadillo ${DEPS_IGNORE_SHA})
ExternalProject_Add(armadillo
    #DEPENDS lua_compat53
  DOWNLOAD_DIR ${DEPS_DOWNLOAD_DIR}/armadillo
  SOURCE_DIR ${DEPS_BUILD_DIR}/src/armadillo

  LIST_SEPARATOR | # alternate list separator
  CMAKE_ARGS ${DEPS_CMAKE_ARGS} ${armadillo_CMAKE_ARGS}
  ${EXTERNALPROJECT_OPTIONS}
  )

add_dependencies(armadillo superlu)


