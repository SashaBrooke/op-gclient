#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include <string>
#include <memory>
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include "rendering/views.hpp"
#include "core/gimbal_state.hpp"
#include "core/communication_backend.hpp"

class Application {
public:
    Application(std::string window_title = "OhPossum Gimbal Client", 
                int width = 1280, int height = 720);
    ~Application();
    
    void init();
    void loop();
    void shutdown();
    
    GLFWwindow* getWindow() { return window_; }

    GimbalState& getGimbalState() { return gimbal_state_; }
    CommunicationBackend& getCommBackend() { return comm_backend_; }
    
private:
    GLFWwindow* window_;
    std::string window_title_;
    int window_width_;
    int window_height_;

    GimbalState gimbal_state_;
    CommunicationBackend comm_backend_;

    std::unique_ptr<Rendering::ViewManager> view_manager_;
    
    void initGLFW();
    void initGL3W();
    void initImGui();
    
    static void glfwErrorCallback(int error, const char* description);
};

#endif // APPLICATION_HPP
