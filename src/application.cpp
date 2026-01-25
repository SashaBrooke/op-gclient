#include "application.hpp"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "implot.h"
#include "util/window_handler.hpp"
#include "util/logging.hpp"
#include "util/events.hpp"
#include "core/connection_manager.hpp"
#include <stdexcept>
#include <iostream>

Application::Application(std::string window_title, int width, int height)
    : window_(nullptr)
    , window_title_(window_title)
    , window_width_(width)
    , window_height_(height)
{
    log_info("=== Application Starting ===");
}

Application::~Application() {
    shutdown();
}

void Application::glfwErrorCallback(int error, const char* description) {
    log_error("GLFW Error {}: {}", error, description);
}

void Application::initGLFW() {
    log_info("Initializing GLFW");
    glfwSetErrorCallback(glfwErrorCallback);
    
    if (!glfwInit()) {
        log_critical("Failed to initialize GLFW");
        throw std::runtime_error("Failed to initialize GLFW");
    }
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    log_debug("macOS: Enabled forward compatibility");
#endif
    
    window_ = glfwCreateWindow(window_width_, window_height_, 
                               window_title_.c_str(), nullptr, nullptr);
    if (window_ == nullptr) {
        log_critical("Failed to create GLFW window");
        throw std::runtime_error("Failed to create GLFW window");
    }
    
    glfwMakeContextCurrent(window_);
    glfwSwapInterval(1);
    log_info("GLFW initialized successfully");
}

void Application::initGL3W() {
    log_info("Initializing OpenGL loader (gl3w)");
    if (gl3wInit() != 0) {
        log_critical("Failed to initialize gl3w");
        throw std::runtime_error("Failed to initialize OpenGL loader (gl3w)");
    }
    log_info("OpenGL loader initialized successfully");
}

void Application::initImGui() {
    log_info("Initializing ImGui");
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    log_debug("ImGui docking and viewports enabled");

    // Handle high DPI displays
    float xscale, yscale;
    glfwGetWindowContentScale(window_, &xscale, &yscale);
    log_info("Display scale:  {}x, {}y", xscale, yscale);
    
    ImGui::StyleColorsDark();
    
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // Scale UI for high DPI BEFORE initializing backends
    if (xscale > 1.0f || yscale > 1.0f) {
        float scale = std::max(xscale, yscale);
        style.ScaleAllSizes(scale);
        log_debug("Scaled ImGui style by {}", scale);
    }
    
    ImGui_ImplGlfw_InitForOpenGL(window_, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Load default font at higher resolution for DPI
    if (xscale > 1.0f || yscale > 1.0f) {
        float scale = std::max(xscale, yscale);
        
        ImFontConfig font_config;
        font_config.SizePixels = 13.0f * scale;
        font_config.OversampleH = 2;
        font_config.OversampleV = 2;
        
        io.Fonts->Clear();
        io.Fonts->AddFontDefault(&font_config);
        io.Fonts->Build();
        
        unsigned char* pixels;
        int width, height;
        io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
        
        log_debug("Loaded high-DPI font at {}px", font_config.SizePixels);
    }

    log_info("ImGui initialized successfully");
}

void Application::init() {
    log_info("Initializing application");
    initGLFW();
    initGL3W();
    initImGui();
    
    // Create ViewManager - it handles all view changes via events
    view_manager_ = std::make_unique<Rendering::ViewManager>();
    
    log_info("Application initialized successfully");
}

void Application::loop() {
    log_info("Starting main loop");
    WindowHandler::getInstance().setMainWindow(window_);
    
    while (!glfwWindowShouldClose(window_)) {
        glfwPollEvents();
        
        // Process queued events BEFORE rendering
        Events::EventQueue::getInstance().pollEvents();
        
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        
        // ViewManager renders the current view
        if (view_manager_) {
            view_manager_->render();
        }
        
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window_, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        
        WindowHandler::getInstance().renderAll();
        
        ImGuiIO& io = ImGui::GetIO();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }
        
        glfwSwapBuffers(window_);
    }
    
    log_info("Main loop exited");
}

void Application::shutdown() {
    log_debug("Application:: shutdown() called");

    // IMPORTANT: Disconnect any active connections FIRST
    // This ensures serial/network cleanup happens before GLFW/ImGui shutdown
    log_debug("Disconnecting any active connections");
    ConnectionManager::getInstance().disconnect();

    // Clear view manager and its event subscriptions
    log_debug("Shutting down ViewManager");
    view_manager_.reset();
    
    // Clear all remaining event subscriptions to prevent static destruction issues
    log_debug("Clearing event queue");
    Events::EventQueue::getInstance().clearAll();
    
    log_debug("Shutting down ImGui and GLFW");
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();
    
    if (window_) {
        glfwDestroyWindow(window_);
    }
    glfwTerminate();

    log_info("=== Application Shutting Down ===");
}
