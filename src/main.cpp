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
#include "world.hpp"

#include "Graphics/GraphicsEngineVulkan/interface/EngineFactoryVk.h"
#include "Graphics/GraphicsEngine/interface/SwapChain.h"

std::unique_ptr<World> gameWorld;

using namespace Diligent;

class Application
{
public:
    Application(uint16_t width = 1024, uint16_t height = 768)
    : wWidth(width), wHeight(height)
    {
        InitWindow();
        InitSwapchain();

        gameWorld = std::make_unique<World>(device, 128);

        CreatePipelines();
    }

    ~Application()
    {
        if (immediateContext) immediateContext->Flush();
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
                    if (event.key.key == SDLK_W) playerZ += 0.01;
                    if (event.key.key == SDLK_A) playerX -= 0.01;
                    if (event.key.key == SDLK_S) playerZ -= 0.01;
                    if (event.key.key == SDLK_D) playerX += 0.01;
                }
                else if (event.type == SDL_EVENT_WINDOW_RESIZED)
                {
                    int w = 0, h = 0;
                    SDL_GetWindowSizeInPixels(window, &w, &h);
                    wWidth  = static_cast<uint16_t>(w);
                    wHeight = static_cast<uint16_t>(h);

                    if (wWidth > 0 && wHeight > 0 && swapChain)
                    {
                        if (immediateContext)
                        {
                            immediateContext->Flush();
                        }

                        if (wWidth > 0 && wHeight > 0 && swapChain)
                        {
                            swapChain->Resize(wWidth, wHeight);
                        }
                    }
                }
            }

            uint32_t flags = SDL_GetWindowFlags(window);
            bool isMinimized = (flags & SDL_WINDOW_MINIMIZED);

            if (wWidth > 0 && wHeight > 0 && !isMinimized)
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
                                    wWidth, wHeight,
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
        pFactoryVk->CreateDeviceAndContextsVk(EngineCI, &device, &immediateContext);

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

        pFactoryVk->CreateSwapChainVk(device, immediateContext, SCDesc, windowData, &swapChain);
    }

    void CreatePipelines()
    {

        GraphicsPipelineStateCreateInfo PSOCreateInfo;

        PSOCreateInfo.PSODesc.Name         = "quad PSO";
        PSOCreateInfo.PSODesc.PipelineType = PIPELINE_TYPE_GRAPHICS;

        ShaderResourceVariableDesc Vars[] = {
            {SHADER_TYPE_PIXEL, "SDFTexture", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE}
        };

        PSOCreateInfo.PSODesc.ResourceLayout.Variables = Vars;
        PSOCreateInfo.PSODesc.ResourceLayout.NumVariables = _countof(Vars);

        SamplerDesc SamplerCI;

        SamplerCI.MinFilter = FILTER_TYPE_LINEAR;
        SamplerCI.MagFilter = FILTER_TYPE_LINEAR;
        SamplerCI.MipFilter = FILTER_TYPE_LINEAR;
        SamplerCI.AddressU = TEXTURE_ADDRESS_CLAMP;
        SamplerCI.AddressV = TEXTURE_ADDRESS_CLAMP;
        SamplerCI.AddressW = TEXTURE_ADDRESS_CLAMP;

        ImmutableSamplerDesc ImrSamplers[] = {
            {SHADER_TYPE_PIXEL, "SDFTexture", SamplerCI}
        };

        PSOCreateInfo.PSODesc.ResourceLayout.ImmutableSamplers = ImrSamplers;
        PSOCreateInfo.PSODesc.ResourceLayout.NumImmutableSamplers = _countof(ImrSamplers);

        PSOCreateInfo.GraphicsPipeline.NumRenderTargets  = 1;
        PSOCreateInfo.GraphicsPipeline.RTVFormats[0]     = swapChain->GetDesc().ColorBufferFormat;
        PSOCreateInfo.GraphicsPipeline.DSVFormat         = TEX_FORMAT_D32_FLOAT;
        PSOCreateInfo.GraphicsPipeline.PrimitiveTopology = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        PSOCreateInfo.GraphicsPipeline.RasterizerDesc.CullMode = CULL_MODE_NONE;
        PSOCreateInfo.GraphicsPipeline.DepthStencilDesc.DepthEnable = False;

        ShaderManager shaderManager(device, "shaders/main.vert", "shaders/main.frag");

        PSOCreateInfo.pVS = shaderManager.getVertexShader();
        PSOCreateInfo.pPS = shaderManager.getFragmentShader();

        device->CreateGraphicsPipelineState(PSOCreateInfo, &PSO);
        PSO->CreateShaderResourceBinding(&SRB, true);


    }

    void Render()
    {
        auto* RTV = swapChain->GetCurrentBackBufferRTV();
        auto* DSV = swapChain->GetDepthBufferDSV();

        immediateContext->SetRenderTargets(1, &RTV, DSV, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        const float ClearColor[] = {0.35f, 0.35f, 0.35f, 1.0f};
        immediateContext->ClearRenderTarget(RTV, ClearColor, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        immediateContext->ClearDepthStencil(DSV, CLEAR_DEPTH_FLAG, 1.f, 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        // Query variable directly from the SRB object instance
        auto* pSRVVar = SRB->GetVariableByName(SHADER_TYPE_PIXEL, "SDFTexture");
        if(pSRVVar) {
            pSRVVar->Set(gameWorld->getSRV());
        }

        // Pass the SRB and use the proper COMMIT_SHADER flag macro
        immediateContext->CommitShaderResources(SRB, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        immediateContext->SetPipelineState(PSO);

        DrawAttribs drawAttrs;
        drawAttrs.NumVertices = 6;
        immediateContext->Draw(drawAttrs);

        swapChain->Present();
    }

private:
    uint16_t wWidth;
    uint16_t wHeight;

    SDL_Window* window = nullptr;

    RefCntAutoPtr<IRenderDevice>  device;
    RefCntAutoPtr<IDeviceContext> immediateContext;
    RefCntAutoPtr<ISwapChain>     swapChain;
    RefCntAutoPtr<IPipelineState> PSO;
    RefCntAutoPtr<IShaderResourceBinding> SRB;

    float playerX = 0;
    float playerY = 0;
    float playerZ = 0;
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
