#include <SDL.h>
#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <unordered_map>
#include <typeindex>
#include <typeinfo>
#include <algorithm>
#include <cmath>

// ============================================================
// MINI C++ GAME ENGINE
// Features:
// - Window creation
// - Game loop
// - Delta time
// - Input
// - Entity/Component system
// - Scene management
// - Basic 2D rendering
// ============================================================

namespace Engine
{
    // --------------------------------------------------------
    // Utility
    // --------------------------------------------------------
    struct Vec2
    {
        float x = 0.0f;
        float y = 0.0f;

        Vec2() = default;
        Vec2(float x_, float y_) : x(x_), y(y_) {}

        Vec2 operator+(const Vec2& other) const { return {x + other.x, y + other.y}; }
        Vec2 operator-(const Vec2& other) const { return {x - other.x, y - other.y}; }
        Vec2 operator*(float s) const { return {x * s, y * s}; }

        Vec2& operator+=(const Vec2& other) { x += other.x; y += other.y; return *this; }
        Vec2& operator-=(const Vec2& other) { x -= other.x; y -= other.y; return *this; }
        Vec2& operator*=(float s) { x *= s; y *= s; return *this; }
    };

    struct Color
    {
        Uint8 r = 255, g = 255, b = 255, a = 255;
    };

    // --------------------------------------------------------
    // Forward declarations
    // --------------------------------------------------------
    class Entity;
    class Scene;
    class EngineCore;

    // --------------------------------------------------------
    // Input
    // --------------------------------------------------------
    class Input
    {
    public:
        void BeginFrame()
        {
            mouseWheel = 0;
            quitRequested = false;
            textInput.clear();
        }

        void HandleEvent(const SDL_Event& e)
        {
            if (e.type == SDL_QUIT)
                quitRequested = true;

            if (e.type == SDL_KEYDOWN && !e.key.repeat)
                keys[e.key.keysym.scancode] = true;

            if (e.type == SDL_KEYUP && !e.key.repeat)
                keys[e.key.keysym.scancode] = false;

            if (e.type == SDL_MOUSEBUTTONDOWN)
                mouseButtons[e.button.button] = true;

            if (e.type == SDL_MOUSEBUTTONUP)
                mouseButtons[e.button.button] = false;

            if (e.type == SDL_MOUSEWHEEL)
                mouseWheel = e.wheel.y;

            if (e.type == SDL_TEXTINPUT)
                textInput += e.text.text;
        }

        bool IsKeyDown(SDL_Scancode scancode) const
        {
            auto it = keys.find(scancode);
            return it != keys.end() && it->second;
        }

        bool IsMouseDown(Uint8 button) const
        {
            auto it = mouseButtons.find(button);
            return it != mouseButtons.end() && it->second;
        }

        bool QuitRequested() const { return quitRequested; }
        int MouseWheel() const { return mouseWheel; }
        const std::string& TextInput() const { return textInput; }

        void GetMousePosition(int& x, int& y) const
        {
            SDL_GetMouseState(&x, &y);
        }

    private:
        std::unordered_map<SDL_Scancode, bool> keys;
        std::unordered_map<Uint8, bool> mouseButtons;
        bool quitRequested = false;
        int mouseWheel = 0;
        std::string textInput;
    };

    // --------------------------------------------------------
    // Time
    // --------------------------------------------------------
    class Time
    {
    public:
        void Tick()
        {
            Uint64 now = SDL_GetPerformanceCounter();
            if (lastCounter == 0)
            {
                lastCounter = now;
                deltaTime = 0.0f;
                return;
            }

            Uint64 freq = SDL_GetPerformanceFrequency();
            deltaTime = static_cast<float>(now - lastCounter) / static_cast<float>(freq);
            lastCounter = now;

            elapsedTime += deltaTime;
        }

        float DeltaTime() const { return deltaTime; }
        float ElapsedTime() const { return elapsedTime; }

    private:
        Uint64 lastCounter = 0;
        float deltaTime = 0.0f;
        float elapsedTime = 0.0f;
    };

