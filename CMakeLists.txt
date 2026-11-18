cmake_minimum_required(VERSION 3.22)

project(ArcadePortfolio VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(ARCADE_BUILD_LAUNCHER "Build the Qt launcher" ON)
option(ARCADE_BUILD_SFML_TEMPLATE "Build the SFML template game" ON)
set(ARCADE_DEPLOYMENT_DIR "${CMAKE_SOURCE_DIR}/Deployment" CACHE PATH "Development deployment directory")

include(CTest)
include(cmake/ArcadeSDKDependencies.cmake)
add_subdirectory(Source/ArcadeSDK)

if(ARCADE_BUILD_LAUNCHER)
    find_package(Qt6 6.5 REQUIRED COMPONENTS Quick QuickControls2)
    add_subdirectory(Source/Launcher)
endif()

if(ARCADE_BUILD_SFML_TEMPLATE)
    include(cmake/SFML3.cmake)
    add_subdirectory("Source/Games/Template SFML3")
endif()
