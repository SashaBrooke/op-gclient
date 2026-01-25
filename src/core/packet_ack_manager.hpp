#ifndef PACKET_ACK_MANAGER_HPP
#define PACKET_ACK_MANAGER_HPP

#include <cstdint>
#include <functional>
#include <unordered_map>
#include <mutex>
#include <chrono>
#include <memory>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/io_context.hpp>

/**
 * Manages packet acknowledgements and timeouts
 * Thread-safe, works with both serial and network
 */
class PacketAckManager {
public:  
    using AckCallback = std::function<void(bool success, uint32_t packet_id)>;
    
    explicit PacketAckManager(boost::asio::io_context& io_context);
    ~PacketAckManager();
    
    /**
     * Register a packet awaiting acknowledgement
     * @param packet_id Unique packet identifier
     * @param callback Called when ack received or timeout
     * @param timeout_ms Timeout in milliseconds
     */
    void registerPacket(uint32_t packet_id, AckCallback callback, uint32_t timeout_ms);
    
    /**
     * Handle received acknowledgement
     * @param packet_id ID of acknowledged packet
     */
    void handleAck(uint32_t packet_id);
    
    /**
     * Cancel all pending packets (e.g., on disconnect)
     */
    void cancelAll();
    
    /**
     * Get next unique packet ID
     */
    uint32_t getNextPacketId();
    
private:
    struct PendingPacket {
        uint32_t packet_id;
        AckCallback callback;
        std::shared_ptr<boost::asio::steady_timer> timer;
    };
    
    void handleTimeout(uint32_t packet_id);
    
    boost::asio::io_context& io_context_;
    std::mutex mutex_;
    std::unordered_map<uint32_t, PendingPacket> pending_packets_;
    uint32_t next_packet_id_;
};

#endif // PACKET_ACK_MANAGER_HPP
