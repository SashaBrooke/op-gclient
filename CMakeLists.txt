cmake_minimum_required(VERSION 3.12)
project(op-gclient VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find Boost (for ASIO)
cmake_policy(SET CMP0167 NEW)
find_package(Boost REQUIRED COMPONENTS system)

# Find Protobuf
find_package(Protobuf REQUIRED)

# Add external directory
add_subdirectory(ext)

# Platform specific source files
if(WIN32)
    set(SERIAL_PORT_HELPER_SRC src/platform/serial_port_helper_windows.cpp)
    message(STATUS "Using Windows serial port enumeration")
elseif(UNIX)
    set(SERIAL_PORT_HELPER_SRC src/platform/serial_port_helper_linux.cpp)
    message(STATUS "Using Linux serial port enumeration")
else()
    message(FATAL_ERROR "Unsupported platform for serial port enumeration")
endif()

# Source files
set(SOURCES
    src/main.cpp
    src/application.cpp
    src/rendering/views.cpp
    src/rendering/views/gimbal_control_view.cpp
    src/util/events.cpp
    src/util/window_handler.cpp
    src/util/logging.cpp
    src/core/communication_backend.cpp
    src/core/gimbal_state.cpp
    src/core/packet_codec.cpp
    src/core/packet_ack_manager.cpp
    src/core/serial_transport.cpp
    src/core/network_transport.cpp
    src/core/packet_framer.cpp
    ${SERIAL_PORT_HELPER_SRC}
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/ext/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/ext/imgui/backends
    ${CMAKE_CURRENT_SOURCE_DIR}/ext/implot
    ${CMAKE_CURRENT_SOURCE_DIR}/ext/glfw/include
    ${CMAKE_CURRENT_SOURCE_DIR}/ext/gl3w/include
    ${Protobuf_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    imgui
    implot
    glfw
    gl3w
    spdlog::spdlog
    Boost::system
    ${Protobuf_LIBRARIES}
    ${CMAKE_DL_LIBS}
)

# Platform specific
if(UNIX AND NOT APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE pthread)
endif()
