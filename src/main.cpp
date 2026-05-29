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

#include "Graphics/GraphicsEngineVulkan/interface/EngineFactoryVk.h"
#include "Graphics/GraphicsEngine/interface/SwapChain.h"

using namespace Diligent;

class Application
{
public:
    Application(uint16_t width = 1024, uint16_t height = 768)
    : m_Width(width), m_Height(height)
    {
        InitWindow();
        InitSwapchain();

        CreatePipelines();
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

        while (!quit)
        {
            while (SDL_PollEvent(&event))
            {
                if (event.type == SDL_EVENT_QUIT)
                {
                    quit = true;
                }
                else if (event.type == SDL_EVENT_KEY_DOWN)
                {
                    if (event.key.key == SDLK_ESCAPE) quit = true;
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
                        }
                    }
                }
            }

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
        SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "x11");
        #endif

        if (!SDL_Init(SDL_INIT_VIDEO)) {
            throw std::runtime_error(std::string("Failed to initialize SDL: ") + SDL_GetError());
        }

        window = SDL_CreateWindow("Tutorial00: Hello Cross-Platform (Vulkan)",
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

        m_pPipeline->draw(m_pImmediateContext);

        m_pSwapChain->Present();
    }

    uint16_t m_Width;
    uint16_t m_Height;

    SDL_Window* window = nullptr;

    std::unique_ptr<ShaderManager> m_pPipeline;

    RefCntAutoPtr<IRenderDevice>  m_pDevice;
    RefCntAutoPtr<IDeviceContext> m_pImmediateContext;
    RefCntAutoPtr<ISwapChain>     m_pSwapChain;
};

int main(int argc, char** argv)
{
    try {
        Application app(1024, 768);
        app.Run();
    } catch (const std::exception& e) {
        std::cerr << "Fatal Error: " << e.what() << "\n";
        return -1;
    }
    return 0;
}
