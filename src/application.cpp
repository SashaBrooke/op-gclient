#include "application.hpp"
#include "rendering/views/default_view.hpp"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "util/window_handler.hpp"
#include <stdexcept>
#include <iostream>

Application::Application(std::string window_title, int width, int height)
    : window_(nullptr)
    , window_title_(window_title)
    , window_width_(width)
    , window_height_(height)
{
}

Application::~Application() {
    // Unsubscribe from events before shutdown
    EventQueue::getInstance().unsubscribe(&view_change_listener_);
    shutdown();
}

void Application::glfwErrorCallback(int error, const char* description) {
    std::cerr << "GLFW Error " << error << ": " << description << std:: endl;
}

void Application:: initGLFW() {
    glfwSetErrorCallback(glfwErrorCallback);
    
    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialize GLFW");
    }
    
    // GL 3.3 + GLSL 330
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    
    window_ = glfwCreateWindow(window_width_, window_height_, 
                               window_title_.c_str(), nullptr, nullptr);
    if (window_ == nullptr) {
        throw std::runtime_error("Failed to create GLFW window");
    }
    
    glfwMakeContextCurrent(window_);
    glfwSwapInterval(1);
}

void Application::initGL3W() {
    if (gl3wInit() != 0) {
        throw std::runtime_error("Failed to initialize OpenGL loader (gl3w)");
    }
}

void Application::initImGui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    
    ImGui::StyleColorsDark();
    
    ImGuiStyle& style = ImGui:: GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }
    
    ImGui_ImplGlfw_InitForOpenGL(window_, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

void Application:: init() {
    initGLFW();
    initGL3W();
    initImGui();
    
    // Setup view change listener
    view_change_listener_. filter = "views/set_view";
    view_change_listener_.callback = [this](Event_ptr& event) {
        auto view_event = dynamic_cast<Rendering::SetViewEvent*>(event. get());
        if (view_event) {
            setView(view_event->getView());
        }
    };
    EventQueue::getInstance().subscribe(&view_change_listener_);
    
    // Set initial view
    current_view_ = std::make_shared<Rendering::DefaultView>();
}

void Application::loop() {
    // Register main window with handler
    WindowHandler::getInstance().setMainWindow(window_);
    
    while (!glfwWindowShouldClose(window_)) {
        glfwPollEvents();
        
        EventQueue::getInstance().pollEvents();
        
        // Render main window
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        
        if (current_view_) {
            current_view_->render();
        }
        
        ImGui:: Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window_, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        
        // Render additional windows
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
