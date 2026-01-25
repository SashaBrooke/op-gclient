#ifndef TRANSPORT_INTERFACE_HPP
#define TRANSPORT_INTERFACE_HPP

#include <vector>
#include <functional>
#include <cstdint>
#include <string>

namespace boost {
namespace asio {
    class io_context;
}
}

/**
 * Abstract transport interface
 * Serial and Network both implement this
 */
class ITransport {
public:
    using PacketReceivedCallback = std::function<void(const std::vector<uint8_t>&)>;
    
    virtual ~ITransport() = default;
    
    /**
     * Open/connect the transport
     */
    virtual bool open() = 0;
    
    /**
     * Close/disconnect the transport
     */
    virtual void close() = 0;
    
    /**
     * Check if transport is open/connected
     */
    virtual bool isOpen() const = 0;
    
    /**
     * Write raw bytes asynchronously
     */
    virtual void writeAsync(const std::vector<uint8_t>& data) = 0;
    
    /**
     * Set callback for received packets
     */
    virtual void setPacketReceivedCallback(PacketReceivedCallback callback) = 0;
    
    /**
     * Get the io_context for this transport (for timers, etc.)
     */
    virtual boost::asio::io_context& getIoContext() = 0;
    
    /**
     * Get human-readable connection info
     */
    virtual std::string getConnectionInfo() const = 0;
};

#endif // TRANSPORT_INTERFACE_HPP
