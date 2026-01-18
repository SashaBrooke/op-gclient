#include "util/events.hpp"
#include "util/logging.hpp"

namespace Events {

void EventQueue::subscribe(const std::string& event_type, EventCallback callback) {
    subscribers_[event_type].push_back(std::move(callback));
    log_debug("Subscribed to event: {}", event_type);
}

void EventQueue::unsubscribe(const std::string& event_type) {
    auto it = subscribers_.find(event_type);
    if (it != subscribers_.end()) {
        subscribers_.erase(it);
        log_debug("Unsubscribed from event: {}", event_type);
    }
}

void EventQueue::clearAll() {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    subscribers_.clear();
    std::queue<Event_ptr> empty;
    event_queue_.swap(empty);
    log_debug("Cleared all event subscriptions and queued events");
}

void EventQueue::post(Event_ptr event) {
    if (!event) {
        log_warn("Attempted to post null event");
        return;
    }
    
    std::lock_guard<std::mutex> lock(queue_mutex_);
    log_debug("Posted event to queue:  {}", event->getType());
    event_queue_.push(std::move(event));
}

void EventQueue::pollEvents() {
    std::queue<Event_ptr> events_to_process;
    
    // Move all events from queue to local queue
    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        events_to_process.swap(event_queue_);
    }
    
    // Process all events
    while (!events_to_process.empty()) {
        auto& event = events_to_process.front();
        const auto& event_type = event->getType();
        
        auto it = subscribers_.find(event_type);
        if (it != subscribers_.end()) {
            log_debug("Processing event: {}", event_type);
            for (const auto& callback : it->second) {
                callback(*event);
            }
        } else {
            log_debug("No subscribers for event: {}", event_type);
        }
        
        events_to_process.pop();
    }
}

} // namespace Events
