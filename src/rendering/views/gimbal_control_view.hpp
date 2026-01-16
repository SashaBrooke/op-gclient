#ifndef GIMBAL_CONTROL_VIEW_HPP
#define GIMBAL_CONTROL_VIEW_HPP

#include "rendering/views.hpp"

namespace Rendering {

class GimbalControlView : public View {
public:
    GimbalControlView();
    void render() override;
};

} // namespace Rendering

#endif // GIMBAL_CONTROL_VIEW_HPP
