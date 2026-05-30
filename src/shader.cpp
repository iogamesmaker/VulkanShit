//shader.cpp
#include "shader.hpp"
#include "utils.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cstring>

using namespace Diligent;

static RefCntAutoPtr<IShader> compileShader(IRenderDevice* pDevice, const std::string& source, SHADER_TYPE type, const std::string& name) {
    RefCntAutoPtr<IShader> pShader;
    ShaderCreateInfo ShaderCI;
    ShaderCI.SourceLanguage = SHADER_SOURCE_LANGUAGE_HLSL;
    ShaderCI.Desc.UseCombinedTextureSamplers = true;
    ShaderCI.Desc.ShaderType = type;
    ShaderCI.EntryPoint = "main";
    ShaderCI.Source = source.c_str();
    ShaderCI.Desc.Name = name.c_str();

    pDevice->CreateShader(ShaderCI, &pShader);
    if(!pShader) throw std::runtime_error("Shader failed to compile, " + name);
    return pShader;
}

ShaderManager::ShaderManager(IRenderDevice* pDevice, const std::string& vertexPath, const std::string& fragmentPath, TEXTURE_FORMAT RTVFormat, TEXTURE_FORMAT DSVFormat)
: m_pDevice(pDevice), vertexPath(vertexPath), fragmentPath(fragmentPath), m_RTVFormat(RTVFormat), m_DSVFormat(DSVFormat) {
    createPipeline();
}

void ShaderManager::createPipeline() {
    // compile shaders
    std::filesystem::path path;
    std::string source;

    path = root_dir() / vertexPath;
    source = readFile(path.string());
    m_pVS = compileShader(m_pDevice, source, SHADER_TYPE_VERTEX, "vertex shader" + path.string());

    path = root_dir() / fragmentPath;
    source = readFile(path.string());
    m_pFS = compileShader(m_pDevice, source, SHADER_TYPE_PIXEL, "fragment shader" + path.string());

    // create graphics pipeline

    GraphicsPipelineStateCreateInfo PSOCI;
    PSOCI.PSODesc.Name = "super shader pso";
    PSOCI.PSODesc.PipelineType = PIPELINE_TYPE_GRAPHICS;

    PSOCI.pVS = m_pVS;
    PSOCI.pPS = m_pFS;

    auto& GraphicsPipeline = PSOCI.GraphicsPipeline;

    GraphicsPipeline.NumRenderTargets = 1;
    GraphicsPipeline.RTVFormats[0] = m_RTVFormat;
    GraphicsPipeline.DSVFormat = m_DSVFormat;
    GraphicsPipeline.PrimitiveTopology = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    GraphicsPipeline.RasterizerDesc.CullMode = CULL_MODE_NONE;
    GraphicsPipeline.DepthStencilDesc.DepthEnable = False;

    ShaderResourceVariableDesc Vars[] =
    {
        {SHADER_TYPE_PIXEL, "Constants", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE}
    };
    PSOCI.PSODesc.ResourceLayout.Variables    = Vars;
    PSOCI.PSODesc.ResourceLayout.NumVariables = _countof(Vars);

    m_pDevice->CreateGraphicsPipelineState(PSOCI, &m_pPSO);
    if(!m_pPSO) throw std::runtime_error("graphics pipeline couldn't be created");

    // create shader resource binding

    m_pPSO->CreateShaderResourceBinding(&m_pSRB, true); // genuis function naming lmao
    if(!m_pSRB) throw std::runtime_error("shader resource binding creation failed");

    BufferDesc CBDesc;
    CBDesc.Name = "Constant buffer";
    CBDesc.Size = ConstantBufferSize;
    CBDesc.Usage = USAGE_DYNAMIC; // CPU can write, GPU can read. perfect.
    CBDesc.BindFlags = BIND_UNIFORM_BUFFER;
    CBDesc.CPUAccessFlags = CPU_ACCESS_WRITE;

    m_pDevice->CreateBuffer(CBDesc, nullptr, &m_pConstantBuffer);
    if(!m_pConstantBuffer) throw std::runtime_error("constant buffer couldn't be created");

    if (IShaderResourceVariable* pVar = m_pSRB->GetVariableByName(SHADER_TYPE_PIXEL, "Constants"))
        pVar->Set(m_pConstantBuffer);
}

void ShaderManager::bindBuffer(const char* name, IBufferView* pBufferView) {
    if(!m_pSRB) return;
    if(IShaderResourceVariable* pVar = m_pSRB->GetVariableByName(SHADER_TYPE_PIXEL, name)) pVar->Set(pBufferView);
}

void ShaderManager::updateConstants(IDeviceContext* pCtx, const void* pData, uint32_t size) {
    if (!m_pConstantBuffer || !pCtx) return;

    void* pMapped = nullptr;
    pCtx->MapBuffer(m_pConstantBuffer, MAP_WRITE, MAP_FLAG_DISCARD, pMapped);

    if(pMapped)memcpy(pMapped, pData, size);

    pCtx->UnmapBuffer(m_pConstantBuffer, MAP_WRITE);
}

void ShaderManager::draw(IDeviceContext* pCtx) {
    if(!m_pPSO || !m_pSRB) return;

    pCtx->SetPipelineState(m_pPSO);

    pCtx->CommitShaderResources(m_pSRB, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

    DrawAttribs drawAttributes;
    drawAttributes.NumVertices = 6;
    pCtx->Draw(drawAttributes);

}
