if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    cmake_minimum_required(VERSION 3.22)
    project(TemplateSFML3Project LANGUAGES CXX)
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)
    set(ARCADE_DEPLOYMENT_DIR "${CMAKE_BINARY_DIR}/deployment" CACHE PATH "Development deployment directory")
    include(CTest)
    include("${CMAKE_CURRENT_LIST_DIR}/../../../cmake/ArcadeSDKDependencies.cmake")
    add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/../../ArcadeSDK" "${CMAKE_BINARY_DIR}/ArcadeSDK")
    include("${CMAKE_CURRENT_LIST_DIR}/../../../cmake/SFML3.cmake")
endif()

add_executable(TemplateSFML main.cpp)
target_link_libraries(TemplateSFML PRIVATE SFML::Graphics ArcadeSDK)

if(WIN32)
    add_custom_command(TARGET TemplateSFML POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:ArcadeSDK>" "$<TARGET_FILE_DIR:TemplateSFML>"
    )
endif()

set(game_deployment_dir "${ARCADE_DEPLOYMENT_DIR}/games/TemplateSFML")
file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/meta.json"
    CONTENT "{\n  \"id\": \"template-sfml3\",\n  \"title\": \"Circle Sample (SFML 3)\",\n  \"executable\": \"$<TARGET_FILE_NAME:TemplateSFML>\"\n}\n"
)
if(BUILD_TESTING)
    add_test(NAME SFML3Deployment
        COMMAND ${CMAKE_COMMAND}
            "-DGAME_DIR=${game_deployment_dir}"
            "-DEXPECTED_EXECUTABLE=$<TARGET_FILE_NAME:TemplateSFML>"
            "-DREQUIRED_LIBRARY=$<TARGET_FILE_NAME:ArcadeSDK>"
            -P "${CMAKE_CURRENT_LIST_DIR}/../../../cmake/CheckGameDeployment.cmake"
    )
endif()
add_custom_target(DeployTemplateSFML ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory "${game_deployment_dir}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:TemplateSFML>" "${game_deployment_dir}/"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:ArcadeSDK>" "${game_deployment_dir}/"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/meta.json" "${game_deployment_dir}/meta.json"
    DEPENDS TemplateSFML
    VERBATIM
)
