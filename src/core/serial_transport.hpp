#ifndef SERIAL_TRANSPORT_HPP
#define SERIAL_TRANSPORT_HPP

#include "core/transport_interface.hpp"
#include "core/stream_framer.hpp"  // ✅ Use shared framer
#include <boost/asio.hpp>
#include <thread>
#include <mutex>
#include <array>

class SerialTransport : public ITransport {
public:
    SerialTransport(const std::string& port, uint32_t baud_rate);
    ~SerialTransport() override;
    
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
    
    std::string port_;
    uint32_t baud_rate_;
    
    boost::asio::io_context io_context_;
    boost::asio::serial_port serial_port_;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work_guard_;
    std::thread io_thread_;
    
    std::array<uint8_t, 1024> temp_read_buffer_;
    StreamFramer framer_;  // ✅ Shared framing logic! 
    
    PacketReceivedCallback packet_callback_;
    std::mutex callback_mutex_;
    
    bool is_open_;
};

#endif // SERIAL_TRANSPORT_HPP
