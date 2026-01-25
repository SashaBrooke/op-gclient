#ifndef GIMBAL_CONTROL_VIEW_HPP
#define GIMBAL_CONTROL_VIEW_HPP

#include <vector>
#include <string>
#include "rendering/views.hpp"
#include "core/gimbal_state.hpp"
#include "core/communication_backend.hpp"

namespace Rendering {

class GimbalControlView : public View {
public:
    GimbalControlView(GimbalState& gimbal_state, CommunicationBackend& comm_backend);
    void render() override;

private:
    void refreshSerialPorts();

    GimbalState& gimbal_state_;
    CommunicationBackend& comm_backend_;
    
    // Serial port dropdown state
    std::vector<std::string> available_serial_ports_;
    size_t selected_port_index_ = 0;
};

} // namespace Rendering

#endif // GIMBAL_CONTROL_VIEW_HPP
