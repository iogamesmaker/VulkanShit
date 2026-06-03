// main.cpp
#if defined(_WIN32) || defined(_WIN64)
#define PLATFORM_WIN32 1
#elif defined(__APPLE__)
#define PLATFORM_MACOS 1
#elif defined(__linux__)
#define PLATFORM_LINUX 1
#endif

#include <iostream>
#include <memory>
#include <stdexcept>
#include <cstring>
#include <SDL3/SDL.h>
#include "shader.hpp"
#include "camera.hpp"
#include "imguishit.hpp"

#include <Graphics/GraphicsEngineVulkan/interface/EngineFactoryVk.h>
#include <Graphics/GraphicsEngine/interface/SwapChain.h>
#include <Common/interface/BasicMath.hpp>

struct ShaderData {
    float width;
    float height;
    float padding2[2];

    float4x4 viewmat;
    float3 campos;
    float time;
    float speed = 3.0;
    int steps = 9;
    float size = 0.3;
    float padding[37];
};


using namespace Diligent;

class Application
{
public:
    Application(uint16_t width = 1024, uint16_t height = 768)
    : m_Width(width), m_Height(height), m_camera(float3(0,0,-3))
    {
        InitWindow();
        InitSwapchain();

        shaderdata.width  = static_cast<float>(m_Width);
        shaderdata.height = static_cast<float>(m_Height);
        shaderdata.campos = {0.0, 0.0, -3.0};

        CreatePipelines();
        imgui = std::make_unique<GUIHandler>(window, m_pDevice, m_pSwapChain->GetDesc().ColorBufferFormat, m_pSwapChain->GetDesc().DepthBufferFormat);
    }

    ~Application()
    {
        if (m_pImmediateContext) m_pImmediateContext->Flush();
        if (window) SDL_DestroyWindow(window);
        SDL_Quit();
    }

    void Run()
    {
        bool quit = false;
        SDL_Event event;

        SDL_SetWindowRelativeMouseMode(window, true);
        Uint64 lastTime = SDL_GetTicks();

        while (!quit)
        {
            Uint64 now = SDL_GetTicks();
            float deltatime = (now - lastTime) / 1000.0f;
            lastTime = now;

            while (SDL_PollEvent(&event))
            {
                imgui->process(&event);
                if (event.type == SDL_EVENT_QUIT)
                {
                    quit = true;
                }
                else if (event.type == SDL_EVENT_KEY_DOWN)
                {
                    if (event.key.key == SDLK_LALT) {
                        m_gui = !m_gui;
                    }
                    if (event.key.key == SDLK_ESCAPE){
                        m_mouselocked = !m_mouselocked;

                        SDL_SetWindowRelativeMouseMode(window, m_mouselocked);
                    } else
                    if (event.key.key == SDLK_F11) {
                        m_fullscreen = !m_fullscreen;
                        SDL_SetWindowFullscreen(window, m_fullscreen);
                    } else
                    if (event.key.key == SDLK_F5) {CreatePipelines();}
                }
                else if (event.type == SDL_EVENT_WINDOW_RESIZED)
                {
                    int w = 0, h = 0;
                    SDL_GetWindowSizeInPixels(window, &w, &h);
                    m_Width  = static_cast<uint16_t>(w);
                    m_Height = static_cast<uint16_t>(h);

                    if (m_Width > 0 && m_Height > 0 && m_pSwapChain)
                    {
                        if (m_pImmediateContext)
                        {
                            m_pImmediateContext->Flush();
                        }

                        if (m_Width > 0 && m_Height > 0 && m_pSwapChain)
                        {
                            m_pSwapChain->Resize(m_Width, m_Height);
                            shaderdata.width = static_cast<float>(m_Width);
                            shaderdata.height = static_cast<float>(m_Height);
                        }
                    }
                }
                else if (event.type == SDL_EVENT_MOUSE_MOTION) {
                    if(m_mouselocked) m_camera.processMouse(event.motion.xrel, event.motion.yrel);
                }
            }
            const bool* keystate = SDL_GetKeyboardState(nullptr);
            m_camera.processKeys(keystate, deltatime);

            shaderdata.viewmat = m_camera.getViewMat();
            shaderdata.campos  = m_camera.pos();

            uint32_t flags = SDL_GetWindowFlags(window);
            bool isMinimized = (flags & SDL_WINDOW_MINIMIZED);

            if (m_Width > 0 && m_Height > 0 && !isMinimized)
            {
                Render();
            }
        }
    }

private:
    void InitWindow()
    {
        #if defined(PLATFORM_LINUX)
        SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "x11"); // wayland does NOT play nice, x11 works on most wayland systems too
        #endif

