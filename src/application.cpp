#include "application.hpp"
#include "rendering/views/default_view.hpp"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stdexcept>
#include <iostream>

Application::Application(std::string window_title, int width, int height)
    : window_(nullptr)
    , window_title_(window_title)
    , window_width_(width)
    , window_height_(height)
    , config_flags_(ImGuiConfigFlags_None)
{
}

Application::~Application() {
    shutdown();
}

void Application::glfwErrorCallback(int error, const char* description) {
    std::cerr << "GLFW Error " << error << ": " << description << std::endl;
}

void Application::initGLFW() {
    glfwSetErrorCallback(glfwErrorCallback);
    
    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialize GLFW");
    }
    
    // GL 3.3 + GLSL 330
    const char* glsl_version = "#version 330";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    
    // Create window
    window_ = glfwCreateWindow(window_width_, window_height_, 
                               window_title_.c_str(), nullptr, nullptr);
    if (window_ == nullptr) {
        throw std::runtime_error("Failed to create GLFW window");
    }
    
    glfwMakeContextCurrent(window_);
    glfwSwapInterval(1); // Enable vsync
}

void Application::initGL3W() {
    if (gl3wInit() != 0) {
        throw std:: runtime_error("Failed to initialize OpenGL loader (gl3w)");
    }
}

void Application::initImGui() {
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    
    // Enable docking and viewports
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    
    // When viewports are enabled, tweak WindowRounding/WindowBg 
    // so platform windows can look identical to regular ones.
    ImGuiStyle& style = ImGui:: GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }
    
    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window_, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

void Application::init() {
    initGLFW();
    initGL3W();
    initImGui();
    
    // Set initial view to default view
    current_view_ = std::make_shared<Rendering::DefaultView>();
}

void Application::loop() {
    while (!glfwWindowShouldClose(window_)) {
        // Poll events
        glfwPollEvents();
        
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        
        // Render current view
        if (current_view_) {
            current_view_->render();
        }
        
        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window_, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui:: GetDrawData());
        
        // Update and Render additional Platform Windows
        ImGuiIO& io = ImGui::GetIO();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }
        
        glfwSwapBuffers(window_);
    }
}

void Application::setView(std::shared_ptr<Rendering::View> view) {
    current_view_ = view;
}

void Application::shutdown() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    
    if (window_) {
        glfwDestroyWindow(window_);
    }
    glfwTerminate();
}
