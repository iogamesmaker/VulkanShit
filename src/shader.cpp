// shader.cpp
#include "shader.hpp"
#include "utils.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>

ShaderManager::ShaderManager(IRenderDevice* pDevice, const std::string& vertexPath, const std::string& fragmentPath) {
    std::filesystem::path path = root_dir() / vertexPath;

    std::string source = ReadFile(path.string());

    ShaderCreateInfo ShaderCI; // shader configuration shit
    ShaderCI.SourceLanguage = SHADER_SOURCE_LANGUAGE_HLSL;
    ShaderCI.Desc.UseCombinedTextureSamplers = true;
    ShaderCI.Desc.ShaderType = SHADER_TYPE_VERTEX;
    ShaderCI.EntryPoint = "main"; // basically like the function that gets called inside the shader
    ShaderCI.Source = source.c_str();

    std::string debugName = "vertex shader at " + path.string();
    ShaderCI.Desc.Name = debugName.c_str();

    pDevice->CreateShader(ShaderCI, &vertexShader); // let the GPU compile the shader

    if(fragmentPath != "") {
        path = root_dir() / fragmentPath;
        source = ReadFile(path.string());

        ShaderCI.Source = source.c_str();
        ShaderCI.Desc.ShaderType = SHADER_TYPE_PIXEL;

        debugName = "pixel shader at " + path.string();
        ShaderCI.Desc.Name = debugName.c_str();
        pDevice->CreateShader(ShaderCI, &fragmentShader);
    }
}

std::string ShaderManager::ReadFile(const std::string& path) {
    std::ifstream file(path);
    if(!file.is_open()) {
        throw std::runtime_error("couldn't open file " + path);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();

    return buffer.str();
}
