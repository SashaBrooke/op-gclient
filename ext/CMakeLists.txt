# ImGui
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/imgui)
add_library(imgui STATIC
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PUBLIC
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${CMAKE_CURRENT_SOURCE_DIR}/glfw/include
    ${CMAKE_CURRENT_SOURCE_DIR}/gl3w/include
)

# ImPlot
set(IMPLOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/implot)
add_library(implot STATIC
    ${IMPLOT_DIR}/implot.cpp
    ${IMPLOT_DIR}/implot_items.cpp
    ${IMPLOT_DIR}/implot_demo.cpp
)
target_include_directories(implot PUBLIC ${IMPLOT_DIR})
target_link_libraries(implot PUBLIC imgui)

# GLFW
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(glfw)

# GL3W
set(GL3W_DIR ${CMAKE_CURRENT_SOURCE_DIR}/gl3w)

# Check if gl3w has been generated
if(NOT EXISTS "${GL3W_DIR}/src/gl3w.c")
    message(STATUS "Generating gl3w...")
    find_package(Python3 REQUIRED COMPONENTS Interpreter)
    execute_process(
        COMMAND ${Python3_EXECUTABLE} gl3w_gen.py
        WORKING_DIRECTORY ${GL3W_DIR}
        RESULT_VARIABLE GL3W_RESULT
        OUTPUT_VARIABLE GL3W_OUTPUT
        ERROR_VARIABLE GL3W_ERROR
    )
    
    if(NOT GL3W_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed to generate gl3w: ${GL3W_ERROR}")
    else()
        message(STATUS "gl3w generated successfully")
    endif()
else()
    message(STATUS "gl3w already generated")
endif()

add_library(gl3w STATIC ${GL3W_DIR}/src/gl3w.c)
target_include_directories(gl3w PUBLIC ${GL3W_DIR}/include)

# spdlog
add_subdirectory(spdlog)
