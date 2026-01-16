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
    ImVec2 avail = ImGui::GetContentRegionAvail();
    
    // Define layout proportions
    float left_panel_width = 350.0f;  // Fixed width for connection panel
    float right_width = avail.x - left_panel_width - 10.0f;
    
    float top_height = 250.0f;  // Height for top row
    float middle_height = 200.0f;  // Height for middle row
    float bottom_height = avail.y - top_height - middle_height - 20.0f;
    
    // ═══════════════════════════════════════
    // TOP LEFT:  Connection Panel
    // ═══════════════════════════════════════
    ImGui::BeginChild("ConnectionPanel", ImVec2(left_panel_width, top_height), true);
    {
        ImGui::SeparatorText("Connection");

        ImGui::Text("Status:");
        ImGui::SameLine();
        
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
        
        static int connection_type = 0;  // 0 = Serial, 1 = Network
        
        bool is_connected_or_connecting = conn.isConnected() || conn.isConnecting();
        
        // Disable controls when connected
        ImGui::BeginDisabled(is_connected_or_connecting);
        {
            ImGui::RadioButton("Serial", &connection_type, 0);
            ImGui::SameLine();
            ImGui::RadioButton("Network", &connection_type, 1);
            
            ImGui::Spacing();
            
            if (connection_type == 0) {
                // Serial connection
                static char port_buffer[128] = "/dev/ttyUSB0";
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
                static int baud_rate_index = 4;
                
                ImGui::InputText("Port", port_buffer, sizeof(port_buffer));
                
                if (ImGui::BeginCombo("Baud Rate", baud_rate_options[baud_rate_index].first)) {
                    for (int n = 0; n < IM_ARRAYSIZE(baud_rate_options); n++) {
                        const bool is_selected = (baud_rate_index == n);
                        if (ImGui::Selectable(baud_rate_options[n].first, is_selected)) {
                            baud_rate_index = n;
                        }
                        if (is_selected) ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }
                
                // Push green color if connected
                int color_count = 0;
                if (conn.isConnected()) {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.6f, 0.0f, 0.6f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 0.6f, 0.0f, 0.6f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 0.6f, 0.0f, 0.6f));
                    color_count = 3;
                }
                
                if (ImGui::Button("Connect Serial", ImVec2(-1, 0))) {
                    int baud_rate = baud_rate_options[baud_rate_index].second;
                    log_info("Connecting to serial:  {} @ {}", port_buffer, baud_rate);
                    conn.connectSerial(port_buffer, baud_rate);
                }
                
                // Pop green color if we pushed it
                if (color_count > 0) {
                    ImGui::PopStyleColor(color_count);
                }
                
            } else {
                // Network connection
                static char ip_buffer[128] = "192.168.1.100";
                static int port = 3883;
                
                ImGui::InputText("IP Address", ip_buffer, sizeof(ip_buffer));
                ImGui::InputInt("Port", &port);
                
                // Push green color if connected
                int color_count = 0;
                if (conn.isConnected()) {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.6f, 0.0f, 0.6f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 0.6f, 0.0f, 0.6f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 0.6f, 0.0f, 0.6f));
                    color_count = 3;
                }
                
                if (ImGui::Button("Connect Network", ImVec2(-1, 0))) {
                    log_info("Connecting to network:  {}:{}", ip_buffer, port);
                    conn.connectNetwork(ip_buffer, port);
                }
                
                // Pop green color if we pushed it
                if (color_count > 0) {
                    ImGui::PopStyleColor(color_count);
                }
            }
        }
        ImGui::EndDisabled();
        
        // Red disconnect button (only shown when connected)
        if (is_connected_or_connecting) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.0f, 0.0f, 0.6f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.0f, 0.0f, 0.8f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.6f, 0.0f, 0.0f, 1.0f));
            
            if (ImGui::Button("Disconnect", ImVec2(-1, 0))) {
                log_info("Disconnecting");
                conn.disconnect();
            }
            
            ImGui::PopStyleColor(3);
        }
    }
    ImGui::EndChild();
    
    ImGui::SameLine();
    
    // ═══════════════════════════════════════
    // TOP RIGHT:  Gimbal Mode Control
    // ═══════════════════════════════════════
    ImGui::BeginChild("ModePanel", ImVec2(right_width, top_height), true);
    {
        ImGui::SeparatorText("Gimbal Mode");
        
        ImGui::BeginDisabled(! conn.isConnected());
        {
            static int mode = 0;  // 0 = Disarmed, 1 = Armed
            
            // Big status display
            if (mode == 0) {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
                ImGui::Text("DISARMED");
                ImGui::PopStyleColor();
            } else {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.3f, 0.0f, 1.0f));
                ImGui::Text("ARMED");
                ImGui::PopStyleColor();
            }
            
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
            
            // Mode buttons
            if (ImGui::Button("ARM", ImVec2(-1, 40))) {
                mode = 1;
                log_info("Gimbal ARMED");
                // TODO: Send ARM command to device
            }
            
            if (ImGui::Button("DISARM", ImVec2(-1, 40))) {
                mode = 0;
                log_info("Gimbal DISARMED");
                // TODO: Send DISARM command to device
            }
        }
        ImGui::EndDisabled();
    }
    ImGui::EndChild();
    
    // ═══════════════════════════════════════
    // MIDDLE LEFT: Axis Position Display
    // ═══════════════════════════════════════
    ImGui::BeginChild("AxisPanel", ImVec2(left_panel_width, middle_height), true);
    {
        ImGui::SeparatorText("Axis Positions");
        
        ImGui::BeginDisabled(!conn.isConnected());
        {
            static float pan_angle = 0.0f;
            static float tilt_angle = 0.0f;
            
            // Display current positions
            ImGui::Text("Pan:   %.2f°", pan_angle);
            ImGui::ProgressBar((pan_angle + 180.0f) / 360.0f, ImVec2(-1, 0));
            
            ImGui::Spacing();
            
            ImGui::Text("Tilt: %.2f°", tilt_angle);
            ImGui::ProgressBar((tilt_angle + 90.0f) / 180.0f, ImVec2(-1, 0));
            
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
            
            // Start/Stop streaming
            static bool streaming = false;
            if (streaming) {
                if (ImGui::Button("Stop Streaming", ImVec2(-1, 0))) {
                    streaming = false;
                    log_info("Stopped axis streaming");
                }
                
                // TODO: Update pan_angle and tilt_angle from device
            } else {
                if (ImGui::Button("Start Streaming", ImVec2(-1, 0))) {
                    streaming = true;
                    log_info("Started axis streaming");
                }
            }
        }
        ImGui::EndDisabled();
    }
    ImGui::EndChild();
    
    ImGui::SameLine();
    
    // ═══════════════════════════════════════
    // MIDDLE RIGHT: Manual Control
    // ═══════════════════════════════════════
    ImGui::BeginChild("ControlPanel", ImVec2(right_width, middle_height), true);
    {
        ImGui::SeparatorText("Manual Control");
        
        ImGui::BeginDisabled(!conn.isConnected());
        {
            static float target_pan = 0.0f;
            static float target_tilt = 0.0f;
            
            ImGui::SliderFloat("Target Pan", &target_pan, -180.0f, 180.0f, "%.1f°");
            ImGui::SliderFloat("Target Tilt", &target_tilt, -90.0f, 90.0f, "%.1f°");
            
            ImGui::Spacing();
            
            if (ImGui::Button("Move to Target", ImVec2(-1, 0))) {
                log_info("Moving to pan={}, tilt={}", target_pan, target_tilt);
                // TODO: Send move command
            }
            
            ImGui::Spacing();
            
            if (ImGui::Button("Center (0, 0)", ImVec2(-1, 0))) {
                target_pan = 0.0f;
                target_tilt = 0.0f;
                log_info("Centering gimbal");
                // TODO: Send center command
            }
        }
        ImGui::EndDisabled();
    }
    ImGui::EndChild();
    
    // ═══════════════════════════════════════
    // BOTTOM:  Diagnostics (Full Width)
    // ═══════════════════════════════════════
    ImGui::BeginChild("DiagnosticsPanel", ImVec2(avail.x, bottom_height), true);
    {
        ImGui::SeparatorText("Diagnostics");
        
        ImGui::BeginDisabled(!conn.isConnected());
        {
            ImGui::Text("Device Temperature: 45.2°C");
            ImGui::Text("Motor Current (Pan): 0.32A");
            ImGui::Text("Motor Current (Tilt): 0.28A");
            ImGui::Text("IMU Status: OK");
            ImGui::Text("Encoder Status: OK");
            
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
            
            if (ImGui::Button("Calibrate IMU")) {
                log_info("Starting IMU calibration");
                // TODO: Send calibration command
            }
            
            ImGui::SameLine();
            
            if (ImGui::Button("Reset Errors")) {
                log_info("Resetting errors");
                // TODO: Send reset command
            }
        }
        ImGui::EndDisabled();
        
        ImGui::Spacing();
        ImGui::Separator();
        
        // Footer
        ImGui::Text("Application: %.3f ms/frame (%.1f FPS)", 
                    1000.0f / ImGui::GetIO().Framerate, 
                    ImGui::GetIO().Framerate);
    }
    ImGui::EndChild();
    
    ImGui::End();
}

} // namespace Rendering
