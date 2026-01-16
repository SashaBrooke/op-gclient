#include "rendering/views/default_view.hpp"
#include "core/connection_manager.hpp"
#include "util/logging.hpp"
#include "imgui.h"
#include <utility>

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
    if (conn.isConnected()) {
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Connected");
        ImGui::SameLine();
        ImGui::Text("to %s", conn.getDeviceInfo().c_str());
    } else if (conn.isConnecting()) {
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Connecting...");
    } else if (conn.hasError()) {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Error");
        ImGui::SameLine();
        ImGui::TextWrapped("%s", conn.getErrorMessage().c_str());
    } else {
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Disconnected");
    }
    
    ImGui::Spacing();
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
            
            // Baud rate options
            static const std::pair<const char*, int> baud_rate_options[] = {
                {"9600", 9600},
                {"19200", 19200},
                {"38400", 38400},
                {"57600", 57600},
                {"115200", 115200},
                {"230400", 230400},
                {"460800", 460800},
                {"921600", 921600}
            };
            static int baud_rate_index = 4;  // Default to 115200
            
            ImGui::InputText("Serial Port", port_buffer, sizeof(port_buffer));
            
            // Baud rate dropdown
            if (ImGui::BeginCombo("Baud Rate", baud_rate_options[baud_rate_index]. first)) {
                for (int n = 0; n < IM_ARRAYSIZE(baud_rate_options); n++) {
                    const bool is_selected = (baud_rate_index == n);
                    if (ImGui::Selectable(baud_rate_options[n].first, is_selected)) {
                        baud_rate_index = n;
                    }
                    
                    if (is_selected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }
            
            if (ImGui::Button("Connect Serial")) {
                int baud_rate = baud_rate_options[baud_rate_index].second;
                log_info("Attempting to connect to serial port: {} at {} baud", port_buffer, baud_rate);
                conn.connectSerial(port_buffer, baud_rate);
            }
        } else {
            // Network connection
            static char ip_buffer[128] = "192.168.1.100";
            static int port = 3883;
            
            ImGui::InputText("IP Address", ip_buffer, sizeof(ip_buffer));
            ImGui::InputInt("Port", &port);
            
            if (ImGui::Button("Connect Network")) {
                log_info("Attempting to connect to network:  {}:{}", ip_buffer, port);
                conn.connectNetwork(ip_buffer, port);  // Use new API
            }
        }
    } else if (conn.isConnected()) {
        if (ImGui::Button("Disconnect")) {
            log_info("Disconnecting from device");
            conn.disconnect();  // Use new API
        }
    } else if (conn.isConnecting()) {
        ImGui::Text("Connecting...");
        if (ImGui::Button("Cancel")) {
            conn.disconnect();  // Use new API
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
            ImGui::Checkbox("Option 2", &option2);
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
