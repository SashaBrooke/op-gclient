cmake_minimum_required(VERSION 3.12)
project(op-gclient VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Add external directory
add_subdirectory(ext)

# Source files
set(SOURCES
    src/main.cpp
    src/application.cpp
    src/rendering/views.cpp
    src/rendering/views/gimbal_control_view.cpp
    src/util/events.cpp
    src/util/window_handler.cpp
    src/util/logging.cpp
    src/core/connection_manager.cpp
    src/core/serial_connection.cpp
    src/core/network_connection.cpp
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/ext/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/ext/imgui/backends
    ${CMAKE_CURRENT_SOURCE_DIR}/ext/glfw/include
    ${CMAKE_CURRENT_SOURCE_DIR}/ext/gl3w/include
)

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    imgui
    glfw
    gl3w
    spdlog::spdlog
    ${CMAKE_DL_LIBS}
)

# Platform specific
if(UNIX AND NOT APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE pthread)
endif()
