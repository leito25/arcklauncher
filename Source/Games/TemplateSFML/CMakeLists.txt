add_executable(TemplateSFML main.cpp)
target_link_libraries(TemplateSFML PRIVATE SFML::Graphics)

set(game_deployment_dir "${PROJECT_SOURCE_DIR}/Deployment/games/TemplateSFML")
file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/meta.json"
    CONTENT "{\n  \"title\": \"Circle Sample\",\n  \"executable\": \"$<TARGET_FILE_NAME:TemplateSFML>\"\n}\n"
)
add_custom_target(DeployTemplateSFML ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory "${game_deployment_dir}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:TemplateSFML>" "${game_deployment_dir}/"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/meta.json" "${game_deployment_dir}/meta.json"
    DEPENDS TemplateSFML
    VERBATIM
)
