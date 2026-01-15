#include "rendering/views/default_view.hpp"

namespace Rendering {
    DefaultView:: DefaultView() {
        // Constructor
    }
    
    void DefaultView::render() {
        // Make the window fill the entire viewport
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);
        
        // No title bar, no resize, no move
        ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | 
                                 ImGuiWindowFlags_NoMove | 
                                 ImGuiWindowFlags_NoResize |
                                 ImGuiWindowFlags_NoBringToFrontOnFocus;
        
        ImGui:: Begin("MainView", nullptr, flags);
        
        ImGui::Text("Welcome to ImGui Viewer!");
        ImGui:: Separator();
        
        ImGui::TextWrapped(
            "This view fills the whole window. "
            "Add all your ImGui code right here in the render() function."
        );
        
        ImGui:: Spacing();
        
        if (ImGui::Button("Switch to Another View")) {
            // Example: Post event to switch views
            // EventQueue::getInstance().post(
            //     Event_ptr(new SetViewEvent(std::make_shared<AnotherView>()))
            // );
        }
        
        ImGui:: Spacing();
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 
                    1000.0f / ImGui::GetIO().Framerate, 
                    ImGui::GetIO().Framerate);
        
        ImGui::End();
    }
}
