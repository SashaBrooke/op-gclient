#ifndef EVENTS_HPP
#define EVENTS_HPP

#include <string>
#include <functional>
#include <unordered_map>
#include <vector>
#include <memory>
#include <queue>
#include <mutex>

// Forward declaration
namespace Rendering {
    class View;
}

namespace Events {

class Event {
public:  
    explicit Event(std::string type) : type_(std::move(type)) {}
    virtual ~Event() = default;
    
    const std::string& getType() const { return type_; }
    
private: 
    std::string type_;
};

class SetViewEvent :  public Event {
public:
    explicit SetViewEvent(std::shared_ptr<Rendering::View> view) 
        : Event("views/set_view")
        , view_(std::move(view)) {}
    
    std::shared_ptr<Rendering::View> getView() const { return view_; }

private:
    std::shared_ptr<Rendering::View> view_;
};

using EventCallback = std::function<void(const Event&)>;
using Event_ptr = std::unique_ptr<Event>;

class EventQueue {
public:
    static EventQueue& getInstance() {
        static EventQueue instance;
        return instance;
    }
    
    // Subscribe to an event type
    void subscribe(const std::string& event_type, EventCallback callback);
    
    // Post an event to the queue (deferred execution)
    void post(Event_ptr event);
    
    // Process all queued events (call once per frame)
    void pollEvents();
    
private:
    EventQueue() = default;
    ~EventQueue() = default;
    EventQueue(const EventQueue&) = delete;
    EventQueue& operator=(const EventQueue&) = delete;
    
    std::unordered_map<std::string, std::vector<EventCallback>> subscribers_;
    std::queue<Event_ptr> event_queue_;
    std::mutex queue_mutex_;
};

} // namespace Events

#endif // EVENTS_HPP
