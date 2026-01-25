#ifndef PACKET_CODEC_HPP
#define PACKET_CODEC_HPP

#include <vector>
#include <cstdint>

/**
 * Packet codec for op-controls protocol:
 * [0xAA sync byte] [1-byte varint length] [payload]
 * 
 * Total packet size: max 64 bytes
 * Therefore: max payload = 64 - 1 (sync) - 1 (length) = 62 bytes
 * 
 * Since payload is max 62 bytes, the varint length is always 1 byte.
 */
class PacketCodec {
public:
    static constexpr uint8_t SYNC_BYTE = 0xAA;
    static constexpr size_t MAX_PACKET_SIZE = 64;  // Total packet size
    static constexpr size_t MAX_PAYLOAD_SIZE = 62;  // 64 - 1 (sync) - 1 (length)
    
    /**
     * Encode payload with sync byte and varint length prefix
     * Returns: [0xAA] [varint length] [payload]
     * 
     * @throws std::runtime_error if payload exceeds MAX_PAYLOAD_SIZE
     */
    std::vector<uint8_t> encode(const std::vector<uint8_t>& payload);
    
    /**
     * Encode varint (though with max 62, this is always 1 byte)
     */
    static std::vector<uint8_t> encodeVarint(uint64_t value);
    
    /**
     * Try to decode varint length from buffer
     * Returns true if varint successfully decoded
     */
    bool decodeVarint(const std::vector<uint8_t>& buffer, 
                     uint64_t& out_length, 
                     size_t& out_bytes_consumed);
};

#endif // PACKET_CODEC_HPP
