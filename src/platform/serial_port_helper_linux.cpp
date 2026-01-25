#include "platform/serial_port_helper.hpp"
#include "util/logging.hpp"
#include <boost/asio/io_context.hpp>
#include <boost/asio/serial_port.hpp>
#include <sstream>

std::vector<std::string> SerialPortHelper::getAvailablePorts() {
    std::vector<std::string> ports;
    const int MAX_PORTS = 32;
    
    // Check /dev/ttyUSB* devices (USB-to-serial adapters)
    for (int i = 0; i < MAX_PORTS; ++i) {
        std::stringstream device_name;
        device_name << "/dev/ttyUSB" << i;
        
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
    
    // Check /dev/ttyACM* devices (USB CDC ACM devices, e.g., Arduino)
    for (int i = 0; i < MAX_PORTS; ++i) {
        std::stringstream device_name;
        device_name << "/dev/ttyACM" << i;
        
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
