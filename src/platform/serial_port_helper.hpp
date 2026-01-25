#ifndef SERIAL_PORT_HELPER_HPP
#define SERIAL_PORT_HELPER_HPP

#include <string>
#include <vector>

/**
 * Platform-specific helper to enumerate available serial ports
 * Works on Linux and Windows
 */
class SerialPortHelper {
public:
    /**
     * Get list of available serial ports on the system
     * @return Vector of port names (e.g., "/dev/ttyUSB0" or "COM3")
     */
    static std::vector<std::string> getAvailablePorts();
};

#endif // SERIAL_PORT_HELPER_HPP
