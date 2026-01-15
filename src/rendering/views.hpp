#ifndef VIEWS_H
#define VIEWS_H

#include <memory>
#include "util/events.hpp"

namespace Rendering {
    /**
     * Base class for all views
     */
    class View {
    public:
        View() = default;
        virtual ~View() = default;
        
        virtual void render() = 0;
    };
    
    /**
     * Event for switching views
     */
    class SetViewEvent : public Event {
    private:
        std::shared_ptr<View> view_;
        
    public:
        explicit SetViewEvent(std::shared_ptr<View> view) 
            : Event("views/set_view")
            , view_(std::move(view)) {}
        
        std::shared_ptr<View> getView() { return view_; }
    };
}

#endif // VIEWS_H
