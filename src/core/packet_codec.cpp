#include "core/packet_codec.hpp"
#include <stdexcept>

std::vector<uint8_t> PacketCodec::encodeVarint(uint64_t value) {
    std::vector<uint8_t> result;
    
    // Standard varint encoding (though with max 62, always 1 byte)
    while (value >= 0x80) {
        result.push_back(static_cast<uint8_t>((value & 0x7F) | 0x80));
        value >>= 7;
    }
    result.push_back(static_cast<uint8_t>(value & 0x7F));
    
    return result;
}

std::vector<uint8_t> PacketCodec::encode(const std::vector<uint8_t>& payload) {
    if (payload.size() > MAX_PAYLOAD_SIZE) {
        throw std::runtime_error("Payload exceeds max size of 62 bytes");
    }
    
    std::vector<uint8_t> packet;
    packet.reserve(MAX_PACKET_SIZE);
    
    // Sync byte
    packet.push_back(SYNC_BYTE);
    
    // Varint length (will be 1 byte for values 0-62)
    auto varint = encodeVarint(payload.size());
    packet.insert(packet.end(), varint.begin(), varint.end());
    
    // Payload
    packet.insert(packet.end(), payload.begin(), payload.end());
    
    // Verify total size
    if (packet.size() > MAX_PACKET_SIZE) {
        throw std::runtime_error("Total packet exceeds 64 bytes");
    }
    
    return packet;
}

bool PacketCodec::decodeVarint(const std::vector<uint8_t>& buffer, 
                               uint64_t& out_length, 
                               size_t& out_bytes_consumed) {
    if (buffer.empty()) {
        return false;
    }
    
    out_length = 0;
    out_bytes_consumed = 0;
    
    int shift = 0;
    for (size_t i = 0; i < buffer.size() && i < 10; ++i) {  // Max 10 bytes for varint
        uint8_t byte = buffer[i];
        out_length |= static_cast<uint64_t>(byte & 0x7F) << shift;
        out_bytes_consumed++;
        
        if ((byte & 0x80) == 0) {
            // Varint complete, validate length
            if (out_length > MAX_PAYLOAD_SIZE) {
                return false;  // Invalid length
            }
            return true;
        }
        shift += 7;
    }
    
    return false;  // Incomplete varint
}
