#include "core/packet_ack_manager.hpp"
#include "util/logging.hpp"
#include <boost/asio.hpp>

PacketAckManager::PacketAckManager(boost::asio::io_context& io_context)
    : io_context_(io_context)
    , next_packet_id_(1) {
}

PacketAckManager::~PacketAckManager() {
    log_debug("PacketAckManager destructor");
    cancelAll();
    log_debug("PacketAckManager destroyed");
}

uint32_t PacketAckManager::getNextPacketId() {
    std::lock_guard<std::mutex> lock(mutex_);
    return next_packet_id_++;
}

void PacketAckManager::registerPacket(uint32_t packet_id, AckCallback callback, uint32_t timeout_ms) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    PendingPacket pending;
    pending.packet_id = packet_id;
    pending.callback = callback;
    pending.timer = std::make_shared<boost::asio::steady_timer>(io_context_);
    
    pending.timer->expires_after(std::chrono::milliseconds(timeout_ms));
    pending.timer->async_wait([this, packet_id](const boost::system::error_code& ec) {
        if (! ec) {
            handleTimeout(packet_id);
        }
    });
    
    pending_packets_[packet_id] = std::move(pending);
    log_debug("Registered packet {} with {}ms timeout", packet_id, timeout_ms);
}

void PacketAckManager::handleAck(uint32_t packet_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = pending_packets_.find(packet_id);
    if (it != pending_packets_.end()) {
        log_debug("Received ACK for packet {}", packet_id);
        it->second.timer->cancel();
        auto callback = it->second.callback;
        pending_packets_.erase(it);
        
        // Call callback outside lock to avoid deadlock
        lock.~lock_guard();
        if (callback) {
            callback(true, packet_id);
        }
    } else {
        log_warn("Received ACK for unknown packet {}", packet_id);
    }
}

void PacketAckManager::handleTimeout(uint32_t packet_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = pending_packets_.find(packet_id);
    if (it != pending_packets_.end()) {
        log_warn("Timeout waiting for ACK of packet {}", packet_id);
        auto callback = it->second.callback;
        pending_packets_.erase(it);
        
        // Call callback outside lock
        lock.~lock_guard();
        if (callback) {
            callback(false, packet_id);
        }
    }
}

void PacketAckManager::cancelAll() {
    std::lock_guard<std::mutex> lock(mutex_);

    log_info("Cancelling {} pending packets", pending_packets_.size());
    for (auto& [id, pending] : pending_packets_) {
        pending.timer->cancel();
        if (pending.callback) {
            pending.callback(false, id);
        }
    }
    pending_packets_.clear();
}
