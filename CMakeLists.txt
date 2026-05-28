cmake_minimum_required(VERSION 3.27)

if(POLICY CMP0167)
    cmake_policy(SET CMP0167 NEW)
endif()

project(MainProject)

set(CMAKE_ROOT_PROJECT_POSTFIX buraq)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set global output_display directories for executables and libraries
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/build")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib") # For .lib/.a if any
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/build")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/build")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/lib")

# This is the main application directory
add_subdirectory(app)
# plugins are in the ext directory
add_subdirectory(exts)

# This command takes the template file and creates the final qt.conf
# in the same directory as your ITools.exe
configure_file(
    "${CMAKE_SOURCE_DIR}/qt.conf.in"
    "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/qt.conf"
)
