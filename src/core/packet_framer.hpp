#ifndef PACKET_FRAMER_HPP
#define PACKET_FRAMER_HPP

#include "core/packet_codec.hpp"
#include <vector>
#include <functional>
#include <cstdint>

/**
 * Handles packet framing for op-controls protocol:
 * [0xAA sync] [1-byte varint length] [payload]
 * 
 * Max total packet size: 64 bytes
 * Max payload: 62 bytes (64 - sync - length)
 * 
 * State machine:
 * 1. Search for 0xAA sync byte
 * 2. Read varint length (guaranteed 1 byte for payloads ≤62)
 * 3. Read payload of specified length
 * 4. Deliver complete packet via callback
 */
class PacketFramer {
public:
    using PacketCallback = std::function<void(const std::vector<uint8_t>&)>;
    
    explicit PacketFramer(PacketCallback callback);
    
    /**
     * Feed incoming bytes to the framer
     * Automatically detects complete packets and calls callback
     */
    void feedData(const uint8_t* data, size_t length);
    
    /**
     * Reset framing state (e.g., after reconnect or error)
     */
    void reset();
    
    /**
     * Get statistics
     */
    size_t getDroppedSyncBytes() const { return dropped_sync_bytes_; }
    size_t getPacketsReceived() const { return packets_received_; }
    
private:
    enum class State {
        SEARCHING_SYNC,   // Looking for 0xAA
        READING_LENGTH,   // Accumulating varint length bytes
        READING_PAYLOAD   // Reading payload bytes
    };
    
    State state_;
    PacketCodec codec_;
    
    std::vector<uint8_t> length_buffer_;  // Accumulate varint bytes
    uint64_t expected_length_;
    std::vector<uint8_t> payload_buffer_;
    
    PacketCallback packet_callback_;
    
    // Statistics
    size_t dropped_sync_bytes_;
    size_t packets_received_;
};

#endif // PACKET_FRAMER_HPP
