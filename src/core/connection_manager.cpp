#include "core/connection_manager.hpp"
#include "core/serial_transport.hpp"
#include "core/network_transport.hpp"
#include "util/logging.hpp"
#include <sstream>

ConnectionManager::ConnectionManager()
    : state_(ConnectionState::Disconnected)
    , type_(ConnectionType::None)
    , device_info_("")
    , error_message_("")
    , backend_(std::make_unique<CommunicationBackend>()) {
    
    log_debug("ConnectionManager initialized");
}

ConnectionManager::~ConnectionManager() {
    log_debug("ConnectionManager destructor");
    disconnect();
    backend_.reset();
    log_debug("ConnectionManager destroyed");
}

// ========================================
// State Queries
// ========================================

bool ConnectionManager::isConnected() const {
    return state_ == ConnectionState::Connected && 
           backend_ && 
           backend_->isConnected();
}

// ========================================
// Connection Management
// ========================================

bool ConnectionManager::connectSerial(const std::string& port, int baud_rate) {
    // Disconnect any existing connection first
    if (state_ != ConnectionState::Disconnected) {
        log_warn("Disconnecting existing {} connection before connecting to serial", 
                 type_ == ConnectionType::Serial ? "serial" : "network");
        disconnect();
    }
    
    log_info("Attempting to connect to serial port: {} at {} baud", port, baud_rate);
    
    try {
        // Set state to connecting
        state_ = ConnectionState::Connecting;
        type_ = ConnectionType::Serial;
        device_info_ = port + " @ " + std::to_string(baud_rate) + " baud";
        error_message_.clear();
        
        // Create serial transport
        auto transport = std::make_unique<SerialTransport>(port, baud_rate);
        
        // Connect backend with transport
        if (backend_->connect(std::move(transport))) {
            state_ = ConnectionState::Connected;
            log_info("Successfully connected to serial port");
            return true;
        } else {
            state_ = ConnectionState::Error;
            type_ = ConnectionType::None;
            error_message_ = "Failed to open serial port";
            device_info_.clear();
            log_error("Failed to open serial port:  {}", port);
            return false;
        }
        
    } catch (const std::exception& e) {
        state_ = ConnectionState::Error;
        type_ = ConnectionType::None;
        error_message_ = std::string("Exception: ") + e.what();
        device_info_.clear();
        log_error("Exception connecting to serial: {}", e.what());
        return false;
    }
}

bool ConnectionManager::connectNetwork(const std::string& host, int port) {
    // Disconnect any existing connection first
    if (state_ != ConnectionState::Disconnected) {
        log_warn("Disconnecting existing {} connection before connecting to network", 
                 type_ == ConnectionType::Serial ? "serial" : "network");
        disconnect();
    }
    
    log_info("Attempting to connect to network: {}:{}", host, port);
    
    try {
        // Set state to connecting
        state_ = ConnectionState::Connecting;
        type_ = ConnectionType::Network;
        device_info_ = host + ":" + std::to_string(port);
        error_message_.clear();
        
        // Create network transport
        auto transport = std::make_unique<NetworkTransport>(host, port);
        
        // Connect backend with transport
        if (backend_->connect(std::move(transport))) {
            state_ = ConnectionState::Connected;
            log_info("Successfully connected to network");
            return true;
        } else {
            state_ = ConnectionState::Error;
            type_ = ConnectionType::None;
            error_message_ = "Failed to connect to network";
            device_info_.clear();
            log_error("Failed to connect to network: {}:{}", host, port);
            return false;
        }
        
    } catch (const std::exception& e) {
        state_ = ConnectionState::Error;
        type_ = ConnectionType::None;
        error_message_ = std::string("Exception: ") + e.what();
        device_info_.clear();
        log_error("Exception connecting to network:  {}", e.what());
        return false;
    }
}

void ConnectionManager::disconnect() {
    if (state_ == ConnectionState::Disconnected) {
        return;
    }
    
    log_info("Disconnecting from {} device:  {}", 
             type_ == ConnectionType::Serial ? "serial" : "network",
             device_info_);
    
    // Disconnect backend (also resets GimbalState)
    if (backend_) {
        backend_->disconnect();
    }
    
    // Reset state
    state_ = ConnectionState::Disconnected;
    type_ = ConnectionType::None;
    device_info_.clear();
    error_message_.clear();
    
    log_info("Disconnected successfully");
}

// ========================================
// Communication
// ========================================

CommunicationBackend* ConnectionManager::getBackend() {
    if (isConnected()) {
        return backend_.get();
    }
    return nullptr;
}

const CommunicationBackend* ConnectionManager::getBackend() const {
    if (isConnected()) {
        return backend_.get();
    }
    return nullptr;
}

void ConnectionManager::sendMessage(const std::vector<uint8_t>& protobuf_data,
                                   std::function<void(bool success)> ack_callback,
                                   uint32_t timeout_ms) {
    if (!isConnected()) {
        log_warn("Cannot send message: not connected");
        if (ack_callback) {
            ack_callback(false);
        }
        return;
    }
    
    backend_->sendMessage(protobuf_data, ack_callback, timeout_ms);
}

// ========================================
// Utility
// ========================================

std::string ConnectionManager::getStatusString() const {
    std::ostringstream oss;
    
    switch (state_) {
        case ConnectionState::Disconnected: 
            oss << "Disconnected";
            break;
            
        case ConnectionState::Connecting:
            oss << "Connecting to ";
            if (type_ == ConnectionType::Serial) {
                oss << "serial";
            } else if (type_ == ConnectionType::Network) {
                oss << "network";
            }
            if (! device_info_.empty()) {
                oss << " (" << device_info_ << ")";
            }
            break;
            
        case ConnectionState::Connected:
            oss << "Connected to ";
            if (type_ == ConnectionType::Serial) {
                oss << "serial";
            } else if (type_ == ConnectionType::Network) {
                oss << "network";
            }
            if (!device_info_.empty()) {
                oss << " (" << device_info_ << ")";
            }
            break;
            
        case ConnectionState::Error:
            oss << "Error";
            if (!error_message_.empty()) {
                oss << ": " << error_message_;
            }
            break;
    }
    
    return oss.str();
}
