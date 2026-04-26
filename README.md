# CoreLock Engine

CoreLock Engine is a lightweight C++ game engine built from scratch using SDL2.  
It is designed as a learning-focused but expandable foundation for building 2D games and eventually a full custom engine similar in structure to Unity (but much simpler and low-level).

---

## 🚀 Features

- 🪟 Cross-platform window system (SDL2)
- 🎮 Real-time game loop
- ⏱ Delta time system
- ⌨ Input handling (keyboard + mouse)
- 🧩 Entity / Component architecture
- 🎬 Scene system
- 🎨 Basic 2D rendering (rectangles)
- 🧠 Physics-ready structure (expandable)
- 🔄 Extensible engine core

---

## 🧱 Architecture

CoreLock Engine is built in layers:


Engine Core
├── Window & Renderer (SDL2)
├── Input System
├── Time System
├── Entity System
│ ├── Components
│ ├── Transform
│ └── Game Objects
├── Scene System
└── Game Layer (your game code)


---

## 📦 Requirements

- C++17 or higher
- SDL2 library

### Install SDL2

**Windows (Visual Studio):**
- Download SDL2 development libraries
- Add `include/` and `lib/` to project settings
- Link:
  - `SDL2.lib`
  - `SDL2main.lib`

**Linux:**
```bash
sudo apt install libsdl2-dev
▶️ How to Build
Windows (Visual Studio)
Create a new C++ project
Add main.cpp
Link SDL2
Build & Run
Linux (g++)
g++ main.cpp -o CoreLockEngine -lSDL2
./CoreLockEngine
🎮 Controls (Demo Scene)
W A S D → Move player
ESC → Exit engine
🧠 Example Code (Entity)
Entity& player = scene.CreateEntity("Player");
player.position = {100, 100};
player.AddComponent<SpriteComponent>(48, 48, {0, 255, 0, 255});
player.AddComponent<PlayerController2D>(&engine.GetInput());
🔧 Goals of CoreLock Engine

CoreLock Engine is not meant to replace Unity or Unreal.

Instead, it focuses on:

Learning how game engines work internally
Building systems from scratch
Understanding rendering, input, and ECS design
Creating a foundation for custom game projects
📈 Planned Features
Texture / Sprite rendering (PNG support)
Camera system (2D/3D-ready architecture)
Collision system
Audio system
JSON scene saving/loading
UI system (buttons, menus)
Script system (C++ or Lua integration)
Editor mode (ImGui-based)
💡 Philosophy

"Understand the engine before you use the engine."

CoreLock Engine is built to teach how real engines like Unity work under the hood.

📁 Project Structure
CoreLockEngine/
 ├── src/
 │    ├── main.cpp
 │    ├── Engine/
 │    ├── Scene/
 │    ├── Components/
 │    └── Input/
 ├── external/
 │    └── SDL2/
 ├── README.md
🧑‍💻 Author

Built as a custom C++ game engine project for learning game development fundamentals.

⚠️ Disclaimer

This engine is in early development and is intended for educational and experimental use.
