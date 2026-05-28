project(updater)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS
		Core
		Gui
		Widgets)

qt_standard_project_setup()

# Add your .qrc file as a resource
qt_add_resources(${PROJECT_NAME} RESOURCES ${CMAKE_SOURCE_DIR}/app/resources.qrc)

set(UPDATER_SOURCES Updater.cpp # ${UPDATER_RESOURCES}
		UpdaterProgressDialog.cpp
		UpdaterProgressDialog.h
		UpdateWorker.cpp
		UpdateWorker.h
		../../include/buraq.h
		../../include/buraq.cpp
)
set(UPDATER_HEADERS ${CMAKE_SOURCE_DIR}/include/IToolsAPI.h)

if (CMAKE_BUILD_TYPE STREQUAL "Release" AND WIN32)
	add_executable(${PROJECT_NAME} WIN32 ${UPDATER_SOURCES} ${CMAKE_SOURCE_DIR}/app/res/buraq.rc)
elseif (CMAKE_BUILD_TYPE STREQUAL "Debug" AND WIN32)
	add_executable(${PROJECT_NAME} ${UPDATER_SOURCES} ${CMAKE_SOURCE_DIR}/app/res/buraq.rc)
endif ()

# Adds Qt, Boost, and Standard Library headers that are used everywhere
target_precompile_headers(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include/pch.h)

target_link_libraries(${PROJECT_NAME} PRIVATE
		Qt6::Core
		Qt6::Gui
		Qt6::Widgets
)

if (WIN32)
	# --- DEPLOYMENT LOGIC ---
	# This command adds a new target to your project named "deploy"
	string(TOLOWER "${CMAKE_BUILD_TYPE}" BUILD_TYPE)
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E echo "Deploying for ${PROJECT_NAME} $<CONFIG>..."
			# The COMMAND runs windeployqt using the Qt6::windeployqt target we found earlier
			COMMAND Qt6::windeployqt
#			"--dir=plugins"
			# Use a generator expression to add the --release or --debug flag automatically
			#"--${BUILD_TYPE}"
			# Use a generator expression to get the path to your .exe file at build time
			"$<TARGET_FILE:${PROJECT_NAME}>"
			# This ensures the deployment command runs from the directory where the .exe is located
			WORKING_DIRECTORY "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
			COMMENT "Running windeployqt for $<TARGET_FILE:${PROJECT_NAME}>"
			# This makes the output appear nicely in the terminal
			USES_TERMINAL
	)
endif ()
