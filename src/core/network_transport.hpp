#ifndef NETWORK_TRANSPORT_HPP
#define NETWORK_TRANSPORT_HPP

#include "core/transport_interface.hpp"
#include "core/packet_framer.hpp"
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
    boost::asio::ip::tcp::socket socket_;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work_guard_;
    std::thread io_thread_;
    
    std::array<uint8_t, 1024> temp_read_buffer_;
    PacketFramer framer_;
    
    PacketReceivedCallback packet_callback_;
    std::mutex callback_mutex_;
    
    bool is_connected_;
};

#endif // NETWORK_TRANSPORT_HPP
