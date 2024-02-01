#include "2s2h/resource/importer/KeyFrameFactory.h"
#include "2s2h/resource/type/KeyFrame.h"
#include <libultraship/libultraship.h>
#include "spdlog/spdlog.h"
#include "../ZAPDTR/ZAPD/ZCKeyFrame.h"

namespace LUS {



std::shared_ptr<IResource> KeyFrameSkelFactory::ReadResource(std::shared_ptr<ResourceInitData> initData,
                                                         std::shared_ptr<BinaryReader> reader) {
    auto resource = std::make_shared<KeyFrameSkel>(initData);
    std::shared_ptr<ResourceVersionFactory> factory = nullptr;

    switch (resource->GetInitData()->ResourceVersion) { 
        case 0:
            factory = std::make_shared<KeyFrameSkelFactoryV0>();
            break;
    }

    if (factory == nullptr) {
        SPDLOG_ERROR("Failed to load Key Frame skel with version {}", resource->GetInitData()->ResourceVersion);
    }

    factory->ParseFileBinary(reader, resource);

    return resource;
}

void KeyFrameSkelFactoryV0::ParseFileBinary(std::shared_ptr<BinaryReader> reader, std::shared_ptr<IResource> resource) {
    std::shared_ptr<KeyFrameSkel> skel = std::static_pointer_cast<KeyFrameSkel>(resource);
    ZKeyframeSkelType skelType;

    ResourceVersionFactory::ParseFileBinary(reader, skel);
    skel->skelData.limbCount = reader->ReadUByte();
    skel->skelData.dListCount = reader->ReadUByte();
    skelType = (ZKeyframeSkelType)reader->ReadUByte();
    uint8_t numLimbs = reader->ReadUByte();

    if (skelType == ZKeyframeSkelType::Normal) {
        KeyFrameStandardLimb* limbs = new KeyFrameStandardLimb[numLimbs];

        for (uint32_t i = 0; i < numLimbs; i++) {
            std::string dlStr = reader->ReadString();
            auto dl = LUS::Context::GetInstance()->GetResourceManager()->LoadResourceProcess(dlStr.c_str());
            limbs[i].dList = nullptr;
            if (dl != nullptr) {
                limbs[i].dList = dl->GetRawPointer();
            }
            limbs[i].numChildren = reader->ReadUByte();
            limbs[i].flags = reader->ReadUByte();
            limbs[i].translation.x = reader->ReadInt16();
            limbs[i].translation.y = reader->ReadInt16();
            limbs[i].translation.z = reader->ReadInt16();
        }
        skel->skelData.limbsStandard = limbs;
    } else {
        KeyFrameFlexLimb* limbs = new KeyFrameFlexLimb[numLimbs];

        for (uint32_t i = 0; i < numLimbs; i++) {
            std::string dlStr = reader->ReadString();
            auto dl = LUS::Context::GetInstance()->GetResourceManager()->LoadResourceProcess(dlStr.c_str());
            limbs[i].dList = nullptr;
            if (dl != nullptr) {
                limbs[i].dList = dl->GetRawPointer();
            }
            limbs[i].numChildren = reader->ReadUByte();
            limbs[i].flags = reader->ReadUByte();
            limbs[i].callbackIndex = reader->ReadUByte();
        }
        skel->skelData.limbsFlex = limbs;   
    }
}

std::shared_ptr<IResource> KeyFrameAnimFactory::ReadResource(std::shared_ptr<ResourceInitData> initData,
                                                             std::shared_ptr<BinaryReader> reader) {
    auto resource = std::make_shared<KeyFrameAnim>(initData);
    std::shared_ptr<ResourceVersionFactory> factory = nullptr;

    switch (resource->GetInitData()->ResourceVersion) {
        case 0:
            factory = std::make_shared<KeyFrameAnimFactoryV0>();
            break;
    }

    if (factory == nullptr) {
        SPDLOG_ERROR("Failed to load Key Frame skel with version {}", resource->GetInitData()->ResourceVersion);
    }

    factory->ParseFileBinary(reader, resource);

    return resource;
}

void KeyFrameAnimFactoryV0::ParseFileBinary(std::shared_ptr<BinaryReader> reader, std::shared_ptr<IResource> resource) {
    std::shared_ptr<KeyFrameAnim> anim = std::static_pointer_cast<KeyFrameAnim>(resource);
    ZKeyframeSkelType skelType;

    ResourceVersionFactory::ParseFileBinary(reader, anim);

    skelType = (ZKeyframeSkelType)reader->ReadUByte();
    
    uint32_t numBitFlags = reader->ReadUInt32();

    if (skelType == ZKeyframeSkelType::Normal) {
        anim->animData.bitFlags = new u8[numBitFlags];
        for (uint32_t i = 0; i < numBitFlags; i++) {
            anim->animData.bitFlags[i] = reader->ReadUByte();
        }
    } else {
        anim->animData.bitFlagsFlex = new u16[numBitFlags];
        for (uint32_t i = 0; i < numBitFlags; i++) {
            anim->animData.bitFlagsFlex[i] = reader->ReadUInt16();
        }
    }

    uint32_t numKf = reader->ReadUInt32();
    anim->animData.keyFrames = new KeyFrame[numKf];

    for (uint32_t i = 0; i < numKf; i++) {
        anim->animData.keyFrames[i].frame = reader->ReadInt16();
        anim->animData.keyFrames[i].value = reader->ReadInt16();
        anim->animData.keyFrames[i].velocity = reader->ReadInt16();
    }

    uint32_t numKfNums = reader->ReadUInt32();
    anim->animData.kfNums = new s16[numKfNums];

    for (uint32_t i = 0; i < numKfNums; i++) {
        anim->animData.kfNums[i] = reader->ReadInt16();
    }

    uint32_t numPresetValues = reader->ReadUInt32();
    anim->animData.presetValues = new s16[numPresetValues];

    for (uint32_t i = 0; i < numPresetValues; i++) {
        anim->animData.presetValues[i] = reader->ReadInt16();
    }

    anim->animData.unk_10[0] = reader->ReadUByte();
    anim->animData.unk_10[0] = reader->ReadUByte();

    anim->animData.duration = reader->ReadUInt16();
}

} // namespace LUS