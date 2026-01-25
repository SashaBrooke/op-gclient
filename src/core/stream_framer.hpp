#ifndef STREAM_FRAMER_HPP
#define STREAM_FRAMER_HPP

#include "core/packet_codec.hpp"
#include <vector>
#include <functional>
#include <cstdint>

/**
 * Handles varint-based framing for byte streams
 * Works with any transport (serial, network, file, etc.)
 * 
 * Usage:
 *   StreamFramer framer([](const std::vector<uint8_t>& packet) {
 *       // Process complete packet
 *   });
 *   
 *   // Feed incoming bytes
 *   framer.feedData(buffer, bytes_read);
 */
class StreamFramer {
public:
    using PacketCallback = std::function<void(const std::vector<uint8_t>&)>;
    
    explicit StreamFramer(PacketCallback callback);
    
    /**
     * Feed incoming bytes to the framer
     * Automatically detects complete packets and calls callback
     */
    void feedData(const uint8_t* data, size_t length);
    
    /**
     * Reset framing state (e.g., after reconnect)
     */
    void reset();
    
private:
    enum class State {
        READING_VARINT,
        READING_PAYLOAD
    };
    
    void processVarintBytes();
    void processPayloadBytes();
    
    State state_;
    VarintPacketCodec codec_;
    std::vector<uint8_t> varint_buffer_;
    std:: vector<uint8_t> payload_buffer_;
    uint64_t expected_payload_size_;
    PacketCallback packet_callback_;
};

#endif // STREAM_FRAMER_HPP
