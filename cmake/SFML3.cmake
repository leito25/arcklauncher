include(FetchContent)

set(BUILD_SHARED_LIBS OFF)
set(SFML_BUILD_AUDIO OFF CACHE BOOL "Build SFML audio")
set(SFML_BUILD_NETWORK OFF CACHE BOOL "Build SFML network")
set(SFML_BUILD_EXAMPLES OFF CACHE BOOL "Build SFML examples")
set(SFML_BUILD_TEST_SUITE OFF CACHE BOOL "Build SFML tests")
FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.0.2
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(SFML)
