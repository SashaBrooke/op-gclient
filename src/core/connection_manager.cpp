#include "core/connection_manager.hpp"
#include "core/serial_connection.hpp"
#include "core/network_connection.hpp"
#include "util/logging.hpp"

bool ConnectionManager::connectSerial(const std::string& port, int baud_rate) {
    // IMPORTANT: Disconnect any existing connection first
    if (state_ != ConnectionState::Disconnected) {
        log_warn("Disconnecting existing {} connection before connecting to serial", 
                 type_ == ConnectionType::Serial ? "serial" : "network");
        disconnect();
    }
    
    log_info("Attempting to connect to serial port: {} at {} baud", port, baud_rate);
    
    try {
        auto serial = std::make_unique<SerialConnection>();
        
        state_ = ConnectionState::Connecting;
        type_ = ConnectionType::Serial;
        device_info_ = port + " @ " + std::to_string(baud_rate) + " baud";
        
        if (serial->open(port, baud_rate)) {
            // Assignment to variant automatically destroys old connection
            connection_ = std::move(serial);
            state_ = ConnectionState::Connected;
            error_message_.clear();
            log_info("Successfully connected to serial port");
            return true;
        } else {
            state_ = ConnectionState::Error;
            type_ = ConnectionType::None;
            error_message_ = "Failed to open serial port";
            log_error("Failed to open serial port: {}", port);
            return false;
        }
        
    } catch (const std::exception& e) {
        state_ = ConnectionState::Error;
        type_ = ConnectionType::None;
        error_message_ = std::string("Exception: ") + e.what();
        log_error("Exception connecting to serial:  {}", e.what());
        return false;
    }
}

bool ConnectionManager::connectNetwork(const std::string& host, int port) {
    // IMPORTANT: Disconnect any existing connection first
    if (state_ != ConnectionState::Disconnected) {
        log_warn("Disconnecting existing {} connection before connecting to network", 
                 type_ == ConnectionType::Serial ? "serial" : "network");
        disconnect();
    }
    
    log_info("Attempting to connect to network:  {}:{}", host, port);
    
    try {
        auto network = std::make_unique<NetworkConnection>();
        
        state_ = ConnectionState::Connecting;
        type_ = ConnectionType::Network;
        device_info_ = host + ":" + std::to_string(port);
        
        if (network->connect(host, port)) {
            // Assignment to variant automatically destroys old connection
            connection_ = std::move(network);
            state_ = ConnectionState::Connected;
            error_message_.clear();
            log_info("Successfully connected to network");
            return true;
        } else {
            state_ = ConnectionState::Error;
            type_ = ConnectionType::None;
            error_message_ = "Failed to connect to network";
            log_error("Failed to connect to network: {}:{}", host, port);
            return false;
        }
        
    } catch (const std::exception& e) {
        state_ = ConnectionState::Error;
        type_ = ConnectionType::None;
        error_message_ = std::string("Exception: ") + e.what();
        log_error("Exception connecting to network:  {}", e.what());
        return false;
    }
}

void ConnectionManager::disconnect() {
    if (state_ != ConnectionState::Disconnected) {
        log_info("Disconnecting from {} device:  {}", 
                 type_ == ConnectionType::Serial ? "serial" : "network",
                 device_info_);
        
        // Setting variant to monostate automatically: 
        // 1. Calls destructor of current connection
        // 2. Cleans up resources (closes ports, sockets, etc.)
        connection_ = std::monostate{};
        
        state_ = ConnectionState::Disconnected;
        type_ = ConnectionType::None;
        device_info_.clear();
        error_message_.clear();
    }
}

bool ConnectionManager::write(const std::vector<uint8_t>& data) {
    if (! isConnected()) {
        log_warn("Attempted to write while not connected");
        return false;
    }
    
    // Use std::visit for type-safe variant access
    return std::visit([&data](auto&& conn) -> bool {
        using T = std::decay_t<decltype(conn)>;
        
        if constexpr (std::is_same_v<T, std::monostate>) {
            // No connection
            return false;
        } else if constexpr (std::is_same_v<T, std::unique_ptr<SerialConnection>>) {
            // Serial connection
            return conn->write(data);
        } else if constexpr (std::is_same_v<T, std::unique_ptr<NetworkConnection>>) {
            // Network connection
            return conn->write(data);
        }
        
        return false;
    }, connection_);
}

bool ConnectionManager::write(const std::string& data) {
    std::vector<uint8_t> bytes(data.begin(), data.end());
    return write(bytes);
}

SerialConnection* ConnectionManager::getSerialConnection() {
    if (type_ != ConnectionType::Serial) {
        return nullptr;
    }
    
    if (auto* serial = std::get_if<std::unique_ptr<SerialConnection>>(&connection_)) {
        return serial->get();
    }
    
    return nullptr;
}

NetworkConnection* ConnectionManager::getNetworkConnection() {
    if (type_ != ConnectionType::Network) {
        return nullptr;
    }
    
    if (auto* network = std::get_if<std::unique_ptr<NetworkConnection>>(&connection_)) {
        return network->get();
    }
    
    return nullptr;
}

std::string ConnectionManager::getStatusString() const {
    switch (state_) {
        case ConnectionState::Disconnected: 
            return "Disconnected";
        case ConnectionState::Connecting: 
            return "Connecting to " + device_info_ + "...";
        case ConnectionState::Connected:
            return "Connected to " + device_info_;
        case ConnectionState::Error:
            return "Error: " + error_message_;
        default:
            return "Unknown";
    }
}
