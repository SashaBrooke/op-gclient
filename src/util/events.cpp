#include "events.hpp"
#include <algorithm>

void EventQueue::subscribe(Listener* listener) {
    std::lock_guard<std::recursive_mutex> guard(listeners_mutex_);
    if (listeners_.find(listener) == listeners_.end()) {
        listeners_.insert(listener);
    }
}

void EventQueue::unsubscribe(Listener* listener) {
    std::lock_guard<std::recursive_mutex> guard(listeners_mutex_);
    listeners_.erase(listener);
}

void EventQueue::post(Event_ptr event) {
    std::lock_guard<std::recursive_mutex> guard(event_mutex_);
    event_queue_.push(event);
}

void EventQueue::pollEvents() {
    std::lock_guard<std::recursive_mutex> event_guard(event_mutex_);
    
    while (!event_queue_.empty()) {
        Event_ptr event = event_queue_.front();
        
        std::lock_guard<std::recursive_mutex> listener_guard(listeners_mutex_);
        for (const auto& listener : listeners_) {
            if (isListener(listener->filter, event->getName())) {
                listener->callback(event);
            }
        }
        
        event_queue_.pop();
    }
}

bool EventQueue::isListener(const std::string& filter, const std::string& event_name) {
    // Simple wildcard matching:  "views*" matches "views/set_view"
    if (filter.empty()) return false;
    
    // Check for wildcard at end
    size_t wildcard_pos = filter.find('*');
    if (wildcard_pos != std::string::npos) {
        // Match everything before the wildcard
        return event_name.compare(0, wildcard_pos, filter, 0, wildcard_pos) == 0;
    }
    
    // Exact match
    return filter == event_name;
}