    // --------------------------------------------------------
    // Component System
    // --------------------------------------------------------
    class Component
    {
    public:
        virtual ~Component() = default;

        virtual void OnCreate(Entity& owner) { (void)owner; }
        virtual void OnUpdate(Entity& owner, float dt) { (void)owner; (void)dt; }
        virtual void OnRender(Entity& owner, SDL_Renderer* renderer) { (void)owner; (void)renderer; }
    };

    // --------------------------------------------------------
    // Entity
    // --------------------------------------------------------
    class Entity
    {
    public:
        Entity() = default;
        explicit Entity(std::string name_) : name(std::move(name_)) {}

        const std::string& GetName() const { return name; }
        void SetName(const std::string& n) { name = n; }

        Vec2 position{0.0f, 0.0f};
        Vec2 scale{1.0f, 1.0f};
        float rotation = 0.0f; // degrees
        bool active = true;

        template<typename T, typename... Args>
        T& AddComponent(Args&&... args)
        {
            static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");

            auto comp = std::make_unique<T>(std::forward<Args>(args)...);
            T& ref = *comp;
            components.push_back(std::move(comp));
            ref.OnCreate(*this);
            return ref;
        }

        template<typename T>
        T* GetComponent()
        {
            for (auto& c : components)
            {
                if (auto ptr = dynamic_cast<T*>(c.get()))
                    return ptr;
            }
            return nullptr;
        }

        void Update(float dt)
        {
            if (!active) return;
            for (auto& c : components)
                c->OnUpdate(*this, dt);
        }

        void Render(SDL_Renderer* renderer)
        {
            if (!active) return;
            for (auto& c : components)
                c->OnRender(*this, renderer);
        }

    private:
        std::string name = "Entity";
        std::vector<std::unique_ptr<Component>> components;
    };

    // --------------------------------------------------------
    // Simple Components
    // --------------------------------------------------------
    class SpriteComponent : public Component
    {
    public:
        SpriteComponent(int w, int h, Color c)
            : width(w), height(h), color(c) {}

        void OnRender(Entity& owner, SDL_Renderer* renderer) override
        {
            SDL_Rect rect;
            rect.x = static_cast<int>(owner.position.x);
            rect.y = static_cast<int>(owner.position.y);
            rect.w = static_cast<int>(width * owner.scale.x);
            rect.h = static_cast<int>(height * owner.scale.y);

            SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
            SDL_RenderFillRect(renderer, &rect);
        }

        int width;
        int height;
        Color color;
    };

    class BoxColliderComponent : public Component
    {
    public:
        BoxColliderComponent(int w, int h)
            : width(w), height(h) {}

        SDL_Rect GetRect(const Entity& owner) const
        {
            SDL_Rect rect;
            rect.x = static_cast<int>(owner.position.x);
            rect.y = static_cast<int>(owner.position.y);
            rect.w = static_cast<int>(width * owner.scale.x);
            rect.h = static_cast<int>(height * owner.scale.y);
            return rect;
        }

        bool Intersects(const Entity& owner, const BoxColliderComponent& other, const Entity& otherOwner) const
        {
            SDL_Rect a = GetRect(owner);
            SDL_Rect b = other.GetRect(otherOwner);
            return SDL_HasIntersection(&a, &b);
        }

        int width;
        int height;
    };

    class PhysicsBody2D : public Component
    {
    public:
        Vec2 velocity{0.0f, 0.0f};
        Vec2 acceleration{0.0f, 0.0f};
        float gravity = 0.0f;
        float damping = 0.98f;
        bool useGravity = false;

        void OnUpdate(Entity& owner, float dt) override
        {
            if (useGravity)
                acceleration.y += gravity;

            velocity += acceleration * dt;
            owner.position += velocity * dt;
            velocity *= damping;

            acceleration = {0.0f, 0.0f};
        }
    };

    class PlayerController2D : public Component
    {
    public:
        float speed = 220.0f;

