# OhPossum Gimbal Client (OP-GClient)

PC application used to control the OhPossum system.

## Features

- GLFW for window management
- OpenGL 3.3+ for rendering
- ImGui for the user interface
- View-based architecture for easy UI organization
- Docking and multi-viewport support

## Building

### Prerequisites

- CMake 3.12 or higher
- C++17 compatible compiler

### Steps

1. Ensure submodules are pulled in:
```bash
git submodule update --init --recursive
```

2. Generate gl3w loader (one-time setup):
```bash
cd ext/gl3w
python3 gl3w_gen.py
cd ../..
```

3. Build the project:
```bash
mkdir build
cd build
cmake .. 
cmake --build . 
```

4. Run the application:
```bash
./op-gclient
```

## Project Structure

```
src/
├── main.cpp                      # Application entry point
├── application.hpp/cpp           # Main application class
├── rendering/
│   ├── views.hpp/cpp             # Base view class
│   └── views/
│       └── default_view.hpp/cpp  # Default view implementation
└── util/
    └── rect.hpp                  # Utility structures
```

## Creating New Views

To create a new view: 

1. Create a new header file in `src/rendering/views/`:
```cpp
#ifndef MY_VIEW_H
#define MY_VIEW_H

#include "rendering/views.hpp"

namespace Rendering {
    class MyView : public View {
    public:
        MyView();
        ~MyView() override = default;
        void render() override;
    };
}

#endif
```

2. Implement the view in a corresponding `.cpp` file:
```cpp
#include "rendering/views/my_view.hpp"

namespace Rendering {
    MyView::MyView() {
        // Initialize view
    }
    
    void MyView::render() {
        ImGui::Begin("My View");
        // Add your ImGui code here
        ImGui::End();
    }
}
```

3. Add the source file to `CMakeLists.txt`
4. Switch to your view using `app.setView(std::make_shared<Rendering:: MyView>())`
