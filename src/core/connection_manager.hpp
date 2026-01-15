#ifndef CONNECTION_MANAGER_HPP
#define CONNECTION_MANAGER_HPP

#include <string>
#include <functional>

/**
 * Singleton to manage external device connection state
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
    ConnectionState state_;
    ConnectionType type_;
    std::string device_info_;  // Port name or IP address
    std::string error_message_;
    
    ConnectionManager() 
        : state_(ConnectionState:: Disconnected)
        , type_(ConnectionType:: None)
        , device_info_("")
        , error_message_("") {}
    
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
    bool isDisconnected() const { return state_ == ConnectionState:: Disconnected; }
    bool hasError() const { return state_ == ConnectionState::Error; }
    
    ConnectionState getState() const { return state_; }
    ConnectionType getType() const { return type_; }
    const std::string& getDeviceInfo() const { return device_info_; }
    const std::string& getErrorMessage() const { return error_message_; }
    
    // State changes
    void setConnecting(ConnectionType type, const std::string& device) {
        state_ = ConnectionState::Connecting;
        type_ = type;
        device_info_ = device;
        error_message_. clear();
    }
    
    void setConnected() {
        state_ = ConnectionState::Connected;
        error_message_.clear();
    }
    
    void setDisconnected() {
        state_ = ConnectionState::Disconnected;
        type_ = ConnectionType::None;
        device_info_.clear();
        error_message_.clear();
    }
    
    void setError(const std::string& error) {
        state_ = ConnectionState::Error;
        error_message_ = error;
    }
    
    // Helper to get status string
    std::string getStatusString() const {
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
};

#endif // CONNECTION_MANAGER_HPP
