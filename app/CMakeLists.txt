project(buraq LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS
        Core
        Gui
        Widgets
        Sql
        Xml
        Svg
        Network)
find_package(Qt6SvgWidgets CONFIG REQUIRED)

qt_standard_project_setup() # Recommended for standard Qt setup

find_package(CURL REQUIRED)

find_package(Boost REQUIRED COMPONENTS property_tree)

find_package(unofficial-sqlite3 CONFIG REQUIRED)

set(ITOOLS_MAIN_SOURCES
        main.cpp
        PluginManager.cpp
        PluginManager.h
        FileObject.cpp
        ui/CommonWidget.h
)

set(ITOOLS_UI_SOURCES
        ui/app_ui/AppUi.cpp
        ui/Filters/Toolbar/ToolBarEvent.cpp
        ui/ToolBar.cpp
        ui/editor/Editor.cpp
        ui/CustomDrawer.cpp
        ui/output_display/OutputDisplay.cpp
        ui/CustomLabel.cpp
)

set(ITOOLS_UTILS_SOURCES
        utils/Minion.cpp
        utils/Config.cpp
        utils/Utils.cpp
)

set(ITOOLS_RESOURCES
        res/buraq.rc
        resources.qrc
)

set(ITOOLS_ALL_SOURCES
        ${ITOOLS_MAIN_SOURCES}
        ${ITOOLS_UI_SOURCES}
        ${ITOOLS_UTILS_SOURCES}
        ${ITOOLS_RESOURCES}
)

set(ITOOLS_HEADERS
        PluginManager.h
        FileObject.h
        ui/app_ui/AppUi.h
        ui/Filters/Toolbar/ToolBarEvent.h
        ui/ToolButton.h
        ui/ToolBar.h
        ui/IconButton.h
        ui/editor/Editor.h
        ui/EditorMargin.h
        ui/CustomDrawer.h
        ui/FilePathLabel.h
        ui/output_display/OutputDisplay.h
        ui/CustomLabel.h
        ui/CommonWidget.h
        utils/Minion.h
        ui/editor/CodeRunner.cpp
        ui/editor/CodeRunner.h
        ui/EditorMargin.cpp
        ui/editor/LineNumberAreaWidget.cpp
        ui/editor/LineNumberAreaWidget.h
        ui/frameless_window/FramelessWindow.cpp
        ui/frameless_window/FramelessWindow.h
        ${CMAKE_SOURCE_DIR}/include/version.h
        app_version.cpp
        app_version.cpp
        app_version.h
        ${CMAKE_SOURCE_DIR}/include/network.cpp
        ${CMAKE_SOURCE_DIR}/include/network.h
        clients/VersionClient/VersionRepository.cpp
        clients/VersionClient/VersionRepository.h
        ui/dialog/VersionUpdateDialog.cpp
        ui/dialog/VersionUpdateDialog.h
        ui/Filters/ThemeManager/ThemeManager.h
        ui/Filters/ThemeManager/ThemeManager.cpp
        ../include/buraq.h
        database/db_conn.cpp
        ../include/buraq.cpp
        clients/PSClient/PSClient.cpp
        clients/PSClient/PSClient.h
        ManagedProcess/ManagedProcess.h
        ui/settings/Dialog/SettingsDialog.cpp
        ui/settings/Dialog/SettingsDialog.h
        ui/settings/UserSettings.h
        ui/settings/SettingManager/SettingsManager.cpp
        ui/settings/SettingManager/SettingsManager.h
        ui/DraggableWidget/DraggableWidget.cpp
        ui/DraggableWidget/DraggableWidget.h
        utils/Frame/Frame.cpp
        utils/Frame/Frame.h
)

if (CMAKE_BUILD_TYPE STREQUAL "Release" AND WIN32)
    add_executable(${PROJECT_NAME} WIN32 ${ITOOLS_ALL_SOURCES} ${ITOOLS_HEADERS})
    add_definitions(-DQT_MESSAGELOGGING_DEBUG)
elseif (CMAKE_BUILD_TYPE STREQUAL "Debug" AND WIN32)
    add_executable(${PROJECT_NAME} ${ITOOLS_ALL_SOURCES} ${ITOOLS_HEADERS})
