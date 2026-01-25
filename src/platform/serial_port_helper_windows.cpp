#include "platform/serial_port_helper.hpp"
#include "util/logging.hpp"
#include <boost/asio/io_context.hpp>
#include <boost/asio/serial_port.hpp>
#include <sstream>

std::vector<std::string> SerialPortHelper::getAvailablePorts() {
    std::vector<std::string> ports;
    const int MAX_PORTS = 256;  // Windows can have many COM ports
    
    // Check COM1 through COM256
    for (int i = 1; i <= MAX_PORTS; ++i) {
        std::stringstream device_name;
        device_name << "COM" << i;
        
        try {
            boost::asio::io_context io_context;
            boost::asio::serial_port serial(io_context, device_name.str());
            ports.push_back(device_name.str());
            serial.close();
            log_debug("Found serial port: {}", device_name.str());
        } catch (const boost::system::system_error&) {
            // Port doesn't exist or can't be opened
        }
    }
    
    if (ports.empty()) {
        log_warn("No serial ports found");
    } else {
        log_info("Found {} serial port(s)", ports.size());
    }
    
    return ports;
}
