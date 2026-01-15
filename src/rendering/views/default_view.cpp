#include "rendering/views/default_view.hpp"

namespace Rendering {
    DefaultView:: DefaultView() {
        // Constructor - initialize any view-specific data here
    }
    
    void DefaultView::render() {
        // Create a simple window
        ImGui::Begin("Default View");
        
        ImGui::Text("Welcome to ImGui Viewer!");
        ImGui:: Separator();
        
        ImGui::TextWrapped(
            "This is a bare-bones ImGui application using GLFW and OpenGL.  "
            "It uses a view-based architecture that makes it easy to add new views."
        );
        
        ImGui:: Spacing();
        
        if (ImGui::CollapsingHeader("Getting Started")) {
            ImGui:: BulletText("Create new view classes by inheriting from Rendering::View");
            ImGui::BulletText("Override the render() method to define your UI");
            ImGui::BulletText("Use app.setView() to switch between views");
        }
        
        ImGui::Spacing();
        ImGui::Text("Application average %. 3f ms/frame (%.1f FPS)", 
                    1000.0f / ImGui:: GetIO().Framerate, 
                    ImGui::GetIO().Framerate);
        
        ImGui::End();
    }
}
