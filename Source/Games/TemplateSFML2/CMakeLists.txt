cmake_minimum_required(VERSION 3.22)
project(TemplateSFML2Project LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(ARCADE_DEPLOYMENT_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../Deployment" CACHE PATH "Development deployment directory")

include(CTest)
include(FetchContent)
set(BUILD_SHARED_LIBS OFF)
set(SFML_BUILD_AUDIO OFF CACHE BOOL "Build SFML audio")
set(SFML_BUILD_NETWORK OFF CACHE BOOL "Build SFML network")
set(SFML_BUILD_EXAMPLES OFF CACHE BOOL "Build SFML examples")
set(SFML_BUILD_DOC OFF CACHE BOOL "Build SFML documentation")
FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 2.6.2
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(SFML)

add_executable(TemplateSFML2 main.cpp)
target_link_libraries(TemplateSFML2 PRIVATE sfml-graphics)
set(game_deployment_dir "${ARCADE_DEPLOYMENT_DIR}/games/TemplateSFML2")
file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/meta.json"
    CONTENT "{\n  \"title\": \"Circle Sample (SFML 2)\",\n  \"executable\": \"$<TARGET_FILE_NAME:TemplateSFML2>\"\n}\n"
)
add_custom_target(DeployTemplateSFML2 ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory "${game_deployment_dir}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:TemplateSFML2>" "${game_deployment_dir}/"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/meta.json" "${game_deployment_dir}/meta.json"
    DEPENDS TemplateSFML2
    VERBATIM
)
if(BUILD_TESTING)
    add_test(NAME SFML2Deployment
        COMMAND ${CMAKE_COMMAND}
            "-DGAME_DIR=${game_deployment_dir}"
            "-DEXPECTED_EXECUTABLE=$<TARGET_FILE_NAME:TemplateSFML2>"
            -P "${CMAKE_CURRENT_LIST_DIR}/../../../cmake/CheckGameDeployment.cmake"
    )
endif()
