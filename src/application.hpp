#ifndef APPLICATION_H
#define APPLICATION_H

#include <string>
#include <memory>
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "rendering/views.hpp"
#include "util/events.hpp"

class Application {
private:
    GLFWwindow* window_;
    std::string window_title_;
    int window_width_;
    int window_height_;
    
    std::shared_ptr<Rendering::View> current_view_;
    
    Listener view_change_listener_;
    
    void initGLFW();
    void initGL3W();
    void initImGui();
    void shutdown();
    
    static void glfwErrorCallback(int error, const char* description);
    
public:
    Application(std::string window_title, int width, int height);
    ~Application();
    
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;
    
    void init();
    void loop();
    void setView(std::shared_ptr<Rendering::View> view);
    
    GLFWwindow* getWindow() { return window_; }
};

#endif // APPLICATION_H
