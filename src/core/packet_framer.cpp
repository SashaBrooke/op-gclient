#include "core/packet_framer.hpp"
#include "util/logging.hpp"

PacketFramer::PacketFramer(PacketCallback callback)
    : state_(State::SEARCHING_SYNC)
    , expected_length_(0)
    , packet_callback_(callback)
    , dropped_sync_bytes_(0)
    , packets_received_(0) {
    payload_buffer_.reserve(PacketCodec::MAX_PAYLOAD_SIZE);
    length_buffer_.reserve(2);  // Max 2 bytes for varint (though typically 1)
}

void PacketFramer::feedData(const uint8_t* data, size_t length) {
    for (size_t i = 0; i < length; ++i) {
        uint8_t byte = data[i];
        
        switch (state_) {
            case State::SEARCHING_SYNC:
                if (byte == PacketCodec::SYNC_BYTE) {
                    log_debug("Found sync byte");
                    length_buffer_.clear();
                    state_ = State::READING_LENGTH;
                } else {
                    dropped_sync_bytes_++;
                }
                break;
                
            case State::READING_LENGTH:
                // Accumulate varint bytes
                length_buffer_.push_back(byte);
                
                // Try to decode varint
                uint64_t payload_length;
                size_t varint_bytes;
                if (codec_.decodeVarint(length_buffer_, payload_length, varint_bytes)) {
                    // Varint complete!
                    expected_length_ = payload_length;
                    
                    log_debug("Decoded length: {} bytes (varint used {} byte{})", 
                              expected_length_, varint_bytes, varint_bytes > 1 ? "s" : "");
                    
                    if (expected_length_ == 0) {
                        // Zero-length packet (valid edge case)
                        log_debug("Received zero-length packet");
                        if (packet_callback_) {
                            std::vector<uint8_t> empty;
                            packet_callback_(empty);
                        }
                        packets_received_++;
                        state_ = State::SEARCHING_SYNC;
                    } else {
                        // Start reading payload
                        payload_buffer_.clear();
                        payload_buffer_.reserve(expected_length_);
                        state_ = State::READING_PAYLOAD;
                    }
                } else if (length_buffer_.size() >= 2) {
                    // Varint should never exceed 1 byte for lengths ≤62
                    // If we have 2+ bytes and still no valid varint, it's corrupt
                    log_warn("Invalid varint length (>1 byte for length ≤62), resetting");
                    length_buffer_.clear();
                    state_ = State::SEARCHING_SYNC;
                    dropped_sync_bytes_++;
                }
                break;
                
            case State::READING_PAYLOAD:
                payload_buffer_.push_back(byte);
                
                // Check if packet complete
                if (payload_buffer_.size() >= expected_length_) {
                    log_debug("Received complete packet ({} bytes payload)", payload_buffer_.size());
                    
                    // Deliver packet
                    if (packet_callback_) {
                        packet_callback_(payload_buffer_);
                    }
                    
                    packets_received_++;
                    
                    // Reset for next packet
                    payload_buffer_.clear();
                    state_ = State::SEARCHING_SYNC;
                }
                break;
        }
    }
}

void PacketFramer::reset() {
    state_ = State::SEARCHING_SYNC;
    length_buffer_.clear();
    payload_buffer_.clear();
    expected_length_ = 0;
    log_debug("PacketFramer reset (dropped: {}, received: {})", 
              dropped_sync_bytes_, packets_received_);
}
