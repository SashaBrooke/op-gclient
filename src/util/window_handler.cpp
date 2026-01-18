#include <GL/gl3w.h>
#include "window_handler.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

void WindowHandler::setMainWindow(GLFWwindow* window) {
    main_window_ = window;
    windows_[window] = {window, 0, nullptr};
}

GLFWwindow* WindowHandler::createWindow(const std::string& title, int width, int height,
                                        std::function<void()> render_callback, int z_index) {
    if (! main_window_) {
        return nullptr;
    }
    
    // Create window sharing OpenGL context with main window
    GLFWwindow* new_window = glfwCreateWindow(width, height, title.c_str(), 
                                               nullptr, main_window_);
    if (!new_window) {
        return nullptr;
    }
    
    // Store window info
    windows_[new_window] = {new_window, z_index, render_callback};
    
    // Setup ImGui for this window
    glfwMakeContextCurrent(new_window);
    ImGui_ImplGlfw_InitForOpenGL(new_window, true);
    
    return new_window;
}

void WindowHandler::removeWindow(GLFWwindow* window) {
    if (window == main_window_) {
        return; // Don't remove main window
    }
    
    auto it = windows_.find(window);
    if (it != windows_.end()) {
        glfwDestroyWindow(window);
        windows_.erase(it);
    }
}

void WindowHandler::renderAll() {
    for (auto& [window, info] : windows_) {
        if (window == main_window_) {
            continue; // Main window is rendered by Application
        }
        
        if (glfwWindowShouldClose(window)) {
            removeWindow(window);
            continue;
        }
        
        glfwMakeContextCurrent(window);
        
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        
        if (info.render_callback) {
            info.render_callback();
        }
        
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        
        glfwSwapBuffers(window);
    }
}
