#include "TextureAnimation.h"

namespace SOH {
TextureAnimation::~TextureAnimation() {
    for (auto& a : anims) {
        switch ((TextureAnimationParamsType)a.type) {
            case TextureAnimationParamsType::SingleScroll:
                delete a.params;
                break;
            case TextureAnimationParamsType::DualScroll:
                delete[] a.params;
                break;
            case TextureAnimationParamsType::ColorChange:
            case TextureAnimationParamsType::ColorChangeLERP:
            case TextureAnimationParamsType::ColorChangeLagrange:
                delete[] ((AnimatedMatColorParams*)a.params)->keyFrames;
                delete[] ((AnimatedMatColorParams*)a.params)->primColors;
                delete[] ((AnimatedMatColorParams*)a.params)->envColors;
                delete a.params;
                break;
            case TextureAnimationParamsType::TextureCycle:
                // BENTODO free the textures
                delete[] ((AnimatedMatTexCycleParams*)a.params)->textureIndexList;
                delete[] ((AnimatedMatTexCycleParams*)a.params)->textureList;
                delete a.params;
                break;
        }
    }
}
AnimatedMaterial* TextureAnimation::GetPointer() {
    return anims.data();
}
size_t TextureAnimation::GetPointerSize() {
    return anims.size() * sizeof(AnimatedMaterial);
}
} // namespace SOH