
# build tracy
# tracy options

set(tracy_CMAKE_ARGS
)

get_externalproject_options(tracy ${DEPS_IGNORE_SHA})
ExternalProject_Add(tracy
    DOWNLOAD_DIR ${DEPS_DOWNLOAD_DIR}/tracy
    SOURCE_DIR ${DEPS_BUILD_DIR}/src/tracy

    #LIST_SEPARATOR | # alternate list separator
    CMAKE_ARGS ${DEPS_CMAKE_ARGS} ${tracy_CMAKE_ARGS}
    ${EXTERNALPROJECT_OPTIONS}

)


