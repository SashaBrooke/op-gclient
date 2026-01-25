#include "core/packet_codec.hpp"

std::vector<uint8_t> VarintPacketCodec::encodeVarint(uint64_t value) {
    std::vector<uint8_t> result;
    while (value >= 0x80) {
        result.push_back(static_cast<uint8_t>((value & 0x7F) | 0x80));
        value >>= 7;
    }
    result.push_back(static_cast<uint8_t>(value & 0x7F));
    return result;
}

std::vector<uint8_t> VarintPacketCodec::encode(const std::vector<uint8_t>& message_data) {
    auto varint = encodeVarint(message_data.size());
    std::vector<uint8_t> packet;
    packet.reserve(varint.size() + message_data.size());
    packet.insert(packet.end(), varint.begin(), varint.end());
    packet.insert(packet.end(), message_data.begin(), message_data.end());
    return packet;
}

bool VarintPacketCodec::decodeVarint(const std::vector<uint8_t>& buffer, 
                                    uint64_t& out_value, 
                                    size_t& out_bytes_consumed) {
    out_value = 0;
    out_bytes_consumed = 0;
    
    int shift = 0;
    for (size_t i = 0; i < buffer.size() && i < 10; ++i) {  // Max 10 bytes for uint64
        uint8_t byte = buffer[i];
        out_value |= static_cast<uint64_t>(byte & 0x7F) << shift;
        out_bytes_consumed++;
        
        if ((byte & 0x80) == 0) {
            return true;  // Success
        }
        shift += 7;
    }
    
    return false;  // Incomplete varint
}
