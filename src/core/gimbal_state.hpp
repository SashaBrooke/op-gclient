#ifndef GIMBAL_STATE_HPP
#define GIMBAL_STATE_HPP

#include <mutex>
#include <cstdint>
#include <chrono>
#include <string>

/**
 * Thread-safe gimbal state matching op-controls firmware
 */
class GimbalState {
public:
    // Match op-controls gimbal_mode_e exactly
    enum class Mode {
        Free,           // GIMBAL_MODE_FREE
        Armed,          // GIMBAL_MODE_ARMED
        LowerLimit,     // GIMBAL_MODE_LOWER_LIMIT
        UpperLimit      // GIMBAL_MODE_UPPER_LIMIT
    };
    
    struct Position {
        float pan_deg = 0.0f;   // Current pan position
        float tilt_deg = 0.0f;  // Future: tilt axis
    };
    
    struct Setpoint {
        float pan_deg = 0.0f;   // Pan position setpoint
        float tilt_deg = 0.0f;  // Future: tilt setpoint
    };
    
    struct Limits {
        float pan_lower = 0.0f;
        float pan_upper = 0.0f;
        float tilt_lower = 0.0f;  // Future
        float tilt_upper = 0.0f;  // Future
    };
    
    struct Health {
        enum class Status {
            Unknown,
            Healthy,
            Warning,
            Error
        };
        Status status = Status::Unknown;
        std::string message;
        uint32_t error_flags = 0;
    };
    
    GimbalState() = default;
    ~GimbalState() = default;
    
    GimbalState(const GimbalState&) = delete;
    GimbalState& operator=(const GimbalState&) = delete;
    
    // Thread-safe getters
    Position getPosition() const;
    Setpoint getSetpoint() const;
    Mode getMode() const;
    Limits getLimits() const;
    Health getHealth() const;
    std::chrono::steady_clock::time_point getLastUpdateTime() const;
    
    // Thread-safe setters
    void setPosition(const Position& position);
    void setSetpoint(const Setpoint& setpoint);
    void setMode(Mode mode);
    void setLimits(const Limits& limits);
    void setHealth(const Health& health);
    
    bool isStale(std::chrono::milliseconds timeout_ms = std::chrono::milliseconds(500)) const;
    void reset();
    
private:
    mutable std::mutex mutex_;
    
    Position position_;
    Setpoint setpoint_;
    Mode mode_ = Mode::Free;
    Limits limits_;
    Health health_;
    std::chrono::steady_clock::time_point last_update_time_;
};

#endif // GIMBAL_STATE_HPP
