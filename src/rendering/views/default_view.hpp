#ifndef DEFAULT_VIEW_H
#define DEFAULT_VIEW_H

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

#endif // DEFAULT_VIEW_H
