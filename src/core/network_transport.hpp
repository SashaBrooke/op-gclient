#ifndef NETWORK_TRANSPORT_HPP
#define NETWORK_TRANSPORT_HPP

#include "core/transport_interface.hpp"
#include "core/stream_framer.hpp"  // ✅ CHANGED: Use shared framer instead of packet_codec
#include <boost/asio.hpp>
#include <thread>
#include <mutex>
#include <array>

/**
 * Network (TCP) transport - handles ONLY I/O
 */
class NetworkTransport :  public ITransport {
public: 
    NetworkTransport(const std::string& host, uint16_t port);
    ~NetworkTransport() override;
    
    bool open() override;
    void close() override;
    bool isOpen() const override;
    void writeAsync(const std::vector<uint8_t>& data) override;
    void setPacketReceivedCallback(PacketReceivedCallback callback) override;
    boost::asio::io_context& getIoContext() override { return io_context_; }
    std::string getConnectionInfo() const override;
    
private:  
    void startAsyncRead();
    void handleReadSome(const boost::system::error_code& ec, size_t bytes_read);
    
    std::string host_;
    uint16_t port_;
    
    boost::asio::io_context io_context_;
    boost::asio::ip::tcp:: socket socket_;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work_guard_;
    std::thread io_thread_;
    
    std:: array<uint8_t, 1024> temp_read_buffer_;
    StreamFramer framer_;  // ✅ CHANGED: Use StreamFramer instead of manual state machine
    
    PacketReceivedCallback packet_callback_;
    std::mutex callback_mutex_;
    
    bool is_connected_;
    
    // ✅ REMOVED: All these are now inside StreamFramer
    // std:: vector<uint8_t> varint_buffer_;
    // std::vector<uint8_t> payload_buffer_;
    // ReadState read_state_;
    // uint64_t expected_payload_size_;
    // VarintPacketCodec codec_;
};

#endif // NETWORK_TRANSPORT_HPP
