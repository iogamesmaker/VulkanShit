// imguishit.hpp
#pragma once

#include <memory>
#include <stdexcept>
#include <SDL3/SDL.h>
#include "imgui.h"

#include <backends/imgui_impl_sdl3.h>
#include <Imgui/interface/ImGuiImplDiligent.hpp>

using namespace Diligent;

class GUIHandler {
public:
    GUIHandler(
        SDL_Window*      window,
        IRenderDevice*   device,
        TEXTURE_FORMAT   colorFormat,
        TEXTURE_FORMAT   depthFormat)
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::StyleColorsLight();

        ImGui_ImplSDL3_InitForOther(window);

        int w, h;
        SDL_GetWindowSize(window, &w, &h);

        ImGuiDiligentCreateInfo CI;
        CI.pDevice        = device;
        CI.BackBufferFmt  = colorFormat;
        CI.DepthBufferFmt = depthFormat;

        m_pImGuiImpl = std::make_unique<ImGuiImplDiligent>(CI);
    }
    ~GUIHandler() {
        m_pImGuiImpl.reset();
        ImGui_ImplSDL3_Shutdown();
        ImGui::DestroyContext();
    }
    void begin(uint32_t width, uint32_t height, SURFACE_TRANSFORM transform) {
        ImGui_ImplSDL3_NewFrame();
        m_pImGuiImpl->NewFrame(width, height, transform);
    }
    void end(IDeviceContext* pContext) {
        m_pImGuiImpl->Render(pContext);
    }

    void process(const SDL_Event* event) {
        ImGui_ImplSDL3_ProcessEvent(event);
    }


private:
    std::unique_ptr<ImGuiImplDiligent> m_pImGuiImpl;

    SDL_Window* m_pWindow;
    bool show = true;
};
