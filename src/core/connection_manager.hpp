#ifndef CONNECTION_MANAGER_HPP
#define CONNECTION_MANAGER_HPP

#include <string>
#include <memory>
#include <variant>
#include <vector>
#include <cstdint>
#include "core/serial_connection.hpp"
#include "core/network_connection.hpp"

/**
 * Singleton to manage external device connection state.
 * Only ONE connection (Serial OR Network) can be active at a time.
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
    
private:
    // Variant enforces:  EITHER Serial OR Network OR Nothing (monostate)
    using Connection = std::variant<
        std::monostate,                      // No connection
        std::unique_ptr<SerialConnection>,   // OR Serial
        std::unique_ptr<NetworkConnection>   // OR Network
    >;
    
    ConnectionState state_;
    ConnectionType type_;
    std::string device_info_;
    std::string error_message_;
    Connection connection_;  // Can only hold ONE type at a time
    
    ConnectionManager() 
        : state_(ConnectionState::Disconnected)
        , type_(ConnectionType::None)
        , device_info_("")
        , error_message_("")
        , connection_(std::monostate{}) {}
    
public:
    static ConnectionManager& getInstance() {
        static ConnectionManager instance;
        return instance;
    }
    
    ConnectionManager(const ConnectionManager&) = delete;
    void operator=(const ConnectionManager&) = delete;
    
    // State queries
    bool isConnected() const { return state_ == ConnectionState::Connected; }
    bool isConnecting() const { return state_ == ConnectionState::Connecting; }
    bool isDisconnected() const { return state_ == ConnectionState::Disconnected; }
    bool hasError() const { return state_ == ConnectionState::Error; }
    
    ConnectionState getState() const { return state_; }
    ConnectionType getType() const { return type_; }
    const std::string& getDeviceInfo() const { return device_info_; }
    const std::string& getErrorMessage() const { return error_message_; }
    
    // Connection management - automatically disconnects existing connection
    bool connectSerial(const std::string& port, int baud_rate);
    bool connectNetwork(const std::string& host, int port);
    void disconnect();
    
    // Generic communication (works for either connection type)
    bool write(const std::vector<uint8_t>& data);
    bool write(const std::string& data);
    
    // Type-safe accessors (returns nullptr if wrong type)
    SerialConnection* getSerialConnection();
    NetworkConnection* getNetworkConnection();
    
    // Helper to get status string
    std::string getStatusString() const;
};

#endif // CONNECTION_MANAGER_HPP
