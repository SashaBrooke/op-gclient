#include "rendering/views/default_view.hpp"
#include "core/connection_manager.hpp"
#include "util/logging.hpp"
#include "imgui.h"

namespace Rendering {

DefaultView::DefaultView() {
    log_debug("DefaultView created");
}

void DefaultView::render() {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | 
                             ImGuiWindowFlags_NoMove | 
                             ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoBringToFrontOnFocus;
    
    ImGui::Begin("MainView", nullptr, flags);
    
    auto& conn = ConnectionManager::getInstance();
    
    // Connection Status Section
    ImGui::SeparatorText("Connection Status");
    
    // Color-coded status
    if (conn. isConnected()) {
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "● Connected");
        ImGui::SameLine();
        ImGui::Text("to %s", conn.getDeviceInfo().c_str());
    } else if (conn.isConnecting()) {
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "● Connecting...");
    } else if (conn.hasError()) {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "● Error");
        ImGui::SameLine();
        ImGui::TextWrapped("%s", conn.getErrorMessage().c_str());
    } else {
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "● Disconnected");
    }
    
    ImGui:: Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    // Connection Controls
    ImGui::SeparatorText("Connection");
    
    if (conn.isDisconnected() || conn.hasError()) {
        // Show connection options when disconnected
        static int connection_type = 0;  // 0 = Serial, 1 = Network
        ImGui::RadioButton("Serial", &connection_type, 0);
        ImGui::SameLine();
        ImGui::RadioButton("Network", &connection_type, 1);
        
        ImGui::Spacing();
        
        if (connection_type == 0) {
            // Serial connection
            static char port_buffer[128] = "/dev/ttyUSB0";
            ImGui:: InputText("Serial Port", port_buffer, sizeof(port_buffer));
            
            if (ImGui::Button("Connect Serial")) {
                log_info("Attempting to connect to serial port: {}", port_buffer);
                conn.setConnecting(ConnectionManager::ConnectionType::Serial, port_buffer);
                // TODO: Actual serial connection logic here
                // For now, simulate success: 
                conn.setConnected();
            }
        } else {
            // Network connection
            static char ip_buffer[128] = "192.168.1.100";
            static int port = 3883;
            ImGui::InputText("IP Address", ip_buffer, sizeof(ip_buffer));
            ImGui::InputInt("Port", &port);
            
            if (ImGui::Button("Connect Network")) {
                std::string device_info = std::string(ip_buffer) + ":" + std::to_string(port);
                log_info("Attempting to connect to network: {}", device_info);
                conn.setConnecting(ConnectionManager::ConnectionType::Network, device_info);
                // TODO: Actual network connection logic here
                // For now, simulate success:
                conn.setConnected();
            }
        }
    } else if (conn.isConnected()) {
        if (ImGui::Button("Disconnect")) {
            log_info("Disconnecting from device");
            conn.setDisconnected();
        }
    } else if (conn.isConnecting()) {
        ImGui::Text("Connecting...");
        if (ImGui::Button("Cancel")) {
            conn.setDisconnected();
        }
    }
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    // Device Controls - DISABLED when not connected
    ImGui::SeparatorText("Device Controls");
    
    // Everything below is disabled unless connected
    ImGui::BeginDisabled(! conn.isConnected());
    {
        if (ImGui::Button("Start Recording")) {
            log_info("Start recording clicked");
        }
        
        ImGui::SameLine();
        
        if (ImGui::Button("Stop Recording")) {
            log_info("Stop recording clicked");
        }
        
        ImGui::Spacing();
        
        if (ImGui::Button("Calibrate Device")) {
            log_info("Calibrate device clicked");
        }
        
        ImGui::Spacing();
        
        static float slider_value = 0.5f;
        ImGui::SliderFloat("Device Parameter", &slider_value, 0.0f, 1.0f);
        
        ImGui::Spacing();
        
        if (ImGui::CollapsingHeader("Advanced Settings")) {
            static bool option1 = false;
            static bool option2 = true;
            ImGui::Checkbox("Option 1", &option1);
            ImGui:: Checkbox("Option 2", &option2);
        }
    }
    ImGui::EndDisabled();
    
    // Info footer (always visible)
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 
                1000.0f / ImGui::GetIO().Framerate, 
                ImGui::GetIO().Framerate);
    
    ImGui::End();
}

} // namespace Rendering
