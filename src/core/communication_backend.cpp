#include "core/communication_backend.hpp"
#include "util/logging.hpp"

CommunicationBackend::CommunicationBackend(GimbalState& gimbal_state)
    : gimbal_state_(gimbal_state)
    , transport_(std::monostate{}) {  // Start with no transport
    log_debug("CommunicationBackend created");
}

CommunicationBackend::~CommunicationBackend() {
    disconnect();
}

ITransport* CommunicationBackend::getActiveTransport() {
    return std::visit([](auto&& arg) -> ITransport* {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, std::monostate>) {
            return nullptr;
        } else {
            return arg.get();
        }
    }, transport_);
}

const ITransport* CommunicationBackend::getActiveTransport() const {
    return std::visit([](auto&& arg) -> const ITransport* {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, std::monostate>) {
            return nullptr;
        } else {
            return arg.get();
        }
    }, transport_);
}

CommunicationBackend::TransportType CommunicationBackend::getTransportType() const {
    return std::visit([](auto&& arg) -> TransportType {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, std::monostate>) {
            return TransportType::None;
        } else if constexpr (std::is_same_v<T, std::unique_ptr<SerialTransport>>) {
            return TransportType::Serial;
        } else if constexpr (std::is_same_v<T, std::unique_ptr<NetworkTransport>>) {
            return TransportType::Network;
        }
        return TransportType::None;
    }, transport_);
}

bool CommunicationBackend::connectSerial(const std::string& port, uint32_t baud_rate) {
    disconnect();  // Ensure clean state
    
    log_info("Connecting to serial: {} @ {} baud", port, baud_rate);
    
    try {
        auto serial = std::make_unique<SerialTransport>(port, baud_rate);
        
        if (!serial->open()) {
            error_message_ = "Failed to open serial port";
            log_error("{}", error_message_);
            return false;
        }
        
        // Create ACK manager
        ack_manager_ = std::make_unique<PacketAckManager>(serial->getIoContext());
        
        // Set callback (runs on serial I/O thread!)
        serial->setPacketReceivedCallback([this](const std::vector<uint8_t>& data) {
            handleReceivedPacket(data);
        });
        
        // Move into variant
        transport_ = std::move(serial);
        error_message_.clear();
        
        log_info("Serial connected successfully");
        return true;
        
    } catch (const std::exception& e) {
        error_message_ = std::string("Exception: ") + e.what();
        log_error("Serial connection failed: {}", e.what());
        return false;
    }
}

bool CommunicationBackend::connectNetwork(const std::string& host, uint16_t port) {
    disconnect();
    
    log_info("Connecting to network: {}:{}", host, port);
    
    try {
        auto network = std::make_unique<NetworkTransport>(host, port);
        
        if (!network->open()) {
            error_message_ = "Failed to connect to network";
            log_error("{}", error_message_);
            return false;
        }
        
        ack_manager_ = std::make_unique<PacketAckManager>(network->getIoContext());
        
        network->setPacketReceivedCallback([this](const std::vector<uint8_t>& data) {
            handleReceivedPacket(data);
        });
        
        transport_ = std::move(network);
        error_message_.clear();
        
        log_info("Network connected successfully");
        return true;
        
    } catch (const std::exception& e) {
        error_message_ = std::string("Exception: ") + e.what();
        log_error("Network connection failed: {}", e.what());
        return false;
    }
}

void CommunicationBackend::disconnect() {
    if (!isConnected()) {
        return;
    }
    
    log_info("Disconnecting communication backend");
    
    if (ack_manager_) {
        ack_manager_->cancelAll();
        ack_manager_.reset();
    }
    
    if (auto* transport = getActiveTransport()) {
        transport->close();
    }
    
    transport_ = std::monostate{};  // Clear variant
    gimbal_state_.reset();
    
    log_info("Communication backend disconnected");
}

bool CommunicationBackend::isConnected() const {
    auto* transport = getActiveTransport();
    return transport && transport->isOpen();
}

std::string CommunicationBackend::getConnectionInfo() const {
    if (auto* transport = getActiveTransport()) {
        return transport->getConnectionInfo();
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
    
    // Validate payload size (max 62 bytes to fit in 64-byte packet)
    if (protobuf_data.size() > PacketCodec::MAX_PAYLOAD_SIZE) {
        log_error("Payload too large: {} bytes (max {})", 
                  protobuf_data.size(), PacketCodec::MAX_PAYLOAD_SIZE);
        if (ack_callback) {
            ack_callback(false);
        }
        return;
    }
    
    uint32_t packet_id = ack_manager_->getNextPacketId();
    
    // Encode: [0xAA] [varint length] [payload]
    // For payloads ≤62 bytes, varint is always 1 byte
    std::vector<uint8_t> encoded_packet;
    try {
        encoded_packet = codec_.encode(protobuf_data);
    } catch (const std::exception& e) {
        log_error("Failed to encode packet: {}", e.what());
        if (ack_callback) {
            ack_callback(false);
        }
        return;
    }
    
    log_debug("Sending message: packet_id={}, payload_size={}, total_size={}/{}", 
              packet_id, protobuf_data.size(), encoded_packet.size(), 
              PacketCodec::MAX_PACKET_SIZE);
    
    if (ack_callback) {
        ack_manager_->registerPacket(packet_id, 
            [ack_callback](bool success, uint32_t id) {
                ack_callback(success);
            }, 
            timeout_ms);
    }
    
    if (auto* transport = getActiveTransport()) {
        transport->writeAsync(encoded_packet);
    }
}

void CommunicationBackend::handleReceivedPacket(const std::vector<uint8_t>& packet_data) {
    log_debug("Received packet: {} bytes", packet_data.size());
    decodeAndProcessMessage(packet_data);
}

void CommunicationBackend::decodeAndProcessMessage(const std::vector<uint8_t>& data) {
    // TODO: Parse protobuf matching op-controls feat/protobuf-commands
    // This updates gimbal_state_ (thread-safe via mutex)
    
    log_debug("Message processed");
}
