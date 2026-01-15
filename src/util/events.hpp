#ifndef EVENTS_H
#define EVENTS_H

#include <string>
#include <memory>
#include <functional>
#include <queue>
#include <vector>
#include <set>
#include <mutex>
#include <chrono>

/**
 * Base Event class
 */
class Event {
protected:
    std::string name_;
    std::chrono::system_clock::time_point time_;
    
public:
    explicit Event(std::string name) 
        : name_(std::move(name))
        , time_(std::chrono::system_clock::now()) {}
    
    virtual ~Event() = default;
    
    const std::string& getName() const { return name_; }
    const std::chrono::system_clock::time_point& getTime() const { return time_; }
};

typedef std::shared_ptr<Event> Event_ptr;

/**
 * Listener struct
 */
struct Listener {
    std::string filter;  // Event name pattern (supports wildcard *)
    std::function<void(Event_ptr&)> callback;
};

/**
 * Thread-safe event queue singleton
 */
class EventQueue {
private:
    std::queue<Event_ptr> event_queue_;
    std::set<Listener*> listeners_;
    std::recursive_mutex event_mutex_;
    std::recursive_mutex listeners_mutex_;
    
    EventQueue() = default;
    
    bool isListener(const std::string& filter, const std::string& event_name);
    
public:
    static EventQueue& getInstance() {
        static EventQueue instance;
        return instance;
    }
    
    EventQueue(const EventQueue&) = delete;
    void operator=(const EventQueue&) = delete;
    
    void subscribe(Listener* listener);
    void unsubscribe(Listener* listener);
    void post(Event_ptr event);
    void pollEvents();
};

#endif // EVENTS_H
