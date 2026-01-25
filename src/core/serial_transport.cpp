#include "core/serial_transport.hpp"
#include "util/logging.hpp"

SerialTransport::SerialTransport(const std::string& port, uint32_t baud_rate)
    : port_(port)
    , baud_rate_(baud_rate)
    , serial_port_(io_context_)
    , work_guard_(boost::asio::make_work_guard(io_context_))
    , framer_([this](const std::vector<uint8_t>& packet) {
          // Framer delivers complete packets
          std::lock_guard<std::mutex> lock(callback_mutex_);
          if (packet_callback_) {
              packet_callback_(packet);
          }
      })
    , is_open_(false) {
}

SerialTransport::~SerialTransport() {
    close();
}

bool SerialTransport::open() {
    if (is_open_) {
        log_warn("Serial transport already open");
        return true;
    }
    
    try {
        log_info("Opening serial:   {} @ {} baud", port_, baud_rate_);
        
        serial_port_.open(port_);
        serial_port_.set_option(boost::asio:: serial_port_base::baud_rate(baud_rate_));
        serial_port_.set_option(boost::asio::serial_port_base::character_size(8));
        serial_port_.set_option(boost::asio::serial_port_base::parity(boost::asio::serial_port_base::parity:: none));
        serial_port_. set_option(boost::asio::serial_port_base::stop_bits(boost::asio:: serial_port_base::stop_bits::one));
        
        io_thread_ = std::thread([this]() {
            log_debug("Serial I/O thread started");
            io_context_.run();
            log_debug("Serial I/O thread stopped");
        });
        
        startAsyncRead();
        
        is_open_ = true;
        log_info("Serial transport opened successfully");
        return true;
        
    } catch (const boost::system::system_error& e) {
        log_error("Failed to open serial:   {}", e.what());
        return false;
    }
}

void SerialTransport:: close() {
    if (!is_open_) {
        return;
    }
    
    log_info("Closing serial transport");
    
    is_open_ = false;
    
    try {
        serial_port_.close();
    } catch (const std::exception& e) {
        log_error("Error closing serial port: {}", e.what());
    } catch (...) {
        log_error("Unknown error closing serial port");
    }
    
    io_context_.stop();
    
    if (io_thread_.joinable()) {
        log_debug("Joining serial I/O thread");
        io_thread_.join();
        log_debug("Serial I/O thread joined");
    }
    
    framer_.reset();
    
    log_info("Serial transport closed");
}

bool SerialTransport::isOpen() const {
    return is_open_;
}

void SerialTransport::writeAsync(const std::vector<uint8_t>& data) {
    if (!is_open_) {
        log_warn("Attempted write to closed serial");
        return;
    }
    
    auto buffer = std::make_shared<std::vector<uint8_t>>(data);
    
    boost::asio::async_write(
        serial_port_,
        boost::asio::buffer(*buffer),
        [buffer](const boost::system::error_code& ec, size_t bytes_written) {
            if (ec) {
                log_error("Serial write error: {}", ec.message());
            } else {
                log_debug("Wrote {} bytes to serial", bytes_written);
            }
        }
    );
}

void SerialTransport:: setPacketReceivedCallback(PacketReceivedCallback callback) {
    std::lock_guard<std::mutex> lock(callback_mutex_);
    packet_callback_ = callback;
}

std::string SerialTransport:: getConnectionInfo() const {
    return port_ + " @ " + std::to_string(baud_rate_) + " baud";
}

void SerialTransport::startAsyncRead() {
    serial_port_.async_read_some(
        boost::asio:: buffer(temp_read_buffer_),
        [this](const boost::system::error_code& ec, size_t bytes_read) {
            handleReadSome(ec, bytes_read);
        }
    );
}

void SerialTransport::handleReadSome(const boost::system::error_code& ec, size_t bytes_read) {
    if (ec) {
        if (ec != boost::asio::error::operation_aborted) {
            log_error("Serial read error: {}", ec. message());
        }
        return;
    }
    
    log_debug("Read {} bytes from serial", bytes_read);
    
    // ✅ Just feed data to framer - it handles everything! 
    framer_.feedData(temp_read_buffer_.data(), bytes_read);
    
    startAsyncRead();
}
