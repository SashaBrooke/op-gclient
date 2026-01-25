#include "core/gimbal_state.hpp"

GimbalState::Position GimbalState::getPosition() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return position_;
}

GimbalState::Setpoint GimbalState::getSetpoint() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return setpoint_;
}

GimbalState::Mode GimbalState::getMode() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return mode_;
}

GimbalState::Limits GimbalState::getLimits() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return limits_;
}

GimbalState::Health GimbalState::getHealth() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return health_;
}

std::chrono::steady_clock::time_point GimbalState::getLastUpdateTime() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return last_update_time_;
}

void GimbalState::setPosition(const Position& position) {
    std::lock_guard<std::mutex> lock(mutex_);
    position_ = position;
    last_update_time_ = std::chrono::steady_clock::now();
}

void GimbalState::setSetpoint(const Setpoint& setpoint) {
    std::lock_guard<std::mutex> lock(mutex_);
    setpoint_ = setpoint;
    last_update_time_ = std::chrono::steady_clock::now();
}

void GimbalState::setMode(Mode mode) {
    std::lock_guard<std::mutex> lock(mutex_);
    mode_ = mode;
    last_update_time_ = std::chrono::steady_clock::now();
}

void GimbalState::setLimits(const Limits& limits) {
    std::lock_guard<std::mutex> lock(mutex_);
    limits_ = limits;
    last_update_time_ = std::chrono::steady_clock::now();
}

void GimbalState::setHealth(const Health& health) {
    std::lock_guard<std::mutex> lock(mutex_);
    health_ = health;
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
    position_ = Position{};
    setpoint_ = Setpoint{};
    mode_ = Mode::Free;
    limits_ = Limits{};
    health_ = Health{};
    last_update_time_ = std::chrono::steady_clock::time_point{};
}
