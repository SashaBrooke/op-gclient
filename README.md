# OhPossum Gimbal Client (op-gclient)

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
│       └── xxx_view.hpp/cpp      # Place view implementations here
└── util/
    └── xxx_util.hpp/cpp          # Place utilities here
```
