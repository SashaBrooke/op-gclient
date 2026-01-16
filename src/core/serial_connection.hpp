#ifndef SERIAL_CONNECTION_HPP
#define SERIAL_CONNECTION_HPP

#include <string>
#include <vector>
#include <cstdint>

/**
 * Serial port connection wrapper
 * Handles communication over RS-232/USB serial ports
 */
class SerialConnection {
public:  
    SerialConnection();
    ~SerialConnection();
    
    // Prevent copying
    SerialConnection(const SerialConnection&) = delete;
    SerialConnection& operator=(const SerialConnection&) = delete;
    
    // Allow moving (if needed later)
    SerialConnection(SerialConnection&&) = default;
    SerialConnection& operator=(SerialConnection&&) = default;
    
    /**
     * Open a serial port
     * @param port Device path (e.g., "/dev/ttyUSB0" or "COM3")
     * @param baud_rate Baud rate (e.g., 9600, 115200)
     * @return true if successful
     */
    bool open(const std::string& port, int baud_rate);
    
    /**
     * Close the serial port
     */
    void close();
    
    /**
     * Check if port is open
     */
    bool isOpen() const;
    
    /**
     * Write data to serial port
     * @param data Bytes to write
     * @return true if successful
     */
    bool write(const std::vector<uint8_t>& data);
    
    /**
     * Write string to serial port
     * @param data String to write
     * @return true if successful
     */
    bool write(const std::string& data);
    
    /**
     * Read available data from serial port
     * @param buffer Buffer to read into
     * @param max_bytes Maximum bytes to read
     * @return Number of bytes actually read
     */
    size_t read(std::vector<uint8_t>& buffer, size_t max_bytes);
    
    /**
     * Get the port name
     */
    std::string getPort() const;
    
    /**
     * Get the baud rate
     */
    int getBaudRate() const;
    
private:
    bool is_open_;
    std::string port_;
    int baud_rate_;
    
    // TODO: Add actual serial implementation
    // - boost::asio::serial_port
    // - or platform-specific file descriptor (Linux)
    // - or HANDLE (Windows)
};

#endif // SERIAL_CONNECTION_HPP