endif ()

# Adds Qt, Boost, and Standard Library headers that are used everywhere
#target_precompile_headers(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include/pch.h)

# Embed resources into the executable
qt_add_resources(${PROJECT_NAME} RESOURCES resources.qrc)

target_include_directories(${PROJECT_NAME} PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/ui" # For ui headers
        "${CMAKE_CURRENT_SOURCE_DIR}/utils" # For utils headers
        "${CMAKE_SOURCE_DIR}/include" # For PluginInterface.h, IToolsAPI.h
        "${CMAKE_CURRENT_SOURCE_DIR}" # For headers in the current source dir
)

target_link_libraries(${PROJECT_NAME} PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Sql
        Qt6::Xml
        Qt6::Svg
        Qt6::SvgPrivate
        Qt6::SvgWidgets
        Qt6::SvgWidgetsPrivate
        CURL::libcurl
        Boost::property_tree
        unofficial::sqlite3::sqlite3
        Qt6::Network
)

# Conditionally add static linking flags for MinGW/GCC
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND WIN32)
    # Get the directory of the C++ compiler. In an MSYS2 MinGW setup,
    # this is typically where the required runtime DLLs are also located.
    get_filename_component(MINGW_RUNTIME_BIN_DIR "${CMAKE_CXX_COMPILER}" DIRECTORY)

    message(STATUS "GCC detected, adding static linking flags for libgcc and libstdc++")
    target_link_options(${PROJECT_NAME} PRIVATE -static-libgcc -static-libstdc++ -lpthread)
else ()
    message(STATUS "Compiler is not GCC, skipping static linking flags for libgcc and libstdc++ (${CMAKE_CXX_COMPILER_ID})")
endif ()

get_target_property(QT_INSTALLATION_DLL_PATH Qt::Core LOCATION)
get_filename_component(INSTALLATION_DLL_DIR "${QT_INSTALLATION_DLL_PATH}" DIRECTORY)

set(QT_THIRD_PARTY_DEPENDENCY_DLLS "libsqlite3.dll")

foreach (DLL_FILENAME ${QT_THIRD_PARTY_DEPENDENCY_DLLS})
    find_file(LIB_SQLITE_PATH
            NAMES ${DLL_FILENAME}
            PATHS
            "${INSTALLATION_DLL_DIR}"
            "${INSTALLATION_DLL_DIR}/../debug/bin"
            REQUIRED
    )
    message(STATUS "LIB_SQLITE_PATH for copy: ${LIB_SQLITE_PATH}")

    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${LIB_SQLITE_PATH}" "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
            COMMENT "Copying ${LIB_SQLITE_PATH} to build directory"
    )
endforeach ()

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/icons/buraq.ico "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
)

set(MANAGED_LIBRARY_PUBLISH_DIR "${CMAKE_SOURCE_DIR}/CSharpManaged/bin/Release/net9.0/win-x64/publish")

# After the build is complete, copy the ENTIRE contents of the publish
# directory to the same directory as your final buraq.exe.
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${MANAGED_LIBRARY_PUBLISH_DIR}/" # Source: The entire publish folder
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/PS.Bridge"      # Destination: Where buraq.exe is
        COMMENT "Copying all self-contained .NET components..."
)

if (WIN32)
    # --- DEPLOYMENT LOGIC ---
    # This command adds a new target to your project named "deploy"
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E echo "Deploying for ${PROJECT_NAME} $<CONFIG>..."
            COMMAND Qt6::windeployqt
            #"--dir=plugins"
            # Use a generator expression to add the --release or --debug flag automatically
            # Use a generator expression to get the path to your .exe file at build time
            "$<TARGET_FILE:${PROJECT_NAME}>"
            # This ensures the deployment command runs from the directory where the .exe is located
            WORKING_DIRECTORY "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
            # This makes sure ${PROJECT_NAME}.exe is built BEFORE this command tries to run
            DEPENDS ${PROJECT_NAME}
            COMMENT "Running windeployqt for $<TARGET_FILE:${PROJECT_NAME}>"
            # This makes the output appear nicely in the terminal
            USES_TERMINAL
    )
endif ()
