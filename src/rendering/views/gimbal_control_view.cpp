#include "rendering/views/gimbal_control_view.hpp"
#include "core/communication_backend.hpp"
#include "core/gimbal_state.hpp"
#include "util/logging.hpp"
#include "platform/serial_port_helper.hpp"
#include "imgui.h"
#include "implot.h"
#include <utility>
#include <vector>
#include <cmath>
#include <cstring>

namespace Rendering {

using TransType = CommunicationBackend::TransportType;

GimbalControlView::GimbalControlView(GimbalState& gimbal_state, CommunicationBackend& comm_backend)
    : gimbal_state_(gimbal_state)
    , comm_backend_(comm_backend) {
    log_debug("GimbalControlView created");
    refreshSerialPorts();
}

void GimbalControlView::refreshSerialPorts() {
    available_serial_ports_ = SerialPortHelper::getAvailablePorts();
    
    if (available_serial_ports_.empty()) {
        log_warn("No serial ports detected");
        available_serial_ports_.push_back("No ports found");
    }
    
    selected_port_index_ = 0;
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
    
    // Use references instead of getInstance()
    auto& comm = comm_backend_;
    auto& gimbal = gimbal_state_;
    
    ImVec2 avail = ImGui::GetContentRegionAvail();
    
    // Define layout proportions
    float left_panel_width = 350.0f;
    float right_width = avail.x - left_panel_width - 10.0f;
    
    float top_height = 180.0f;
    float middle_height = 200.0f;
    float bottom_height = avail.y - top_height - middle_height - 20.0f;
    
    static TransType connection_type = TransType::Serial;
    
    // ═══════════════════════════════════════
    // TOP LEFT: Connection Panel
    // ═══════════════════════════════════════
    ImGui::BeginChild("ConnectionPanel", ImVec2(left_panel_width, top_height), true);
    {
        ImGui::SeparatorText("Connection");

        ImGui::Text("Status:");
        ImGui::SameLine();
        
        if (comm.isConnected()) {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Connected");
            ImGui::SameLine();
            ImGui::Text("to %s", comm.getConnectionInfo().c_str());
        } else {
            auto transport_type = comm.getTransportType();
            if (transport_type != TransType::None) {
                ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Connecting...");
            } else {
                ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Disconnected");
            }
            
            auto error = comm.getErrorMessage();
            if (!error.empty()) {
                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Error:");
                ImGui::SameLine();
                ImGui::TextWrapped("%s", error.c_str());
            }
        }
        
        ImGui::Spacing();
        
        bool is_connected = comm.isConnected();
        
        ImGui::BeginDisabled(is_connected);
        {
            int connection_type_int = static_cast<int>(connection_type);
            
            if (ImGui::RadioButton("Serial", &connection_type_int, static_cast<int>(TransType::Serial))) {
                connection_type = TransType::Serial;
            }
            ImGui::SameLine();
            if (ImGui::RadioButton("Network", &connection_type_int, static_cast<int>(TransType::Network))) {
                connection_type = TransType::Network;
            }
            
            ImGui::Spacing();
            
            if (connection_type == TransType::Serial) {
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
                
                float combo_width = ImGui::GetContentRegionAvail().x;
                float label_width = 80.0f;
                float refresh_button_width = 55.0f;
                float spacing = ImGui::GetStyle().ItemSpacing.x;

                // Port dropdown + refresh button
                ImGui::SetNextItemWidth(combo_width - refresh_button_width - label_width - spacing * 2);
                const char* current_port = available_serial_ports_[selected_port_index_].c_str();
                if (ImGui::BeginCombo("##PortCombo", current_port)) {
                    for (size_t i = 0; i < available_serial_ports_.size(); ++i) {
                        bool is_selected = (selected_port_index_ == i);
                        if (ImGui::Selectable(available_serial_ports_[i].c_str(), is_selected)) {
                            selected_port_index_ = i;
                        }
                        if (is_selected) {
                            ImGui::SetItemDefaultFocus();
                        }
                    }
                    ImGui::EndCombo();
                }

                ImGui::SameLine();
                if (ImGui::Button("Refresh", ImVec2(refresh_button_width, 0))) {
                    log_info("Refreshing serial port list");
                    refreshSerialPorts();
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Refresh serial port list");
                }

                ImGui::SameLine();
                ImGui::Text("Port");

                // Baud rate dropdown
                ImGui::SetNextItemWidth(combo_width - label_width - spacing);
                if (ImGui::BeginCombo("##BaudRate", baud_rate_options[baud_rate_index].first)) {
                    for (int n = 0; n < IM_ARRAYSIZE(baud_rate_options); n++) {
                        const bool is_selected = (baud_rate_index == n);
                        if (ImGui::Selectable(baud_rate_options[n].first, is_selected)) {
                            baud_rate_index = n;
                        }
                        if (is_selected) ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }

                ImGui::SameLine();
                ImGui::Text("Baud Rate");
                
                if (ImGui::Button("Connect Serial", ImVec2(-1, 0))) {
                    const std::string& selected_port = available_serial_ports_[selected_port_index_];
                    
                    if (selected_port == "No ports found") {
                        log_error("No serial ports available");
                    } else {
                        int baud_rate = baud_rate_options[baud_rate_index].second;
                        log_info("Connecting to serial: {} @ {}", selected_port, baud_rate);
                        comm.connectSerial(selected_port, baud_rate);
                    }
                }
                
            } else if (connection_type == TransType::Network) {
                static char ip_buffer[128] = "192.168.1.100";
                static int port = 3883;
                
                ImGui::InputText("IP Address", ip_buffer, sizeof(ip_buffer));
                ImGui::InputInt("Port", &port);
                
                if (ImGui::Button("Connect Network", ImVec2(-1, 0))) {
                    log_info("Connecting to network: {}:{}", ip_buffer, port);
                    comm.connectNetwork(ip_buffer, static_cast<uint16_t>(port));
                }
            }
        }
        ImGui::EndDisabled();
        
        if (is_connected) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.0f, 0.0f, 0.6f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.0f, 0.0f, 0.8f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.6f, 0.0f, 0.0f, 1.0f));
            
            if (ImGui::Button("Disconnect", ImVec2(-1, 0))) {
                log_info("Disconnecting");
                comm.disconnect();
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
        
        ImGui::BeginDisabled(!comm.isConnected());
        {
            auto mode = gimbal.getMode();
            
            ImGui::Text("Mode:");
            ImGui::SameLine();
            
            switch (mode) {
                case GimbalState::Mode::Free:
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
                    ImGui::Text("FREE");
                    ImGui::PopStyleColor();
                    break;
                case GimbalState::Mode::Armed:
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
                    ImGui::Text("ARMED");
                    ImGui::PopStyleColor();
                    break;
                case GimbalState::Mode::LowerLimit:
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.3f, 0.0f, 1.0f));
                    ImGui::Text("LOWER LIMIT");
                    ImGui::PopStyleColor();
                    break;
                case GimbalState::Mode::UpperLimit:
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.3f, 0.0f, 1.0f));
                    ImGui::Text("UPPER LIMIT");
                    ImGui::PopStyleColor();
                    break;
            }
            
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
            
            if (ImGui::Button("ARM", ImVec2(-1, 40))) {
                log_info("Sending ARM command");
                // TODO: Create protobuf ARM message
                // std::vector<uint8_t> cmd_data = createArmCommand();
                // comm.sendMessage(cmd_data, [](bool success) {
                //     log_info("ARM command {}", success ? "ACKed" : "timeout");
                // });
            }
            
            if (ImGui::Button("DISARM (FREE)", ImVec2(-1, 40))) {
                log_info("Sending DISARM command");
                // TODO: Create protobuf DISARM message
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
        
        bool enable_tracking = comm.isConnected() && (connection_type == TransType::Network);
        ImGui::BeginDisabled(!enable_tracking);
        {
            if (ImGui::Button("Enable Tracking", ImVec2(-1, 40))) {
                is_tracking = true;
                log_info("Object tracking enabled");
                // TODO: Send tracking command
            }
            
            if (ImGui::Button("Disable Tracking", ImVec2(-1, 40))) {
                is_tracking = false;
                log_info("Object tracking disabled");
                // TODO: Send tracking command
            }
        }
        ImGui::EndDisabled();
        
        if (!enable_tracking && comm.isConnected()) {
            ImGui::Spacing();
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Note: Tracking only available via network");
        }
    }
    ImGui::EndChild();
    
    // ═══════════════════════════════════════
    // MIDDLE LEFT: Streaming Control
    // ═══════════════════════════════════════
    ImGui::BeginChild("StreamingPanel", ImVec2(left_panel_width, middle_height), true);
    {
        ImGui::SeparatorText("Streaming");
        
        static bool is_streaming = false;
        static bool was_connected = false;
        
        bool currently_connected = comm.isConnected();
        
        // Auto-enable streaming on serial connection
        if (currently_connected && !was_connected) {
            if (connection_type == TransType::Serial) {
                is_streaming = true;
                log_info("Serial connected - streaming enabled by default");
            } else {
                is_streaming = false;
            }
        }
        
        // Auto-disable on disconnect
        if (!currently_connected && was_connected) {
            is_streaming = false;
        }
        
        was_connected = currently_connected;
        
        ImGui::Text("Status:");
        ImGui::SameLine();
        
        if (!comm.isConnected()) {
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "N/A");
        } else if (is_streaming) {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Streaming");
        } else {
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Not Streaming");
        }
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        ImGui::BeginDisabled(!comm.isConnected());
        {
            static const std::pair<const char*, int> stream_rate_options[] = {
                {"5 Hz", 5}, {"10 Hz", 10}, {"20 Hz", 20},
                {"30 Hz", 30}, {"40 Hz", 40}, {"50 Hz", 50}
            };
            static int stream_rate_index = 2;
            
            ImGui::Text("Stream Rate:");
            
            if (ImGui::BeginCombo("##StreamRate", stream_rate_options[stream_rate_index].first)) {
                for (int n = 0; n < IM_ARRAYSIZE(stream_rate_options); n++) {
                    const bool is_selected = (stream_rate_index == n);
                    if (ImGui::Selectable(stream_rate_options[n].first, is_selected)) {
                        stream_rate_index = n;
                        if (is_streaming) {
                            int rate = stream_rate_options[stream_rate_index].second;
                            log_info("Stream rate changed to {} Hz", rate);
                            // TODO: Send rate change command
                        }
                    }
                    if (is_selected) ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
            
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
            
            if (ImGui::Button("Start Streaming", ImVec2(-1, 0))) {
                if (!is_streaming) {
                    is_streaming = true;
                    int rate = stream_rate_options[stream_rate_index].second;
                    log_info("Started streaming at {} Hz", rate);
                    // TODO: Send start streaming command
                }
            }
            
            ImGui::BeginDisabled(!is_streaming);
            {
                if (ImGui::Button("Stop Streaming", ImVec2(-1, 0))) {
                    is_streaming = false;
                    log_info("Stopped streaming");
                    // TODO: Send stop streaming command
                }
            }
            ImGui::EndDisabled();
        }
        ImGui::EndDisabled();
    }
    ImGui::EndChild();
    
    ImGui::SameLine();
    
    // ═══════════════════════════════════════
    // MIDDLE & BOTTOM RIGHT: Position Plot (NO VELOCITY)
    // ═══════════════════════════════════════
    float right_combined_height = middle_height + bottom_height + 10.0f;
    ImGui::BeginChild("PositionPanel", ImVec2(right_width, right_combined_height), true);
    {
        ImGui::SeparatorText("Gimbal Position");
        
        static const int MAX_POINTS = 500;
        static std::vector<double> time_data;
        static std::vector<double> pan_pos_data;
        static std::vector<double> pan_setpoint_data;
        static double last_update_time = 0.0;
        
        // Get real data from GimbalState
        auto position = gimbal.getPosition();
        auto setpoint = gimbal.getSetpoint();
        
        // Update plot data when connected (20Hz)
        double current_time = ImGui::GetTime();
        if (comm.isConnected() && !gimbal.isStale() && (current_time - last_update_time) >= 0.05) {
            time_data.push_back(current_time);
            pan_pos_data.push_back(position.pan_deg);
            pan_setpoint_data.push_back(setpoint.pan_deg);
            
            if (time_data.size() > MAX_POINTS) {
                time_data.erase(time_data.begin());
                pan_pos_data.erase(pan_pos_data.begin());
                pan_setpoint_data.erase(pan_setpoint_data.begin());
            }
            
            last_update_time = current_time;
        }
        
        // Clear data when disconnected
        static bool was_connected_last_frame = false;
        if (!comm.isConnected() && was_connected_last_frame) {
            time_data.clear();
            pan_pos_data.clear();
            pan_setpoint_data.clear();
        }
        was_connected_last_frame = comm.isConnected();
        
        // Display real values
        ImGui::BeginDisabled(!comm.isConnected());
        {
            ImGui::Text("Pan Position:");
            ImGui::SameLine(150);
            ImGui::Text("%.2f°", position.pan_deg);
            
            ImGui::Text("Pan Setpoint:");
            ImGui::SameLine(150);
            ImGui::Text("%.2f°", setpoint.pan_deg);
            
            ImGui::Text("Error:");
            ImGui::SameLine(150);
            ImGui::Text("%.2f°", setpoint.pan_deg - position.pan_deg);
        }
        ImGui::EndDisabled();
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        // Plot
        int data_size = static_cast<int>(time_data.size());
        if (data_size > 1) {
            if (ImPlot::BeginPlot("Pan Position", ImVec2(-1, right_combined_height - 120))) {
                ImPlot::SetupAxes("Time (s)", "Position (deg)");
                ImPlot::SetupAxisLimits(ImAxis_X1, time_data[0], time_data[data_size - 1], ImGuiCond_Always);
                ImPlot::SetupAxisLimits(ImAxis_Y1, -180, 180, ImGuiCond_Once);
                
                ImPlot::PlotLine("Position", time_data.data(), pan_pos_data.data(), data_size);
                ImPlot::PlotLine("Setpoint", time_data.data(), pan_setpoint_data.data(), data_size);
                
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
    float window_padding_x = ImGui::GetStyle().WindowPadding.x;
    ImGui::SetCursorPos(ImVec2(window_padding_x, top_height + 10.0f + middle_height + 10.0f));
    
    ImGui::BeginChild("ControlPanel", ImVec2(left_panel_width, bottom_height), true);
    {
        ImGui::SeparatorText("Manual Control");
        
        ImGui::BeginDisabled(!comm.isConnected());
        {
            static char target_pan_buffer[32] = "0.0";
            
            ImGui::InputText("Target Pan (°)", target_pan_buffer, sizeof(target_pan_buffer));
            
            ImGui::Spacing();
            
            if (ImGui::Button("Demand Position", ImVec2(-1, 0))) {
                float pan = std::atof(target_pan_buffer);
                log_info("Demanding position: pan={}", pan);
                // TODO: Create protobuf position command
                // std::vector<uint8_t> cmd = createPositionCommand(pan);
                // comm.sendMessage(cmd, [](bool success) {
                //     log_info("Position command {}", success ? "ACKed" : "timeout");
                // });
            }
            
            ImGui::Spacing();
            
            if (ImGui::Button("Go to 0°", ImVec2(-1, 0))) {
                std::strcpy(target_pan_buffer, "0.0");
            }
            
            if (ImGui::Button("Go to 90°", ImVec2(-1, 0))) {
                std::strcpy(target_pan_buffer, "90.0");
            }
            
            if (ImGui::Button("Go to -90°", ImVec2(-1, 0))) {
                std::strcpy(target_pan_buffer, "-90.0");
            }
        }
        ImGui::EndDisabled();
    }
    ImGui::EndChild();
    
    ImGui::End();
}

} // namespace Rendering
