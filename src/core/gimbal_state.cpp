#include "core/gimbal_state.hpp"

GimbalState::Attitude GimbalState::getAttitude() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return attitude_;
}

GimbalState::Velocity GimbalState::getVelocity() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return velocity_;
}

GimbalState::Mode GimbalState::getMode() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return mode_;
}

GimbalState::Health GimbalState::getHealth() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return health_;
}

std::chrono::steady_clock::time_point GimbalState::getLastUpdateTime() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return last_update_time_;
}

void GimbalState::setAttitude(const Attitude& attitude) {
    std::lock_guard<std::mutex> lock(mutex_);
    attitude_ = attitude;
    last_update_time_ = std::chrono::steady_clock::now();
}

void GimbalState::setVelocity(const Velocity& velocity) {
    std::lock_guard<std::mutex> lock(mutex_);
    velocity_ = velocity;
    last_update_time_ = std::chrono::steady_clock::now();
}

void GimbalState::setMode(const Mode& mode) {
    std::lock_guard<std::mutex> lock(mutex_);
    mode_ = mode;
    last_update_time_ = std::chrono::steady_clock::now();
}

void GimbalState::setHealth(const Health& health) {
    std::lock_guard<std::mutex> lock(mutex_);
    health_ = health;
    last_update_time_ = std::chrono::steady_clock::now();
}

void GimbalState::updateAll(const Attitude& att, const Velocity& vel, const Mode& mode) {
    std::lock_guard<std::mutex> lock(mutex_);
    attitude_ = att;
    velocity_ = vel;
    mode_ = mode;
    last_update_time_ = std::chrono::steady_clock::now();
}

bool GimbalState::isStale(std::chrono::milliseconds timeout_ms) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_update_time_);
    return elapsed > timeout_ms;
}

void GimbalState::reset() {
    std::lock_guard<std::mutex> lock(mutex_);
    attitude_ = Attitude{};
    velocity_ = Velocity{};
    mode_ = Mode{};
    health_ = Health{};
    last_update_time_ = std::chrono::steady_clock::time_point{};
}
