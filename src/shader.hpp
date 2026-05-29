// shaders.hpp
#pragma once

#include <string>
#include "Graphics/GraphicsEngine/interface/RenderDevice.h"
#include "Graphics/GraphicsEngine/interface/DeviceContext.h"
#include "Common/interface/RefCntAutoPtr.hpp"

using namespace Diligent;

class ShaderManager {
public:
    ShaderManager(IRenderDevice* pDevice, const std::string& vertexpath, const std::string& fragmentpath = "");
    ~ShaderManager() = default;

    IShader* getVertexShader() const {return vertexShader;}
    IShader* getFragmentShader() const {return fragmentShader;}
private:
    std::string ReadFile(const std::string& path);

    RefCntAutoPtr<IShader> vertexShader;
    RefCntAutoPtr<IShader> fragmentShader;
};
