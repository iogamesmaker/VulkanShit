// shader.hpp
#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <Graphics/GraphicsEngine/interface/RenderDevice.h>
#include <Graphics/GraphicsEngine/interface/DeviceContext.h>
#include <Graphics/GraphicsEngine/interface/Buffer.h>
#include <Graphics/GraphicsEngine/interface/TextureView.h>
#include <Common/interface/RefCntAutoPtr.hpp>

using namespace Diligent;

class ShaderManager {
public:
    ShaderManager(IRenderDevice* Device, const std::string& vertexpath, const std::string& fragmentpath = "");
    ~ShaderManager() = default;

    void BindResource(IShaderResourceBinding* pSRB, SHADER_TYPE shaderType, const char* varName, IDeviceObject* pResource);

    IShader* getVertexShader() const {return vertexShader;}
    IShader* getFragmentShader() const {return fragmentShader;}
private:
    std::string ReadFile(const std::string& path);

    RefCntAutoPtr<IShader> vertexShader;
    RefCntAutoPtr<IShader> fragmentShader;
};
