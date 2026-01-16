#ifndef NETWORK_CONNECTION_HPP
#define NETWORK_CONNECTION_HPP

#include <string>
#include <vector>
#include <cstdint>

/**
 * TCP/IP network connection wrapper
 * Handles communication over network sockets
 */
class NetworkConnection {
public:
    NetworkConnection();
    ~NetworkConnection();
    
    // Prevent copying
    NetworkConnection(const NetworkConnection&) = delete;
    NetworkConnection& operator=(const NetworkConnection&) = delete;
    
    // Allow moving (if needed later)
    NetworkConnection(NetworkConnection&&) = default;
    NetworkConnection& operator=(NetworkConnection&&) = default;
    
    /**
     * Connect to a remote host
     * @param host Hostname or IP address (e.g., "192.168.1.100")
     * @param port Port number (e.g., 3883)
     * @return true if successful
     */
    bool connect(const std::string& host, int port);
    
    /**
     * Disconnect from remote host
     */
    void disconnect();
    
    /**
     * Check if connected
     */
    bool isConnected() const;
    
    /**
     * Write data to network
     * @param data Bytes to write
     * @return true if successful
     */
    bool write(const std::vector<uint8_t>& data);
    
    /**
     * Write string to network
     * @param data String to write
     * @return true if successful
     */
    bool write(const std::string& data);
    
    /**
     * Read available data from network
     * @param buffer Buffer to read into
     * @param max_bytes Maximum bytes to read
     * @return Number of bytes actually read
     */
    size_t read(std::vector<uint8_t>& buffer, size_t max_bytes);
    
    /**
     * Get the host address
     */
    std::string getHost() const;
    
    /**
     * Get the port number
     */
    int getPort() const;
    
private:
    bool is_connected_;
    std::string host_;
    int port_;
    
    // TODO: Add actual network implementation
    // - boost::asio::ip::tcp::socket
    // - or platform-specific socket (POSIX/Windows)
};

#endif // NETWORK_CONNECTION_HPP
