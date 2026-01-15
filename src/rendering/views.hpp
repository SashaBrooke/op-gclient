#ifndef VIEWS_H
#define VIEWS_H

#include <memory>
#include <vector>

namespace Rendering {
    /**
     * Base class for all views in the application
     * Views represent different screens/pages in your application
     */
    class View {
    public:
        View() = default;
        virtual ~View() = default;
        
        /**
         * Main rendering function that should be overridden by derived views
         * This is called every frame to render the view's UI
         */
        virtual void render() = 0;
    };
}

#endif // VIEWS_H
