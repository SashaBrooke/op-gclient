#include "core/network_transport.hpp"
#include "util/logging.hpp"

NetworkTransport::NetworkTransport(const std::string& host, uint16_t port)
    : host_(host)
    , port_(port)
    , socket_(io_context_)
    , work_guard_(boost::asio::make_work_guard(io_context_))
    , framer_([this](const std::vector<uint8_t>& packet) {
          // Framer delivers complete packets
          std::lock_guard<std::mutex> lock(callback_mutex_);
          if (packet_callback_) {
              packet_callback_(packet);
          }
      })
    , is_connected_(false) {
}

NetworkTransport::~NetworkTransport() {
    close();
}

bool NetworkTransport::open() {
    if (is_connected_) {
        log_warn("Network transport already open");
        return true;
    }
    
    try {
        log_info("Connecting to network: {}:{}", host_, port_);
        
        // Resolve hostname
        boost::asio::ip::tcp::resolver resolver(io_context_);
        auto endpoints = resolver.resolve(host_, std::to_string(port_));
        
        // Connect to endpoint
        boost::asio::connect(socket_, endpoints);
        
        // Start I/O thread
        io_thread_ = std::thread([this]() {
            log_debug("Network I/O thread started");
            io_context_.run();
            log_debug("Network I/O thread stopped");
        });
        
        // Start reading
        startAsyncRead();
        
        is_connected_ = true;
        log_info("Network transport connected successfully");
        return true;
        
    } catch (const boost::system::system_error& e) {
        log_error("Failed to connect to network: {}", e.what());
        return false;
    }
}

void NetworkTransport::close() {
    if (!is_connected_) {
        return;
    }
    
    log_info("Closing network transport");
    
    is_connected_ = false;
    
    try {
        socket_.close();
    } catch (const std::exception& e) {
        log_error("Error closing socket: {}", e.what());
    } catch (...) {
        log_error("Unknown error closing socket");
    }
    
    io_context_.stop();
    
    if (io_thread_.joinable()) {
        log_debug("Joining network I/O thread");
        io_thread_.join();
        log_debug("Network I/O thread joined");
    }
    
    framer_.reset();
    
    log_info("Network transport closed");
}

bool NetworkTransport::isOpen() const {
    return is_connected_;
}

void NetworkTransport::writeAsync(const std::vector<uint8_t>& data) {
    if (!is_connected_) {
        log_warn("Attempted write to disconnected network");
        return;
    }
    
    auto buffer = std::make_shared<std::vector<uint8_t>>(data);
    
    boost::asio::async_write(
        socket_,
        boost::asio::buffer(*buffer),
        [buffer](const boost::system::error_code& ec, size_t bytes_written) {
            if (ec) {
                log_error("Network write error: {}", ec.message());
            } else {
                log_debug("Wrote {} bytes to network", bytes_written);
            }
        }
    );
}

void NetworkTransport::setPacketReceivedCallback(PacketReceivedCallback callback) {
    std::lock_guard<std::mutex> lock(callback_mutex_);
    packet_callback_ = callback;
}

std::string NetworkTransport::getConnectionInfo() const {
    return host_ + ":" + std::to_string(port_);
}

void NetworkTransport::startAsyncRead() {
    socket_.async_read_some(
        boost::asio::buffer(temp_read_buffer_),
        [this](const boost::system::error_code& ec, size_t bytes_read) {
            handleReadSome(ec, bytes_read);
        }
    );
}

void NetworkTransport::handleReadSome(const boost::system::error_code& ec, size_t bytes_read) {
    if (ec) {
        if (ec != boost::asio::error::operation_aborted) {
            log_error("Network read error: {}", ec.message());
        }
        return;
    }
    
    log_debug("Read {} bytes from network", bytes_read);

    framer_.feedData(temp_read_buffer_.data(), bytes_read);
    
    startAsyncRead();
}
