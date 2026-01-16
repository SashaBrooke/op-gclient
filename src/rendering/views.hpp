#ifndef VIEWS_HPP
#define VIEWS_HPP

#include <memory>

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
    ViewManager();
    void render();
    void setView(std::shared_ptr<View> view);
    
private:
    std::shared_ptr<View> current_view_;
};

} // namespace Rendering

#endif // VIEWS_HPP
