#include "2s2h/resource/importer/TextureAnimationFactory.h"
#include "2s2h/resource/type/TextureAnimation.h"
#include <libultraship/libultraship.h>
#include "spdlog/spdlog.h"

namespace LUS {

std::shared_ptr<IResource> TextureAnimationFactory::ReadResource(std::shared_ptr<ResourceInitData> initData,
                                        std::shared_ptr<BinaryReader> reader) {
    auto resource = std::make_shared<TextureAnimation>(initData);
    std::shared_ptr<ResourceVersionFactory> factory = nullptr;

    switch (resource->GetInitData()->ResourceVersion) {
        case 0:
            factory = std::make_shared<TextureAnimationFactoryV0>();
            break;
    }

    if (factory == nullptr) {
        SPDLOG_ERROR("Failed to load Texture Animation with version {}", resource->GetInitData()->ResourceVersion);
        return nullptr;
    }

    factory->ParseFileBinary(reader, resource);

    return resource;
}

void LUS::TextureAnimationFactoryV0::ParseFileBinary(std::shared_ptr<BinaryReader> reader,
                                                     std::shared_ptr<IResource> resource) {
    std::shared_ptr<TextureAnimation> tAnim = std::static_pointer_cast<TextureAnimation>(resource);
    ResourceVersionFactory::ParseFileBinary(reader, tAnim);

    size_t numEntries = reader->ReadUInt32();
    
    for (size_t i = 0; i < numEntries; i++) {
        AnimatedMaterial anim;
        anim.segment = reader->ReadInt8();
        anim.type = reader->ReadInt16();

        switch ((TextureAnimationParamsType)anim.type) {
            case TextureAnimationParamsType::SingleScroll: {
                auto* e = new AnimatedMatTexScrollParams;
                e->xStep = reader->ReadInt8();
                e->yStep = reader->ReadInt8();
                e->width = reader->ReadUByte();
                e->height = reader->ReadUByte();
                anim.params = e;
                break;
            }
            case TextureAnimationParamsType::DualScroll: {
                auto* e = new AnimatedMatTexScrollParams[2];
                e[0].xStep = reader->ReadInt8();
                e[0].yStep = reader->ReadInt8();
                e[0].width = reader->ReadUByte();
                e[0].height = reader->ReadUByte();
                e[1].xStep = reader->ReadInt8();
                e[1].yStep = reader->ReadInt8();
                e[1].width = reader->ReadUByte();
                e[1].height = reader->ReadUByte();
                anim.params = e;
                break;
            }
            case TextureAnimationParamsType::ColorChange:
            case TextureAnimationParamsType::ColorChangeLERP:
            case TextureAnimationParamsType::ColorChangeLagrange: {
                auto* e = new AnimatedMatColorParams;
                e->keyFrameLength = reader->ReadUInt16();
                e->keyFrameCount = reader->ReadUInt16();
                
                size_t frames = reader->ReadUInt32();

                e->keyFrames = new uint16_t[frames];
                for (size_t i = 0; i < frames; i++) {
                    e->keyFrames[i] = reader->ReadUInt16();
                }
                size_t primColorSize = reader->ReadUInt32();
                e->primColors = new F3DPrimColor[primColorSize];
                
                for (size_t i = 0; i < primColorSize; i++) {
                    e->primColors[i].r = reader->ReadUByte();
                    e->primColors[i].g = reader->ReadUByte();
                    e->primColors[i].b = reader->ReadUByte();
                    e->primColors[i].a = reader->ReadUByte();
                    e->primColors[i].lodFrac = reader->ReadUByte();
                }

                size_t envColorSize = reader->ReadUInt16();
                e->envColors = new F3DEnvColor[envColorSize];
                for (size_t i = 0; i < envColorSize; i++) {
                    e->envColors[i].r = reader->ReadUByte();
                    e->envColors[i].g = reader->ReadUByte();
                    e->envColors[i].b = reader->ReadUByte();
                    e->envColors[i].a = reader->ReadUByte();
                }
                anim.params = e;
                break;
            }
            case TextureAnimationParamsType::TextureCycle: {
                auto* e = new AnimatedMatTexCycleParams;

                e->keyFrameLength = reader->ReadUInt16();
                e->textureList = new void*[e->keyFrameLength];
                e->textureIndexList = new uint8_t[e->keyFrameLength];

                tAnim->textureCycleTextures.reserve(e->keyFrameLength);

                for (size_t i = 0; i < e->keyFrameLength; i++) {
                    tAnim->textureCycleTextures.emplace_back("__OTR__" + reader->ReadString());
                    e->textureList[i] = (Gfx*)tAnim->textureCycleTextures[i].c_str();
                }
                for (size_t i = 0; i < e->keyFrameLength; i++) {
                    e->textureIndexList[i] = reader->ReadUByte();
                }
                anim.params = e;
                break;
            }
            case TextureAnimationParamsType::Empty: {
                anim.params = nullptr;
                break;
            }
        }
        tAnim->anims.emplace_back(anim);
    }
}

} // namespace LUS