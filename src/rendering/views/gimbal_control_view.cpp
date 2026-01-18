#include "rendering/views/gimbal_control_view.hpp"
#include "core/connection_manager.hpp"
#include "util/logging.hpp"
#include "imgui.h"
#include "implot.h"
#include <utility>
#include <vector>
#include <cmath>

namespace Rendering {

using ConnType = ConnectionManager::ConnectionType;

GimbalControlView::GimbalControlView() {
    log_debug("GimbalControlView created");
}

void GimbalControlView::render() {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | 
                             ImGuiWindowFlags_NoMove | 
                             ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoBringToFrontOnFocus;
    
    ImGui::Begin("GimbalControlView", nullptr, flags);
    
    auto& conn = ConnectionManager::getInstance();
    ImVec2 avail = ImGui::GetContentRegionAvail();
    
    // Define layout proportions
    float left_panel_width = 350.0f;
    float right_width = avail.x - left_panel_width - 10.0f;
    
    float top_height = 180.0f;
    float middle_height = 200.0f;
    float bottom_height = avail.y - top_height - middle_height - 20.0f;
    
    // Move connection_type to function scope so it's accessible everywhere
    static ConnType connection_type = ConnType::Serial;
    
    // ═══════════════════════════════════════
    // TOP LEFT: Connection Panel
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
        
        bool is_connected_or_connecting = conn.isConnected() || conn.isConnecting();
        
        // Disable controls when connected
        ImGui::BeginDisabled(is_connected_or_connecting);
        {
            // Cast to int for ImGui radio buttons
            int connection_type_int = static_cast<int>(connection_type);
            
            if (ImGui::RadioButton("Serial", &connection_type_int, static_cast<int>(ConnType::Serial))) {
                connection_type = ConnType::Serial;
            }
            ImGui::SameLine();
            if (ImGui::RadioButton("Network", &connection_type_int, static_cast<int>(ConnType::Network))) {
                connection_type = ConnType::Network;
            }
            
            ImGui::Spacing();
            
            if (connection_type == ConnType::Serial) {
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
                    log_info("Connecting to serial: {} @ {}", port_buffer, baud_rate);
                    conn.connectSerial(port_buffer, baud_rate);
                }
                
                if (color_count > 0) {
                    ImGui::PopStyleColor(color_count);
                }
                
            } else if (connection_type == ConnType::Network) {
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
                    log_info("Connecting to network: {}:{}", ip_buffer, port);
                    conn.connectNetwork(ip_buffer, port);
                }
                
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
    // TOP MIDDLE: Gimbal Mode Control
    // ═══════════════════════════════════════
    float top_right_half = right_width / 2.0f - 5.0f;
    ImGui::BeginChild("ModePanel", ImVec2(top_right_half, top_height), true);
    {
        ImGui::SeparatorText("Gimbal Mode");
        
        ImGui::BeginDisabled(!conn.isConnected());
        {
            static int mode = 0;  // 0 = Disarmed, 1 = Armed
            
            // Mode status display
            ImGui::Text("Mode:");
            ImGui::SameLine();
            
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
    
    ImGui::SameLine();
    
    // ═══════════════════════════════════════
    // TOP RIGHT: Object Tracking
    // ═══════════════════════════════════════
    ImGui::BeginChild("TrackingPanel", ImVec2(top_right_half, top_height), true);
    {
        ImGui::SeparatorText("Object Tracking");
        
        static bool is_tracking = false;
        
        // Tracking status display
        ImGui::Text("Tracking:");
        ImGui::SameLine();
        
        if (is_tracking) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
            ImGui::Text("TRACKING");
            ImGui::PopStyleColor();
        } else {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
            ImGui::Text("NOT TRACKING");
            ImGui::PopStyleColor();
        }
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        // Only enable tracking controls when connected via network
        bool enable_tracking = conn.isConnected() && (connection_type == ConnType::Network);
        ImGui::BeginDisabled(!enable_tracking);
        {
            if (ImGui::Button("Enable Tracking", ImVec2(-1, 40))) {
                is_tracking = true;
                log_info("Object tracking enabled");
                // TODO: Send enable tracking command
            }
            
            if (ImGui::Button("Disable Tracking", ImVec2(-1, 40))) {
                is_tracking = false;
                log_info("Object tracking disabled");
                // TODO: Send disable tracking command
            }
        }
        ImGui::EndDisabled();
        
        if (!enable_tracking && conn.isConnected()) {
            ImGui::Spacing();
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Note: Tracking only available via network connection");
        }
    }
    ImGui::EndChild();
    
    // ═══════════════════════════════════════
    // MIDDLE LEFT: Streaming Control Panel
    // ═══════════════════════════════════════
    ImGui::BeginChild("StreamingPanel", ImVec2(left_panel_width, middle_height), true);
    {
        ImGui::SeparatorText("Streaming");
        
        // Track streaming state and previous connection state
        static bool is_streaming = false;
        static bool was_connected = false;
        static ConnType last_connection_type = ConnType::Serial;
        
        // Detect connection state changes
        bool currently_connected = conn.isConnected();
        
        // When connection state changes from disconnected to connected
        if (currently_connected && !was_connected) {
            // Set streaming based on connection type
            if (connection_type == ConnType::Serial) {
                is_streaming = true;
                log_info("Serial connected - streaming enabled by default");
            } else if (connection_type == ConnType::Network) {
                is_streaming = false;
                log_info("Network connected - streaming disabled by default");
            }
            last_connection_type = connection_type;
        }
        
        // When disconnected, reset streaming state
        if (!currently_connected && was_connected) {
            is_streaming = false;
            log_info("Disconnected - streaming reset");
        }
        
        was_connected = currently_connected;
        
        // Display streaming status
        ImGui::Text("Status:");
        ImGui::SameLine();
        
        if (!conn.isConnected()) {
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "N/A");
        } else if (is_streaming) {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Streaming");
        } else {
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Not Streaming");
        }
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        ImGui::BeginDisabled(!conn.isConnected());
        {
            // Stream rate options
            static const std::pair<const char*, int> stream_rate_options[] = {
                {"5 Hz", 5},
                {"10 Hz", 10},
                {"20 Hz", 20},
                {"30 Hz", 30},
                {"40 Hz", 40},
                {"50 Hz", 50}
            };
            static int stream_rate_index = 2;  // Default to 20 Hz
            
            ImGui::Text("Stream Rate:");
            
            if (ImGui::BeginCombo("##StreamRate", stream_rate_options[stream_rate_index].first)) {
                for (int n = 0; n < IM_ARRAYSIZE(stream_rate_options); n++) {
                    const bool is_selected = (stream_rate_index == n);
                    if (ImGui::Selectable(stream_rate_options[n].first, is_selected)) {
                        stream_rate_index = n;
                        if (is_streaming) {
                            int rate = stream_rate_options[stream_rate_index].second;
                            log_info("Stream rate changed to {} Hz", rate);
                            // TODO: Send stream rate change command to device
                        }
                    }
                    if (is_selected) ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
            
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
            
            // Start streaming button
            if (ImGui::Button("Start Streaming", ImVec2(-1, 0))) {
                if (!is_streaming) {
                    is_streaming = true;
                    int rate = stream_rate_options[stream_rate_index].second;
                    log_info("Started streaming at {} Hz", rate);
                    // TODO: Send start streaming command to device
                }
            }
            
            // Stop streaming button (disabled when not streaming)
            ImGui::BeginDisabled(!is_streaming);
            {
                if (ImGui::Button("Stop Streaming", ImVec2(-1, 0))) {
                    is_streaming = false;
                    log_info("Stopped streaming");
                    // TODO: Send stop streaming command to device
                }
            }
            ImGui::EndDisabled();
        }
        ImGui::EndDisabled();
    }
    ImGui::EndChild();
    
    ImGui::SameLine();
    
    // ═══════════════════════════════════════
    // MIDDLE RIGHT: Gimbal Position & Velocity Plots
    // ═══════════════════════════════════════
    float right_combined_height = middle_height + bottom_height + 10.0f;
    ImGui::BeginChild("PositionPanel", ImVec2(right_width, right_combined_height), true);
    {
        ImGui::SeparatorText("Gimbal Position & Velocity");
        
        // Data buffers for plotting (store last 500 points)
        static const int MAX_POINTS = 500;
        static std::vector<double> time_data;
        static std::vector<double> pan_pos_data;
        static std::vector<double> tilt_pos_data;
        static std::vector<double> pan_vel_data;
        static std::vector<double> tilt_vel_data;
        static double last_update_time = 0.0;
        
        // Mock position and velocity data (TODO: replace with actual telemetry)
        static float current_pan = 0.0f;
        static float current_tilt = 0.0f;
        static float pan_velocity = 0.0f;
        static float tilt_velocity = 0.0f;
        
        // Update plot data when connected (simulate 20Hz update rate)
        double current_time = ImGui::GetTime();
        if (conn.isConnected() && (current_time - last_update_time) >= 0.05) {
            // TODO: Replace with actual telemetry data from ConnectionManager
            // For now, generate mock sinusoidal data
            current_pan = 45.0f * std::sin(current_time * 0.5f);
            current_tilt = 30.0f * std::cos(current_time * 0.3f);
            pan_velocity = 22.5f * std::cos(current_time * 0.5f) * 0.5f;
            tilt_velocity = -9.0f * std::sin(current_time * 0.3f) * 0.3f;
            
            // Add to buffers
            time_data.push_back(current_time);
            pan_pos_data.push_back(current_pan);
            tilt_pos_data.push_back(current_tilt);
            pan_vel_data.push_back(pan_velocity);
            tilt_vel_data.push_back(tilt_velocity);
            
            // Keep buffer size limited
            if (time_data.size() > MAX_POINTS) {
                time_data.erase(time_data.begin());
                pan_pos_data.erase(pan_pos_data.begin());
                tilt_pos_data.erase(tilt_pos_data.begin());
                pan_vel_data.erase(pan_vel_data.begin());
                tilt_vel_data.erase(tilt_vel_data.begin());
            }
            
            last_update_time = current_time;
        }
        
        // Clear data when disconnected
        static bool was_connected_last_frame = false;
        if (!conn.isConnected() && was_connected_last_frame) {
            time_data.clear();
            pan_pos_data.clear();
            tilt_pos_data.clear();
            pan_vel_data.clear();
            tilt_vel_data.clear();
            current_pan = 0.0f;
            current_tilt = 0.0f;
            pan_velocity = 0.0f;
            tilt_velocity = 0.0f;
        }
        was_connected_last_frame = conn.isConnected();
        
        // Current values display
        ImGui::BeginDisabled(!conn.isConnected());
        {
            ImGui::Text("Pan Position:");
            ImGui::SameLine(150);
            ImGui::Text("%.2f°", current_pan);
            ImGui::SameLine(250);
            ImGui::Text("Pan Velocity:");
            ImGui::SameLine(380);
            ImGui::Text("%.2f°/s", pan_velocity);
            
            ImGui::Text("Tilt Position:");
            ImGui::SameLine(150);
            ImGui::Text("%.2f°", current_tilt);
            ImGui::SameLine(250);
            ImGui::Text("Tilt Velocity:");
            ImGui::SameLine(380);
            ImGui::Text("%.2f°/s", tilt_velocity);
        }
        ImGui::EndDisabled();
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        // Position plot
        int data_size = (int)time_data.size();
        if (data_size > 1) {
            if (ImPlot::BeginPlot("Position", ImVec2(-1, (right_combined_height - 150) / 2.0f))) {
                ImPlot::SetupAxes("Time (s)", "Position (deg)");
                ImPlot::SetupAxisLimits(ImAxis_X1, 
                    time_data[0], 
                    time_data[data_size - 1], 
                    ImGuiCond_Always);
                ImPlot::SetupAxisLimits(ImAxis_Y1, -90, 90, ImGuiCond_Once);
                
                ImPlot::PlotLine("Pan", time_data.data(), pan_pos_data.data(), data_size);
                ImPlot::PlotLine("Tilt", time_data.data(), tilt_pos_data.data(), data_size);
                
                ImPlot::EndPlot();
            }
            
            ImGui::Spacing();
            
            // Velocity plot
            if (ImPlot::BeginPlot("Velocity", ImVec2(-1, (right_combined_height - 150) / 2.0f))) {
                ImPlot::SetupAxes("Time (s)", "Velocity (deg/s)");
                ImPlot::SetupAxisLimits(ImAxis_X1, 
                    time_data[0], 
                    time_data[data_size - 1], 
                    ImGuiCond_Always);
                ImPlot::SetupAxisLimits(ImAxis_Y1, -50, 50, ImGuiCond_Once);
                
                ImPlot::PlotLine("Pan Vel", time_data.data(), pan_vel_data.data(), data_size);
                ImPlot::PlotLine("Tilt Vel", time_data.data(), tilt_vel_data.data(), data_size);
                
                ImPlot::EndPlot();
            }
        } else {
            ImGui::TextDisabled("No data to plot - connect and start streaming");
        }
    }
    ImGui::EndChild();
    
    // ═══════════════════════════════════════
    // BOTTOM LEFT: Manual Control
    // ═══════════════════════════════════════
    // Set cursor position to be below the Streaming Panel on the left
    float window_padding_x = ImGui::GetStyle().WindowPadding.x;
    ImGui::SetCursorPos(ImVec2(window_padding_x, top_height + 10.0f + middle_height + 10.0f));
    
    ImGui::BeginChild("ControlPanel", ImVec2(left_panel_width, bottom_height), true);
    {
        ImGui::SeparatorText("Manual Control");
        
        ImGui::BeginDisabled(!conn.isConnected());
        {
            static char target_pan_buffer[32] = "0.0";
            static char target_tilt_buffer[32] = "0.0";
            
            ImGui::InputText("Target Pan (°)", target_pan_buffer, sizeof(target_pan_buffer));
            ImGui::InputText("Target Tilt (°)", target_tilt_buffer, sizeof(target_tilt_buffer));
            
            ImGui::Spacing();
            
            if (ImGui::Button("Demand Position", ImVec2(-1, 0))) {
                float pan = std::atof(target_pan_buffer);
                float tilt = std::atof(target_tilt_buffer);
                log_info("Demanding position: pan={}, tilt={}", pan, tilt);
                // TODO: Send move command
            }
            
            ImGui::Spacing();
            
            if (ImGui::Button("Reset", ImVec2(-1, 0))) {
                strcpy(target_pan_buffer, "0.0");
                strcpy(target_tilt_buffer, "0.0");
                log_info("Reset target position to 0, 0");
            }
        }
        ImGui::EndDisabled();
    }
    ImGui::EndChild();
    
    ImGui::End();
}

} // namespace Rendering