        if (!SDL_Init(SDL_INIT_VIDEO)) {
            throw std::runtime_error(std::string("Failed to initialize SDL: ") + SDL_GetError());
        }

        window = SDL_CreateWindow("likjghf",
                                  m_Width, m_Height,
                                  SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
        if (!window) {
            throw std::runtime_error(std::string("Failed to create SDL window: ") + SDL_GetError());
        }
        SDL_SetWindowMinimumSize(window, 200, 200);
    }

    void InitSwapchain()
    {
        EngineVkCreateInfo EngineCI;

        auto* pFactoryVk = GetEngineFactoryVk();
        pFactoryVk->CreateDeviceAndContextsVk(EngineCI, &m_pDevice, &m_pImmediateContext);

        SwapChainDesc SCDesc;
        NativeWindow windowData;
        SDL_PropertiesID props = SDL_GetWindowProperties(window);

        #if defined(PLATFORM_WIN32)
        windowData.hWnd = SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);
        #elif defined(PLATFORM_LINUX)
        windowData.WindowId = static_cast<uint32_t>(SDL_GetNumberProperty(props, SDL_PROP_WINDOW_X11_WINDOW_NUMBER, 0));
        windowData.pDisplay = SDL_GetPointerProperty(props, SDL_PROP_WINDOW_X11_DISPLAY_POINTER, nullptr);
        #elif defined(PLATFORM_MACOS)
        windowData.pNSView = SDL_GetPointerProperty(props, SDL_PROP_WINDOW_COCOA_METAL_VIEW_POINTER, nullptr);
        #endif

        pFactoryVk->CreateSwapChainVk(m_pDevice, m_pImmediateContext, SCDesc, windowData, &m_pSwapChain);
    }

    void CreatePipelines()
    {
        m_pPipeline = std::make_unique<ShaderManager>(
            m_pDevice,
            "shaders/main.vert", "shaders/main.frag",
            m_pSwapChain->GetDesc().ColorBufferFormat,
            m_pSwapChain->GetDesc().DepthBufferFormat
        );
    }

    void Render()
    {
        auto* pRTV = m_pSwapChain->GetCurrentBackBufferRTV();
        auto* pDSV = m_pSwapChain->GetDepthBufferDSV();

        m_pImmediateContext->SetRenderTargets(1, &pRTV, pDSV, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        const float ClearColor[] = {0.0f, 0.0f, 0.0f, 1.0f};
        m_pImmediateContext->ClearRenderTarget(pRTV, ClearColor, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        m_pImmediateContext->ClearDepthStencil(pDSV, CLEAR_DEPTH_FLAG, 1.f, 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        shaderdata.time = SDL_GetTicks() * 0.001f;

        m_pPipeline->updateConstants(m_pImmediateContext, &shaderdata, sizeof(shaderdata));

        m_pPipeline->draw(m_pImmediateContext);

        if(m_gui) {
            imgui->begin(m_Width, m_Height, m_pSwapChain->GetDesc().PreTransform);

            ImGui::Begin("Shader configuration");

            // ImGui::SliderFloat("Black hole size", &shaderdata.size, 0.0f, 10.0f);
            ImGui::SliderInt("Acreation disk steps", &shaderdata.steps, 0, 9);
            ImGui::SliderFloat("Black hole speed", &shaderdata.speed, 0.0f, 10.0f);


            ImGui::End();
            imgui->end(m_pImmediateContext);
        }
        m_pSwapChain->Present();
    }

    uint16_t m_Width;
    uint16_t m_Height;

    ShaderData shaderdata{};
    Camera m_camera;

    SDL_Window* window = nullptr;
    bool m_mouselocked = true;
    bool m_fullscreen = false;
    bool m_gui = false;

    std::unique_ptr<GUIHandler> imgui;
    std::unique_ptr<ShaderManager> m_pPipeline;

    RefCntAutoPtr<IRenderDevice>  m_pDevice;
    RefCntAutoPtr<IDeviceContext> m_pImmediateContext;
    RefCntAutoPtr<ISwapChain>     m_pSwapChain;
};

int main(int argc, char** argv)
{
    try {
        Application app(800, 600);
        app.Run();
    } catch (const std::exception& e) {
        std::cerr << "Fatal Error: " << e.what() << "\n";
        return -1;
    }
    return 0;
}