        PlayerController2D(Input* input_) : input(input_) {}

        void OnUpdate(Entity& owner, float dt) override
        {
            if (!input) return;

            Vec2 move{0.0f, 0.0f};

            if (input->IsKeyDown(SDL_SCANCODE_W)) move.y -= 1.0f;
            if (input->IsKeyDown(SDL_SCANCODE_S)) move.y += 1.0f;
            if (input->IsKeyDown(SDL_SCANCODE_A)) move.x -= 1.0f;
            if (input->IsKeyDown(SDL_SCANCODE_D)) move.x += 1.0f;

            float len = std::sqrt(move.x * move.x + move.y * move.y);
            if (len > 0.0f)
            {
                move.x /= len;
                move.y /= len;
            }

            owner.position += move * speed * dt;
        }

    private:
        Input* input = nullptr;
    };

    // --------------------------------------------------------
    // Scene
    // --------------------------------------------------------
    class Scene
    {
    public:
        virtual ~Scene() = default;

        virtual void OnCreate(EngineCore& engine) { (void)engine; }
        virtual void OnUpdate(EngineCore& engine, float dt) { (void)engine; (void)dt; }
        virtual void OnRender(EngineCore& engine, SDL_Renderer* renderer) { (void)engine; (void)renderer; }

        Entity& CreateEntity(const std::string& name)
        {
            entities.push_back(std::make_unique<Entity>(name));
            return *entities.back();
        }

        std::vector<std::unique_ptr<Entity>>& GetEntities()
        {
            return entities;
        }

    protected:
        std::vector<std::unique_ptr<Entity>> entities;
    };

    // --------------------------------------------------------
    // Example scene
    // --------------------------------------------------------
    class DemoScene : public Scene
    {
    public:
        void OnCreate(EngineCore& engine) override;
        void OnUpdate(EngineCore& engine, float dt) override;
        void OnRender(EngineCore& engine, SDL_Renderer* renderer) override;

    private:
        Entity* player = nullptr;
        Entity* enemy = nullptr;
        Entity* ground = nullptr;
        Entity* movingBox = nullptr;
    };

    // --------------------------------------------------------
    // Engine Core
    // --------------------------------------------------------
    class EngineCore
    {
    public:
        bool Init(const std::string& title, int width, int height)
        {
            if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS) != 0)
            {
                std::cerr << "SDL_Init failed: " << SDL_GetError() << "\n";
                return false;
            }

            window = SDL_CreateWindow(
                title.c_str(),
                SDL_WINDOWPOS_CENTERED,
                SDL_WINDOWPOS_CENTERED,
                width, height,
                SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
            );

            if (!window)
            {
                std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << "\n";
                return false;
            }

            renderer = SDL_CreateRenderer(
                window,
                -1,
                SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
            );

            if (!renderer)
            {
                std::cerr << "SDL_CreateRenderer failed: " << SDL_GetError() << "\n";
                return false;
            }

            width_ = width;
            height_ = height;

