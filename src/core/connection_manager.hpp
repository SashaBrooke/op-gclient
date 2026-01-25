#ifndef CONNECTION_MANAGER_HPP
#define CONNECTION_MANAGER_HPP

#include <string>
#include <memory>
#include <vector>
#include <cstdint>
#include "core/communication_backend.hpp"

/**
 * Singleton to manage external device connection state.
 * Only ONE connection (Serial OR Network) can be active at a time.
 * 
 * This is the main interface used by the UI to: 
 * - Connect/disconnect from devices
 * - Query connection state
 * - Send commands via the communication backend
 */
class ConnectionManager {
public:
    enum class ConnectionType {
        None,
        Serial,
        Network
    };
    
    enum class ConnectionState {
        Disconnected,
        Connecting,
        Connected,
        Error
    };
    
    static ConnectionManager& getInstance() {
        static ConnectionManager instance;
        return instance;
    }
    
    // Delete copy/move constructors
    ConnectionManager(const ConnectionManager&) = delete;
    ConnectionManager& operator=(const ConnectionManager&) = delete;
    ConnectionManager(ConnectionManager&&) = delete;
    ConnectionManager& operator=(ConnectionManager&&) = delete;
    
    // ========================================
    // State Queries
    // ========================================
    
    /**
     * Check if currently connected to a device
     */
    bool isConnected() const;
    
    /**
     * Check if connection attempt is in progress
     */
    bool isConnecting() const { return state_ == ConnectionState::Connecting; }
    
    /**
     * Check if disconnected
     */
    bool isDisconnected() const { return state_ == ConnectionState::Disconnected; }
    
    /**
     * Check if there was a connection error
     */
    bool hasError() const { return state_ == ConnectionState::Error; }
    
    /**
     * Get current connection state
     */
    ConnectionState getState() const { return state_; }
    
    /**
     * Get current connection type
     */
    ConnectionType getType() const { return type_; }
    
    /**
     * Get device info string (e.g., "/dev/ttyUSB0 @ 115200 baud")
     */
    const std::string& getDeviceInfo() const { return device_info_; }
    
    /**
     * Get error message if hasError() is true
     */
    const std::string& getErrorMessage() const { return error_message_; }
    
    // ========================================
    // Connection Management
    // ========================================
    
    /**
     * Connect to a serial port
     * @param port Device path (e.g., "/dev/ttyUSB0" or "COM3")
     * @param baud_rate Baud rate (e.g., 115200)
     * @return true if connection successful
     */
    bool connectSerial(const std::string& port, int baud_rate);
    
    /**
     * Connect to a network device (TCP)
     * @param host IP address or hostname
     * @param port TCP port number
     * @return true if connection successful
     */
    bool connectNetwork(const std::string& host, int port);
    
    /**
     * Disconnect from current device
     */
    void disconnect();
    
    // ========================================
    // Communication
    // ========================================
    
    /**
     * Get the communication backend for sending commands
     * Returns nullptr if not connected
     */
    CommunicationBackend* getBackend();
    const CommunicationBackend* getBackend() const;
    
    /**
     * Convenience method:  send a message
     * @param protobuf_data Serialized protobuf message
     * @param ack_callback Optional ACK callback
     * @param timeout_ms ACK timeout
     */
    void sendMessage(const std::vector<uint8_t>& protobuf_data,
                    std::function<void(bool success)> ack_callback = nullptr,
                    uint32_t timeout_ms = 1000);
    
    // ========================================
    // Utility
    // ========================================
    
    /**
     * Get a human-readable status string
     */
    std::string getStatusString() const;
    
private:
    ConnectionManager();
    ~ConnectionManager();
    
    ConnectionState state_;
    ConnectionType type_;
    std::string device_info_;
    std::string error_message_;
    
    std::unique_ptr<CommunicationBackend> backend_;
};

#endif // CONNECTION_MANAGER_HPP
