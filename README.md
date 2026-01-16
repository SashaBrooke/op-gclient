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

2. Build the project:
```bash
mkdir build
cd build
cmake .. 
cmake --build . 
```

3. Run the application:
```bash
./op-gclient
```

## Project Structure

```
op-gclient/
├── src/
│   ├── main.cpp                # Application entry point
│   ├── application.hpp/cpp     # Main application class
│   ├── core/                   # Core business logic modules
│   ├── rendering/              # Everything related to what is rendered to the screen
│   │   └── views/              # Different application views (place view implementations here)
│   └── util/                   # Utilities to make the application structure function (not business logic)
│
├── ext/                        # External dependencies
└── docs/                       # Documentation
```
