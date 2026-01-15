#include "application.hpp"
#include "rendering/views/default_view.hpp"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "util/window_handler.hpp"
#include "util/logging.hpp"
#include <stdexcept>
#include <iostream>

Application* g_app_instance = nullptr;

Application::Application(std::string window_title, int width, int height)
    : window_(nullptr)
    , window_title_(window_title)
    , window_width_(width)
    , window_height_(height)
{
    g_app_instance = this;
}

Application::~Application() {
    EventQueue::getInstance().unsubscribe(&view_change_listener_);
    g_app_instance = nullptr;
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
    ImGuiIO& io = ImGui::GetIO();
    
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    log_debug("ImGui docking and viewports enabled");
    
    ImGui::StyleColorsDark();
    
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }
    
    ImGui_ImplGlfw_InitForOpenGL(window_, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    log_info("ImGui initialized successfully");
}

void Application::init() {
    log_info("Initializing application");
    initGLFW();
    initGL3W();
    initImGui();
    
    // Setup view change listener
    view_change_listener_.filter = "views/set_view";
    view_change_listener_.callback = [this](Event_ptr& event) {
        auto view_event = dynamic_cast<Rendering::SetViewEvent*>(event.get());
        if (view_event) {
            log_info("View change event received");
            setView(view_event->getView());
        }
    };
    EventQueue::getInstance().subscribe(&view_change_listener_);
    log_debug("View change listener subscribed");
    
    // Set initial view
    current_view_ = std::make_shared<Rendering::DefaultView>();
    log_info("Application initialized successfully");
}

void Application::loop() {
    log_info("Starting main loop");
    WindowHandler::getInstance().setMainWindow(window_);
    
    while (!glfwWindowShouldClose(window_)) {
        glfwPollEvents();
        EventQueue::getInstance().pollEvents();
        
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        
        if (current_view_) {
            current_view_->render();
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

void Application::setView(std::shared_ptr<Rendering::View> view) {
    log_debug("Setting new view");
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
