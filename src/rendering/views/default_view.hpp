#ifndef DEFAULT_VIEW_HPP
#define DEFAULT_VIEW_HPP

#include "rendering/views.hpp"
#include "imgui.h"

namespace Rendering {
    class DefaultView : public View {
    public:
        DefaultView();
        ~DefaultView() override = default;
        
        void render() override;
    };
}

#endif // DEFAULT_VIEW_HPP
