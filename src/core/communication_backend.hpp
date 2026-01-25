#ifndef COMMUNICATION_BACKEND_HPP
#define COMMUNICATION_BACKEND_HPP

#include <memory>
#include <functional>
#include <vector>
#include <cstdint>
#include <variant>
#include <optional>
#include "core/transport_interface.hpp"
#include "core/serial_transport.hpp"
#include "core/network_transport.hpp"
#include "core/packet_codec.hpp"
#include "core/packet_ack_manager.hpp"
#include "core/gimbal_state.hpp"

/**
 * Communication backend with variant transport
 * Runs on transport's I/O thread, NOT the GUI thread
 */
class CommunicationBackend {
public:
    enum class TransportType {
        None,
        Serial,
        Network
    };
    
    explicit CommunicationBackend(GimbalState& gimbal_state);
    ~CommunicationBackend();
    
    // Delete copy/move
    CommunicationBackend(const CommunicationBackend&) = delete;
    CommunicationBackend& operator=(const CommunicationBackend&) = delete;
    
    // Connection management
    bool connectSerial(const std::string& port, uint32_t baud_rate);
    bool connectNetwork(const std::string& host, uint16_t port);
    void disconnect();
    
    // Status queries
    bool isConnected() const;
    TransportType getTransportType() const;
    std::string getConnectionInfo() const;
    std::string getErrorMessage() const { return error_message_; }
    
    // Communication
    void sendMessage(const std::vector<uint8_t>& protobuf_data,
                    std::function<void(bool success)> ack_callback = nullptr,
                    uint32_t timeout_ms = 1000);
    
private:
    void handleReceivedPacket(const std::vector<uint8_t>& packet_data);
    void decodeAndProcessMessage(const std::vector<uint8_t>& data);
    
    GimbalState& gimbal_state_;
    
    // Variant holds exactly ONE transport at a time
    using TransportVariant = std::variant<
        std::monostate,  // None/disconnected
        std::unique_ptr<SerialTransport>,
        std::unique_ptr<NetworkTransport>
    >;
    
    TransportVariant transport_;
    std::unique_ptr<PacketAckManager> ack_manager_;
    PacketCodec codec_;
    
    std::string error_message_;
    
    // Helper to get active transport interface
    ITransport* getActiveTransport();
    const ITransport* getActiveTransport() const;
};

#endif // COMMUNICATION_BACKEND_HPP
