// world.cpp
#include "world.hpp"
#include <vector>
#include <cmath>

World::World(IRenderDevice* device, int size)
: size(size)
{
    TextureDesc TexDesc;
    TexDesc.Name      = "World SDF 3D Texture";
    TexDesc.Type      = RESOURCE_DIM_TEX_3D;
    TexDesc.Width     = static_cast<uint32_t>(size);
    TexDesc.Height    = static_cast<uint32_t>(size);
    TexDesc.Depth     = static_cast<uint32_t>(size);
    TexDesc.Format    = TEX_FORMAT_R32_FLOAT;
    TexDesc.Usage     = USAGE_DEFAULT;
    TexDesc.BindFlags = BIND_SHADER_RESOURCE;

    std::vector<float> cpuData(size * size * size);

    float center = size / 2.0f;
    float radius = size * 0.35f;

    for (int z = 0; z < size; ++z) {
        for (int y = 0; y < size; ++y) {
            for (int x = 0; x < size; ++x) {
                float dx = x - center;
                float dy = y - center;
                float dz = z - center;

                float dist = std::sqrt(dx*dx + dy*dy + dz*dz) - radius;

                int index = x + (y * size) + (z * size * size);
                cpuData[index] = dist;
            }
        }
    }

    TextureSubResData SubresourceData[] = {
        {
            cpuData.data(),
            static_cast<Uint64>(size * sizeof(float)), // row
            static_cast<Uint64>(size * size * sizeof(float)) // depth
        }
    };
    TextureData TexData(SubresourceData, 1);

    // 4. Upload to VRAM
    device->CreateTexture(TexDesc, &TexData, &SDFTexture);

    SDFSRV = SDFTexture->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);
}
