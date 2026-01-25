#ifndef COMMUNICATION_BACKEND_HPP
#define COMMUNICATION_BACKEND_HPP

#include <memory>
#include <functional>
#include <vector>
#include <cstdint>
#include "core/transport_interface.hpp"
#include "core/packet_codec.hpp"
#include "core/packet_ack_manager.hpp"

/**
 * Single communication backend that handles: 
 * - Protobuf encoding/decoding
 * - Packet ACK management
 * - GimbalState updates
 * - Works with ANY transport (serial, network, etc.)
 */
class CommunicationBackend {
public:
    CommunicationBackend();
    ~CommunicationBackend();
    
    /**
     * Connect using a specific transport
     * Takes ownership of transport
     */
    bool connect(std::unique_ptr<ITransport> transport);
    
    /**
     * Disconnect and cleanup
     */
    void disconnect();
    
    /**
     * Check if connected
     */
    bool isConnected() const;
    
    /**
     * Get connection info string
     */
    std::string getConnectionInfo() const;
    
    /**
     * Send a protobuf message with optional ACK
     * @param protobuf_data Serialized protobuf message
     * @param ack_callback Optional callback for ACK confirmation
     * @param timeout_ms ACK timeout
     */
    void sendMessage(const std::vector<uint8_t>& protobuf_data,
                    std::function<void(bool success)> ack_callback = nullptr,
                    uint32_t timeout_ms = 1000);
    
    /**
     * Send a command (convenience wrapper)
     */
    template<typename ProtoMessageType>
    void sendCommand(const ProtoMessageType& message,
                    std::function<void(bool)> ack_callback = nullptr);
    
private:
    void handleReceivedPacket(const std::vector<uint8_t>& packet_data);
    void decodeAndProcessMessage(const std::vector<uint8_t>& data);
    
    std::unique_ptr<ITransport> transport_;
    std::unique_ptr<PacketAckManager> ack_manager_;
    VarintPacketCodec codec_;
};

#endif // COMMUNICATION_BACKEND_HPP
