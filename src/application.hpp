#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include <string>
#include <memory>
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include "rendering/views.hpp"

class Application {
public:
    Application(std::string window_title = "Application", 
                int width = 1280, int height = 720);
    ~Application();
    
    void init();
    void loop();
    void shutdown();
    
    GLFWwindow* getWindow() { return window_; }
    
private:
    GLFWwindow* window_;
    std::string window_title_;
    int window_width_;
    int window_height_;
    
    // ViewManager handles all view logic
    std::unique_ptr<Rendering::ViewManager> view_manager_;
    
    void initGLFW();
    void initGL3W();
    void initImGui();
    
    static void glfwErrorCallback(int error, const char* description);
};

#endif // APPLICATION_HPP