            SDL_StartTextInput();
            running = true;
            return true;
        }

        void Shutdown()
        {
            SDL_StopTextInput();

            if (renderer)
            {
                SDL_DestroyRenderer(renderer);
                renderer = nullptr;
            }

            if (window)
            {
                SDL_DestroyWindow(window);
                window = nullptr;
            }

            SDL_Quit();
        }

        void SetScene(std::unique_ptr<Scene> newScene)
        {
            currentScene = std::move(newScene);
            if (currentScene)
                currentScene->OnCreate(*this);
        }

        void Run()
        {
            while (running)
            {
                time.Tick();
                input.BeginFrame();

                SDL_Event e;
                while (SDL_PollEvent(&e))
                {
                    input.HandleEvent(e);

                    if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
                    {
                        width_ = e.window.data1;
                        height_ = e.window.data2;
                    }
                }

                if (input.QuitRequested() || input.IsKeyDown(SDL_SCANCODE_ESCAPE))
                    running = false;

                float dt = time.DeltaTime();

                if (currentScene)
                    currentScene->OnUpdate(*this, dt);

                // Clear screen
                SDL_SetRenderDrawColor(renderer, 18, 18, 22, 255);
                SDL_RenderClear(renderer);

                if (currentScene)
                {
                    currentScene->OnRender(*this, renderer);

                    for (auto& entity : currentScene->GetEntities())
                        entity->Render(renderer);
                }

                SDL_RenderPresent(renderer);
            }
        }

        SDL_Renderer* GetRenderer() { return renderer; }
        Input& GetInput() { return input; }
        Time& GetTime() { return time; }

        int GetWidth() const { return width_; }
        int GetHeight() const { return height_; }

    private:
        SDL_Window* window = nullptr;
        SDL_Renderer* renderer = nullptr;
        bool running = false;
        int width_ = 1280;
        int height_ = 720;

        Input input;
        Time time;
        std::unique_ptr<Scene> currentScene;
    };

    // --------------------------------------------------------
    // DemoScene implementation
    // --------------------------------------------------------
    void DemoScene::OnCreate(EngineCore& engine)
    {
        // Player
        player = &CreateEntity("Player");
        player->position = {100.0f, 100.0f};
        player->AddComponent<SpriteComponent>(48, 48, {60, 200, 120, 255});
        player->AddComponent<PlayerController2D>(&engine.GetInput());

        // Enemy box
        enemy = &CreateEntity("Enemy");
        enemy->position = {500.0f, 250.0f};
        enemy->AddComponent<SpriteComponent>(64, 64, {220, 80, 80, 255});

        // Ground
        ground = &CreateEntity("Ground");
        ground->position = {0.0f, 640.0f};
        ground->scale = {1.0f, 1.0f};
        ground->AddComponent<SpriteComponent>(1280, 80, {70, 70, 90, 255});

        // Moving box
        movingBox = &CreateEntity("MovingBox");
        movingBox->position = {300.0f, 400.0f};
        movingBox->AddComponent<SpriteComponent>(40, 40, {80, 140, 230, 255});
        auto& body = movingBox->AddComponent<PhysicsBody2D>();
        body.velocity = {80.0f, 0.0f};
    }

    void DemoScene::OnUpdate(EngineCore& engine, float dt)
    {
        (void)engine;

        // Simple movement logic for demo box
        if (movingBox)
        {
            auto* body = movingBox->GetComponent<PhysicsBody2D>();
            if (body)
            {
                if (movingBox->position.x < 0.0f || movingBox->position.x > 1240.0f)
                    body->velocity.x = -body->velocity.x;
            }
        }

        // Update all entities
        for (auto& entity : entities)
            entity->Update(dt);
    }

    void DemoScene::OnRender(EngineCore& engine, SDL_Renderer* renderer)
    {
        (void)engine;

        // Draw a simple grid background
        SDL_SetRenderDrawColor(renderer, 30, 30, 36, 255);
        for (int x = 0; x < engine.GetWidth(); x += 40)
            SDL_RenderDrawLine(renderer, x, 0, x, engine.GetHeight());

        for (int y = 0; y < engine.GetHeight(); y += 40)
            SDL_RenderDrawLine(renderer, 0, y, engine.GetWidth(), y);

        // Draw a simple title bar rectangle
        SDL_Rect topBar{0, 0, engine.GetWidth(), 28};
        SDL_SetRenderDrawColor(renderer, 45, 45, 55, 255);
        SDL_RenderFillRect(renderer, &topBar);
    }
} // namespace Engine

// ------------------------------------------------------------
// Entry point
// ------------------------------------------------------------
int main(int argc, char** argv)
{
    (void)argc;
    (void)argv;

    Engine::EngineCore engine;

    if (!engine.Init("My C++ Engine", 1280, 720))
        return 1;

    engine.SetScene(std::make_unique<Engine::DemoScene>());
    engine.Run();
    engine.Shutdown();

    return 0;
}
