#ifndef PACKET_CODEC_HPP
#define PACKET_CODEC_HPP

#include <vector>
#include <cstdint>
#include <optional>
#include <functional>

/**
 * Abstract packet codec interface
 * Handles encoding/decoding of packets with varint length prefix
 */
class IPacketCodec {
public: 
    virtual ~IPacketCodec() = default;
    
    /**
     * Encode a protobuf message to bytes with varint length prefix
     * @param message_data Serialized protobuf message
     * @return Complete packet (varint + payload)
     */
    virtual std::vector<uint8_t> encode(const std::vector<uint8_t>& message_data) = 0;
    
    /**
     * Try to decode a varint from buffer
     * @param buffer Input buffer
     * @param out_value Decoded value
     * @param out_bytes_consumed Number of bytes consumed
     * @return true if varint successfully decoded
     */
    virtual bool decodeVarint(const std::vector<uint8_t>& buffer, 
                             uint64_t& out_value, 
                             size_t& out_bytes_consumed) = 0;
};

/**
 * Standard varint implementation (protobuf style)
 */
class VarintPacketCodec : public IPacketCodec {
public:
    std::vector<uint8_t> encode(const std::vector<uint8_t>& message_data) override;
    bool decodeVarint(const std::vector<uint8_t>& buffer, 
                     uint64_t& out_value, 
                     size_t& out_bytes_consumed) override;
    
private:
    std::vector<uint8_t> encodeVarint(uint64_t value);
};

#endif // PACKET_CODEC_HPP
