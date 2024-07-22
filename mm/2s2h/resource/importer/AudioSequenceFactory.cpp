#include "2s2h/resource/importer/AudioSequenceFactory.h"
#include "2s2h/resource/type/AudioSequence.h"
#include "spdlog/spdlog.h"
#include "StringHelper.h"
#include "libultraship/libultraship.h"

#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"

namespace SOH {
std::shared_ptr<Ship::IResource> ResourceFactoryBinaryAudioSequenceV2::ReadResource(std::shared_ptr<Ship::File> file) {
    if (!FileHasValidFormatAndReader(file)) {
        return nullptr;
    }

    auto audioSequence = std::make_shared<AudioSequence>(file->InitData);
    auto reader = std::get<std::shared_ptr<Ship::BinaryReader>>(file->Reader);

    audioSequence->sequence.seqDataSize = reader->ReadInt32();
    audioSequence->sequenceData.reserve(audioSequence->sequence.seqDataSize);
    for (uint32_t i = 0; i < audioSequence->sequence.seqDataSize; i++) {
        audioSequence->sequenceData.push_back(reader->ReadChar());
    }
    audioSequence->sequence.seqData = audioSequence->sequenceData.data();

    audioSequence->sequence.seqNumber = reader->ReadUByte();
    audioSequence->sequence.medium = reader->ReadUByte();
    audioSequence->sequence.cachePolicy = reader->ReadUByte();

    audioSequence->sequence.numFonts = reader->ReadUInt32();
    for (uint32_t i = 0; i < 16; i++) {
        audioSequence->sequence.fonts[i] = 0;
    }
    for (uint32_t i = 0; i < audioSequence->sequence.numFonts; i++) {
        audioSequence->sequence.fonts[i] = reader->ReadUByte();
    }

    return audioSequence;
}

int8_t SOH::ResourceFactoryXMLAudioSequenceV0::MediumStrToInt(const char* str) {
    if (!strcmp("Ram", str)) {
        return 0;
    } else if (!strcmp("Unk", str)) {
        return 1;
    } else if (!strcmp("Cart", str)) {
        return 2;
    } else if (!strcmp("Disk", str)) {
        return 3;
        // 4 is skipped
    } else if (!strcmp("RamUnloaded", str)) {
        return 5;
    } else {
        throw std::runtime_error(
            StringHelper::Sprintf("Bad medium value. Got %s, expected Ram, Unk, Cart, or Disk.", str));
    }
}

int8_t ResourceFactoryXMLAudioSequenceV0::CachePolicyToInt(const char* str) {
    if (!strcmp("Temporary", str)) {
        return 0;
    } else if (!strcmp("Persistent", str)) {
        return 1;
    } else if (!strcmp("Either", str)) {
        return 2;
    } else if (!strcmp("Permanent", str)) {
        return 3;
    } else {
        throw std::runtime_error(StringHelper::Sprintf(
            "Bad cache policy value. Got %s, expected Temporary, Persistent, Either, or Permanent.", str));
    }
}

std::shared_ptr<Ship::IResource> ResourceFactoryXMLAudioSequenceV0::ReadResource(std::shared_ptr<Ship::File> file) {
    if (!FileHasValidFormatAndReader(file)) {
        return nullptr;
    }
    auto sequence = std::make_shared<AudioSequence>(file->InitData);
    auto child = std::get<std::shared_ptr<tinyxml2::XMLDocument>>(file->Reader)->FirstChildElement();
    unsigned int i = 0;
    std::shared_ptr<Ship::ResourceInitData> initData = std::make_shared<Ship::ResourceInitData>();

    sequence->sequence.medium = MediumStrToInt(child->Attribute("Medium"));
    sequence->sequence.cachePolicy = CachePolicyToInt(child->Attribute("CachePolicy"));
    sequence->sequence.seqDataSize = child->IntAttribute("Size");
    sequence->sequence.seqNumber = child->IntAttribute("Index");
    const char* customStr = child->Attribute("CustomFormat");

    memset(sequence->sequence.fonts, 0, sizeof(sequence->sequence.fonts));

    tinyxml2::XMLElement* fontsElement = child->FirstChildElement();
    tinyxml2::XMLElement* fontElement = fontsElement->FirstChildElement();
    while (fontElement != nullptr) {
        sequence->sequence.fonts[i] = fontElement->IntAttribute("FontIdx");
        fontElement = fontElement->NextSiblingElement();
        i++;
    }
    sequence->sequence.numFonts = i;

    const char* path = child->Attribute("Path");
    initData->Path = path;
    initData->IsCustom = false;
    initData->ByteOrder = Ship::Endianness::Native;
    auto seqFile = Ship::Context::GetInstance()->GetResourceManager()->GetArchiveManager()->LoadFile(path, initData);

    if (customStr == nullptr) {
        sequence->sequenceData = *seqFile->Buffer.get();
        sequence->sequence.seqData = sequence->sequenceData.data();
    } else {
        for (size_t i = 0; customStr[i] != 0; i++) {
            const_cast<char*>(customStr)[i] = tolower(customStr[i]);
        }
        if (strcmp(customStr, "wav") == 0) {
            drwav wav;
            drwav_uint64 numFrames;

            drwav_bool32 ret =
                drwav_init_memory(&wav, seqFile->Buffer.get()->data(), seqFile->Buffer.get()->size(), nullptr);

            drwav_get_length_in_pcm_frames(&wav, &numFrames);

            sequence->sampleRate = wav.fmt.sampleRate;
            sequence->numChannels = wav.fmt.channels;
            sequence->customSeqData = std::make_unique<short[]>(numFrames);
            drwav_read_pcm_frames_s16(&wav, numFrames, sequence->customSeqData.get());

            sequence->sequence.seqData = (char*)sequence->customSeqData.get();
            // Write a marker to show this is a streamed audio file
            memcpy(sequence->sequence.fonts, "07151129", sizeof("07151129"));
            drwav_uninit(&wav);
        } else if (strcmp(customStr, "mp3") == 0) {
            // BENTODO
        } else if (strcmp(customStr, "flac") == 0) {
            // BENTODO
        } else {
            throw std::runtime_error(
                StringHelper::Sprintf("Bad audio format value. Got %s, expected wav, mp3, or flac.", customStr));
        }
    }
    return sequence;
}
} // namespace SOH
