//shader.hpp
#pragma once

#include <string>
#include <Graphics/GraphicsEngine/interface/RenderDevice.h>
#include <Graphics/GraphicsEngine/interface/DeviceContext.h>
#include <Graphics/GraphicsEngine/interface/PipelineState.h>
#include <Graphics/GraphicsEngine/interface/Shader.h>
#include <Graphics/GraphicsEngine/interface/TextureView.h>
#include <Graphics/GraphicsEngine/interface/Buffer.h>
#include <Graphics/GraphicsEngine/interface/Sampler.h>
#include <Graphics/GraphicsEngine/interface/DeviceContext.h>
#include <Common/interface/RefCntAutoPtr.hpp>

using namespace Diligent;

class ShaderManager {
public:
    ShaderManager(IRenderDevice* pDevice,
                   const std::string& vertexPath,
                   const std::string& fragmentPath,
                   TEXTURE_FORMAT RTVFormat,
                   TEXTURE_FORMAT DSVFormat = TEX_FORMAT_UNKNOWN);
    ~ShaderManager() = default;

    void bindBuffer(const char* name, IBufferView* pBufferView);

    void updateConstants(IDeviceContext* pCtx, const void* pData, uint32_t size); // call this to upload new data with custom structures

    void draw(IDeviceContext* pCtx);

    IShaderResourceBinding* getSRB() { return m_pSRB; }
private:
    void createPipeline();

    IRenderDevice* m_pDevice = nullptr;

    std::string vertexPath, fragmentPath;
    TEXTURE_FORMAT m_RTVFormat, m_DSVFormat;

    RefCntAutoPtr<IShader> m_pVS, m_pFS;
    RefCntAutoPtr<IPipelineState> m_pPSO;
    RefCntAutoPtr<IShaderResourceBinding> m_pSRB;

    RefCntAutoPtr<IBuffer> m_pConstantBuffer;
    static constexpr uint32_t ConstantBufferSize = 256;
};
