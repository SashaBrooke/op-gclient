#include "core/stream_framer.hpp"
#include "util/logging.hpp"

StreamFramer::StreamFramer(PacketCallback callback)
    : state_(State::READING_VARINT)
    , expected_payload_size_(0)
    , packet_callback_(callback) {
}

void StreamFramer::feedData(const uint8_t* data, size_t length) {
    size_t offset = 0;
    
    while (offset < length) {
        if (state_ == State::READING_VARINT) {
            // Accumulate varint bytes
            size_t bytes_to_copy = length - offset;
            varint_buffer_.insert(varint_buffer_.end(), 
                                 data + offset, 
                                 data + offset + bytes_to_copy);
            offset += bytes_to_copy;
            
            // Try to decode varint
            uint64_t packet_length;
            size_t varint_bytes;
            if (codec_.decodeVarint(varint_buffer_, packet_length, varint_bytes)) {
                log_debug("Decoded varint:  payload length = {}", packet_length);
                
                expected_payload_size_ = packet_length;
                payload_buffer_.clear();
                payload_buffer_.reserve(packet_length);
                
                // Move any extra bytes to payload buffer
                if (varint_buffer_.size() > varint_bytes) {
                    payload_buffer_.insert(payload_buffer_.end(),
                                          varint_buffer_.begin() + varint_bytes,
                                          varint_buffer_.end());
                }
                
                varint_buffer_.clear();
                state_ = State::READING_PAYLOAD;
            }
            
        } else if (state_ == State::READING_PAYLOAD) {
            // Accumulate payload bytes
            size_t remaining = expected_payload_size_ - payload_buffer_.size();
            size_t bytes_to_copy = std::min(length - offset, remaining);
            
            payload_buffer_.insert(payload_buffer_.end(),
                                  data + offset,
                                  data + offset + bytes_to_copy);
            offset += bytes_to_copy;
            
            // Check if packet complete
            if (payload_buffer_.size() >= expected_payload_size_) {
                log_debug("Received complete packet ({} bytes)", payload_buffer_.size());
                
                // Deliver packet
                if (packet_callback_) {
                    packet_callback_(payload_buffer_);
                }
                
                // Reset for next packet
                state_ = State::READING_VARINT;
                payload_buffer_.clear();
                varint_buffer_.clear();
            }
        }
    }
}

void StreamFramer::reset() {
    state_ = State::READING_VARINT;
    varint_buffer_.clear();
    payload_buffer_.clear();
    expected_payload_size_ = 0;
    log_debug("StreamFramer reset");
}
