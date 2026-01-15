#ifndef DEFAULT_VIEW_H
#define DEFAULT_VIEW_H

#include "rendering/views.hpp"
#include "imgui.h"

namespace Rendering {
    /**
     * Default view that shows a simple window with text
     * This serves as a template for creating new views
     */
    class DefaultView : public View {
    public:
        DefaultView();
        ~DefaultView() override = default;
        
        void render() override;
    };
}

#endif // DEFAULT_VIEW_H
