#include "core/communication_backend.hpp"
#include "core/gimbal_state.hpp"
#include "util/logging.hpp"

// TODO: Include your protobuf headers
// #include "gimbal_messages.pb.h"

CommunicationBackend::CommunicationBackend() {
    log_debug("CommunicationBackend created");
}

CommunicationBackend::~CommunicationBackend() {
    disconnect();
}

bool CommunicationBackend::connect(std::unique_ptr<ITransport> transport) {
    if (transport_) {
        log_warn("Already connected, disconnecting first");
        disconnect();
    }
    
    if (!transport->open()) {
        log_error("Failed to open transport");
        return false;
    }
    
    transport_ = std::move(transport);
    
    // Create ACK manager using transport's io_context
    ack_manager_ = std::make_unique<PacketAckManager>(transport_->getIoContext());
    
    // Set packet received callback
    transport_->setPacketReceivedCallback([this](const std::vector<uint8_t>& data) {
        handleReceivedPacket(data);
    });
    
    log_info("Communication backend connected:  {}", transport_->getConnectionInfo());
    return true;
}

void CommunicationBackend::disconnect() {
    if (!transport_) {
        return;
    }
    
    log_info("Disconnecting communication backend");
    
    // Cancel all pending ACKs FIRST (before closing transport)
    if (ack_manager_) {
        ack_manager_->cancelAll();
    }
    
    // Close transport (stops I/O thread)
    if (transport_) {
        transport_->close();
        transport_.reset();  // ✅ Explicitly destroy transport
    }
    
    // Reset ACK manager
    ack_manager_.reset();  // ✅ Explicitly destroy ACK manager
    
    // Reset gimbal state
    GimbalState::getInstance().reset();
    
    log_info("Communication backend disconnected");
}

bool CommunicationBackend::isConnected() const {
    return transport_ && transport_->isOpen();
}

std::string CommunicationBackend::getConnectionInfo() const {
    if (transport_) {
        return transport_->getConnectionInfo();
    }
    return "Not connected";
}

void CommunicationBackend::sendMessage(const std::vector<uint8_t>& protobuf_data,
                                      std::function<void(bool)> ack_callback,
                                      uint32_t timeout_ms) {
    if (!isConnected()) {
        log_warn("Cannot send message: not connected");
        if (ack_callback) {
            ack_callback(false);
        }
        return;
    }
    
    // Get unique packet ID
    uint32_t packet_id = ack_manager_->getNextPacketId();
    
    // TODO: Embed packet_id into protobuf message header
    // For now, assume it's already in protobuf_data
    
    // Encode with varint length prefix
    std::vector<uint8_t> encoded_packet = codec_.encode(protobuf_data);
    
    log_debug("Sending message: packet_id={}, size={}", packet_id, encoded_packet.size());
    
    // Register for ACK if callback provided
    if (ack_callback) {
        ack_manager_->registerPacket(packet_id, 
            [ack_callback](bool success, uint32_t id) {
                ack_callback(success);
            }, 
            timeout_ms);
    }
    
    // Send via transport
    transport_->writeAsync(encoded_packet);
}

void CommunicationBackend::handleReceivedPacket(const std::vector<uint8_t>& packet_data) {
    log_debug("Received packet: {} bytes", packet_data.size());
    decodeAndProcessMessage(packet_data);
}

void CommunicationBackend::decodeAndProcessMessage(const std::vector<uint8_t>& data) {
    // TODO: Replace with actual protobuf parsing
    // This runs on the transport's I/O thread
    
    /*
    GimbalMessage msg;
    if (! msg.ParseFromArray(data.data(), data.size())) {
        log_error("Failed to parse protobuf message");
        return;
    }
    
    if (msg.has_telemetry()) {
        auto& telem = msg.telemetry();
        
        GimbalState::Attitude att;
        att.pan_deg = telem.pan_deg();
        att.tilt_deg = telem.tilt_deg();
        att.roll_deg = telem.roll_deg();
        
        GimbalState::Velocity vel;
        vel.pan_deg_s = telem.pan_vel();
        vel.tilt_deg_s = telem.tilt_vel();
        vel.roll_deg_s = telem.roll_vel();
        
        GimbalState::Mode mode;
        mode.type = static_cast<GimbalState::Mode::Type>(telem.mode());
        mode.is_enabled = telem.enabled();
        
        GimbalState::getInstance().updateAll(att, vel, mode);
        
    } else if (msg.has_ack()) {
        uint32_t packet_id = msg.ack().packet_id();
        ack_manager_->handleAck(packet_id);
        
    } else if (msg.has_error()) {
        GimbalState::Health health;
        health.status = GimbalState::Health::Status::Error;
        health.message = msg.error().message();
        health.error_flags = msg.error().flags();
        
        GimbalState::getInstance().setHealth(health);
    }
    */
    
    log_debug("Message processed");
}
