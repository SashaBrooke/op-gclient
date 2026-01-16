#ifndef WINDOW_HANDLER_HPP
#define WINDOW_HANDLER_HPP

#include <GLFW/glfw3.h>
#include <map>
#include <memory>
#include <functional>
#include <string>

/**
 * Manages multiple GLFW windows
 * Based on BM-Segmenter's GLFWwindowHandler
 */
class WindowHandler {
private:
    struct WindowInfo {
        GLFWwindow* window;
        int z_index;
        std::function<void()> render_callback;
    };
    
    std::map<GLFWwindow*, WindowInfo> windows_;
    GLFWwindow* main_window_;
    
    WindowHandler() :  main_window_(nullptr) {}
    
public:
    static WindowHandler& getInstance() {
        static WindowHandler instance;
        return instance;
    }
    
    WindowHandler(const WindowHandler&) = delete;
    void operator=(const WindowHandler&) = delete;
    
    void setMainWindow(GLFWwindow* window);
    GLFWwindow* createWindow(const std::string& title, int width, int height, 
                             std::function<void()> render_callback, int z_index = 0);
    void removeWindow(GLFWwindow* window);
    void renderAll();
    bool hasWindows() const { return !windows_.empty(); }
    GLFWwindow* getMainWindow() const { return main_window_; }
};

#endif // WINDOW_HANDLER_HPP
