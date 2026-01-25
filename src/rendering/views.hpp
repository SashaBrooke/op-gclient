#ifndef VIEWS_HPP
#define VIEWS_HPP

#include <memory>
#include "core/gimbal_state.hpp"
#include "core/communication_backend.hpp"

namespace Rendering {

// Forward declaration
class View;

class View {
public: 
    virtual ~View() = default;
    virtual void render() = 0;
};

class ViewManager {
public:
    ViewManager(GimbalState& gimbal_state, CommunicationBackend& comm_backend);
    ~ViewManager();
    void render();
    void setView(std::shared_ptr<View> view);
    
private:
    std::shared_ptr<View> current_view_;
    GimbalState& gimbal_state_;
    CommunicationBackend& comm_backend_;
};

} // namespace Rendering

#endif // VIEWS_HPP
