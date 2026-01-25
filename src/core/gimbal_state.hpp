#ifndef GIMBAL_STATE_HPP
#define GIMBAL_STATE_HPP

#include <mutex>
#include <cstdint>
#include <chrono>
#include <string>

/**
 * Thread-safe shared gimbal state
 * Updated by comms backend, read by UI thread
 */
class GimbalState {
public:
    struct Attitude {
        float pan_deg = 0.0f;
        float tilt_deg = 0.0f;
        float roll_deg = 0.0f;
    };
    
    struct Velocity {
        float pan_deg_s = 0.0f;
        float tilt_deg_s = 0.0f;
        float roll_deg_s = 0.0f;
    };
    
    struct Mode {
        enum class Type {
            Idle,
            Position,
            Velocity,
            Tracking,
            Calibrating
        };
        Type type = Type::Idle;
        bool is_enabled = false;
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
    
    static GimbalState& getInstance() {
        static GimbalState instance;
        return instance;
    }
    
    // Thread-safe getters
    Attitude getAttitude() const;
    Velocity getVelocity() const;
    Mode getMode() const;
    Health getHealth() const;
    std::chrono::steady_clock::time_point getLastUpdateTime() const;
    
    // Thread-safe setters (called by comms backend)
    void setAttitude(const Attitude& attitude);
    void setVelocity(const Velocity& velocity);
    void setMode(const Mode& mode);
    void setHealth(const Health& health);
    
    // Batch update (more efficient)
    void updateAll(const Attitude& att, const Velocity& vel, const Mode& mode);
    
    // Check if data is stale
    bool isStale(std::chrono::milliseconds timeout_ms = std::chrono::milliseconds(500)) const;
    
    // Reset all state
    void reset();
    
private:
    GimbalState() = default;
    GimbalState(const GimbalState&) = delete;
    void operator=(const GimbalState&) = delete;
    
    mutable std::mutex mutex_;
    
    Attitude attitude_;
    Velocity velocity_;
    Mode mode_;
    Health health_;
    std::chrono::steady_clock::time_point last_update_time_;
};

#endif // GIMBAL_STATE_HPP
