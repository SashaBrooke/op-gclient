#include "rendering/views.hpp"
#include "rendering/views/gimbal_control_view.hpp"
#include "util/logging.hpp"
#include "util/events.hpp"
#include <memory>

namespace Rendering {

ViewManager::ViewManager() {
    log_debug("ViewManager created");
    
    // Set initial view
    setView(std::make_shared<GimbalControlView>());
    
    // Subscribe to view change events
    Events::EventQueue::getInstance().subscribe("views/set_view", [this](const Events::Event& event) {
        if (auto* setViewEvent = dynamic_cast<const Events::SetViewEvent*>(&event)) {
            setView(setViewEvent->getView());
        }
    });
    
    log_debug("View change listener subscribed");
}

void ViewManager::setView(std::shared_ptr<View> view) {
    log_info("Changing view");
    current_view_ = std::move(view);
}

void ViewManager::render() {
    if (current_view_) {
        current_view_->render();
    }
}

} // namespace Rendering
