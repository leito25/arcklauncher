cmake_minimum_required(VERSION 3.22)

project(ArcadePortfolio VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(ARCADE_BUILD_LAUNCHER "Build the Qt launcher" ON)
option(ARCADE_BUILD_SFML_TEMPLATE "Build the SFML template game" ON)

include(CTest)

if(ARCADE_BUILD_LAUNCHER)
    find_package(Qt6 6.5 REQUIRED COMPONENTS Quick QuickControls2)
    add_subdirectory(Source/Launcher)
endif()

if(ARCADE_BUILD_SFML_TEMPLATE)
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
    add_subdirectory(Source/Games/TemplateSFML)
endif()
