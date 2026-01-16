#include "core/serial_connection.hpp"
#include "util/logging.hpp"

SerialConnection::SerialConnection() 
    : is_open_(false)
    , port_("")
    , baud_rate_(0) {
    log_debug("SerialConnection created");
}

SerialConnection::~SerialConnection() {
    close();
    log_debug("SerialConnection destroyed");
}

bool SerialConnection::open(const std::string& port, int baud_rate) {
    log_info("Opening serial port: {} at {} baud", port, baud_rate);
    
    if (is_open_) {
        log_warn("Serial port already open, closing first");
        close();
    }
    
    port_ = port;
    baud_rate_ = baud_rate;
    
    // TODO: Implement actual serial port opening
    // - Linux: open() syscall on /dev/ttyUSB0 or /dev/ttyACM0
    // - Windows: CreateFile() on COM ports
    // - Or use boost::asio::serial_port
    
    is_open_ = true;  // Temporary - remove when implementing
    log_info("Serial port opened successfully (stub)");
    return true;
}

void SerialConnection::close() {
    if (is_open_) {
        log_info("Closing serial port:  {}", port_);
        
        // TODO: Implement actual close
        
        is_open_ = false;
        port_.clear();
        baud_rate_ = 0;
    }
}

bool SerialConnection::isOpen() const {
    return is_open_;
}

bool SerialConnection::write(const std::vector<uint8_t>& data) {
    if (!is_open_) {
        log_warn("Attempted to write to closed serial port");
        return false;
    }
    
    log_debug("Writing {} bytes to serial port", data.size());
    
    // TODO: Implement actual write
    // - Linux: ::write() syscall
    // - Windows: WriteFile()
    // - Or use boost::asio::write()
    
    return true;  // Temporary - remove when implementing
}

bool SerialConnection::write(const std::string& data) {
    std::vector<uint8_t> bytes(data.begin(), data.end());
    return write(bytes);
}

size_t SerialConnection::read(std::vector<uint8_t>& buffer, size_t max_bytes) {
    if (!is_open_) {
        log_warn("Attempted to read from closed serial port");
        return 0;
    }
    
    log_debug("Reading up to {} bytes from serial port", max_bytes);
    
    // TODO: Implement actual read
    // - Linux: ::read() syscall
    // - Windows: ReadFile()
    // - Or use boost::asio::read()
    
    buffer.clear();
    return 0;  // Temporary - remove when implementing
}

std::string SerialConnection::getPort() const {
    return port_;
}

int SerialConnection::getBaudRate() const {
    return baud_rate_;
}
