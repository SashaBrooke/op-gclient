#ifndef GIMBAL_CONTROL_VIEW_HPP
#define GIMBAL_CONTROL_VIEW_HPP

#include "rendering/views.hpp"
#include <vector>
#include <string>

namespace Rendering {

class GimbalControlView : public View {
public:
    GimbalControlView();
    void render() override;

private:
    void refreshSerialPorts();
    
    // Serial port dropdown state
    std::vector<std::string> available_serial_ports_;
    size_t selected_port_index_ = 0;
};

} // namespace Rendering

#endif // GIMBAL_CONTROL_VIEW_HPP
