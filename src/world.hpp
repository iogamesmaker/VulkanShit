// world.hpp
#include "Graphics/GraphicsEngine/interface/RenderDevice.h"
#include "Graphics/GraphicsEngine/interface/Texture.h"
#include "Graphics/GraphicsEngine/interface/TextureView.h"
#include "Common/interface/RefCntAutoPtr.hpp"

using namespace Diligent;

class World {
public:
    World(IRenderDevice* device, int size = 128);
    ~World() = default;

    ITexture* getTexture() const { return SDFTexture; }
    ITextureView* getSRV() const { return SDFSRV; }

    int getSize() const { return size; }

private:
    int size;
    RefCntAutoPtr<ITexture> SDFTexture;
    RefCntAutoPtr<ITextureView> SDFSRV;
};
