qt_standard_project_setup(REQUIRES 6.5)

qt_add_executable(ArcadeLauncher main.cpp GameScanner.cpp GameScanner.h)
qt_add_qml_module(ArcadeLauncher
    URI ArcadeLauncher
    VERSION 1.0
    QML_FILES Main.qml
)

target_link_libraries(ArcadeLauncher PRIVATE Qt6::Quick Qt6::QuickControls2)

add_custom_target(DeployArcadeLauncher ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory "${ARCADE_DEPLOYMENT_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:ArcadeLauncher>" "${ARCADE_DEPLOYMENT_DIR}/"
    DEPENDS ArcadeLauncher
    VERBATIM
)

if(BUILD_TESTING)
    find_package(Qt6 6.5 REQUIRED COMPONENTS Test)
    qt_add_executable(GameScannerTests
        tests/GameScannerTests.cpp
        GameScanner.cpp
        GameScanner.h
    )
    target_include_directories(GameScannerTests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
    target_link_libraries(GameScannerTests PRIVATE Qt6::Test)
    add_test(NAME GameScannerTests COMMAND GameScannerTests)
    if(WIN32)
        set_tests_properties(GameScannerTests PROPERTIES
            ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:$<TARGET_FILE_DIR:Qt6::Core>"
        )
    endif()
endif()
