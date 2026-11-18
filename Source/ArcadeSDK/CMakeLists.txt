add_library(ArcadeSDK SHARED ArcadeSDK.cpp ArcadeSDK.h)
target_compile_definitions(ArcadeSDK PRIVATE ARCADE_SDK_BUILD)
target_include_directories(ArcadeSDK PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(ArcadeSDK PRIVATE nlohmann_json::nlohmann_json)
if(WIN32)
    target_link_libraries(ArcadeSDK PRIVATE shell32 ole32)
endif()

if(BUILD_TESTING)
    add_executable(ArcadeSDKTests tests/ArcadeSDKTests.cpp)
    target_link_libraries(ArcadeSDKTests PRIVATE ArcadeSDK nlohmann_json::nlohmann_json)
    if(WIN32)
        add_custom_command(TARGET ArcadeSDKTests POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "$<TARGET_FILE:ArcadeSDK>" "$<TARGET_FILE_DIR:ArcadeSDKTests>"
        )
    endif()
    add_test(NAME ArcadeSDKTests COMMAND ArcadeSDKTests)
endif()
