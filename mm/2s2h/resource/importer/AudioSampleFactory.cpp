#include "2s2h/resource/importer/AudioSampleFactory.h"
#include "2s2h/resource/type/AudioSample.h"
#include "2s2h/resource/importer/AudioSoundFontFactory.h"
#include "StringHelper.h"
#include "libultraship/libultraship.h"
#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"

#define DR_MP3_IMPLEMENTATION
#include "dr_mp3.h"

namespace SOH {
std::shared_ptr<Ship::IResource> ResourceFactoryBinaryAudioSampleV2::ReadResource(std::shared_ptr<Ship::File> file) {
    if (!FileHasValidFormatAndReader(file)) {
        return nullptr;
    }

    auto audioSample = std::make_shared<AudioSample>(file->InitData);
    auto reader = std::get<std::shared_ptr<Ship::BinaryReader>>(file->Reader);

    audioSample->sample.codec = reader->ReadUByte();
    audioSample->sample.medium = reader->ReadUByte();
    audioSample->sample.unk_bit26 = reader->ReadUByte();
    audioSample->sample.unk_bit25 = reader->ReadUByte();
    audioSample->sample.size = reader->ReadUInt32();

    audioSample->audioSampleData.reserve(audioSample->sample.size);
    for (uint32_t i = 0; i < audioSample->sample.size; i++) {
        audioSample->audioSampleData.push_back(reader->ReadUByte());
    }
    audioSample->sample.sampleAddr = audioSample->audioSampleData.data();

    audioSample->loop.start = reader->ReadUInt32();
    audioSample->loop.end = reader->ReadUInt32();
    audioSample->loop.count = reader->ReadUInt32();

    audioSample->loopStateCount = reader->ReadUInt32();
    for (int i = 0; i < 16; i++) {
        audioSample->loop.state[i] = 0;
    }
    for (uint32_t i = 0; i < audioSample->loopStateCount; i++) {
        audioSample->loop.state[i] = reader->ReadInt16();
    }
    audioSample->sample.loop = &audioSample->loop;

    audioSample->book.order = reader->ReadInt32();
    audioSample->book.npredictors = reader->ReadInt32();
    audioSample->bookDataCount = reader->ReadUInt32();

    audioSample->bookData.reserve(audioSample->bookDataCount);
    for (uint32_t i = 0; i < audioSample->bookDataCount; i++) {
        audioSample->bookData.push_back(reader->ReadInt16());
    }
    audioSample->book.book = audioSample->bookData.data();
    audioSample->sample.book = &audioSample->book;

    return audioSample;
}
std::shared_ptr<Ship::IResource> ResourceFactoryXMLAudioSampleV0::ReadResource(std::shared_ptr<Ship::File> file) {
    if (!FileHasValidFormatAndReader(file)) {
        return nullptr;
    }

    auto audioSample = std::make_shared<AudioSample>(file->InitData);
    auto child = std::get<std::shared_ptr<tinyxml2::XMLDocument>>(file->Reader)->FirstChildElement();
    std::shared_ptr<Ship::ResourceInitData> initData = std::make_shared<Ship::ResourceInitData>();
    const char* customFormatStr = child->Attribute("CustomFormat");
    memset(&audioSample->sample, 0, sizeof(audioSample->sample));
    audioSample->sample.codec = CodecStrToInt(child->Attribute("Codec"));
    audioSample->sample.medium = ResourceFactoryXMLSoundFontV0::MediumStrToInt(child->Attribute("Medium"));
    audioSample->sample.unk_bit25 = child->IntAttribute("Relocated");
    audioSample->sample.unk_bit26 = child->IntAttribute("bit26");
    audioSample->loop.start = child->IntAttribute("LoopStart");
    audioSample->loop.end = child->IntAttribute("LoopEnd");
    audioSample->loop.count = child->IntAttribute("LoopCount");
    
    // CODEC_ADPCM || CODEC_SMALL_ADPCM
    if (audioSample->sample.codec == 0 || audioSample->sample.codec == 3) {
        tinyxml2::XMLElement* loopElement = child->FirstChildElement();
        size_t i = 0;
        memset(audioSample->loop.state, 0, sizeof(audioSample->loop.state));
        if (loopElement != nullptr) {
            while (strcmp(loopElement->Name(), "LoopState") == 0) {
                audioSample->loop.state[i] = loopElement->IntAttribute("Loop");
                loopElement = loopElement->NextSiblingElement();
                i++;
            }
            audioSample->loop.count = i;
            i = 0;
            while (loopElement != nullptr && strcmp(loopElement->Name(), "Books") == 0) {
                audioSample->bookData.push_back(loopElement->IntAttribute("Book"));
                loopElement = loopElement->NextSiblingElement();
                i++;
            }
        }
        audioSample->book.npredictors = child->IntAttribute("Npredictors");
        audioSample->book.order = child->IntAttribute("Order");
        
        audioSample->book.book = audioSample->bookData.data();
        audioSample->sample.book = &audioSample->book;
    }
    audioSample->sample.loop = &audioSample->loop;
    size_t size = child->Int64Attribute("SampleSize");
    audioSample->sample.size = size;

    const char* path = child->Attribute("SamplePath");
    initData->Path = path;
    initData->IsCustom = false;
    initData->ByteOrder = Ship::Endianness::Native;
    auto sampleFile = Ship::Context::GetInstance()->GetResourceManager()->GetArchiveManager()->LoadFile(path, initData);
    if (customFormatStr != nullptr && strcmp(customFormatStr, "wav") == 0) {
        drwav wav;
        drwav_uint64 numFrames;

        drwav_bool32 ret =
            drwav_init_memory(&wav, sampleFile->Buffer.get()->data(), sampleFile->Buffer.get()->size(), nullptr);

        drwav_get_length_in_pcm_frames(&wav, &numFrames);

        audioSample->audioSampleData.reserve(numFrames * 4);
        
        drwav_read_pcm_frames_s16(&wav, numFrames, (int16_t*)audioSample->audioSampleData.data());
    } else if (customFormatStr != nullptr && strcmp(customFormatStr, "mp3") == 0) {
        drmp3 mp3;
        drwav_uint64 numFrames;
        drmp3_bool32 ret =
            drmp3_init_memory(&mp3, sampleFile->Buffer.get()->data(), sampleFile->Buffer.get()->size(), nullptr);
        numFrames = drmp3_get_pcm_frame_count(&mp3);
        drwav_uint64 channels = mp3.channels;
        drwav_uint64 sampleRate = mp3.sampleRate;

        audioSample->audioSampleData.reserve(numFrames * channels * 2); // 2 for sizeof(s16)
        drmp3_read_pcm_frames_s16(&mp3, numFrames, (int16_t*)audioSample->audioSampleData.data());
    } else {
        audioSample->audioSampleData.reserve(size);
        for (uint32_t i = 0; i < size; i++) {
            audioSample->audioSampleData.push_back((char)(sampleFile->Buffer.get()->at(i)));
        }
    }
    audioSample->sample.sampleAddr = audioSample->audioSampleData.data();

    return audioSample;
}
#include <cassert>
uint8_t ResourceFactoryXMLAudioSampleV0::CodecStrToInt(const char* str) {
    if (strcmp("ADPCM", str) == 0) {
        return 0;
    } else if (strcmp("S8", str) == 0) {
        return 1;
    } else if (strcmp("S16MEM", str) == 0) {
        return 2;
    } else if (strcmp("ADPCMSMALL", str) == 0) {
        return 3;
    } else if (strcmp("REVERB", str) == 0) {
        return 4;
    } else if (strcmp("S16", str) == 0) {
        return 5;
    } else if (strcmp("UNK6", str) == 0) {
        return 6;
    } else if (strcmp("UNK7", str) == 0) {
        return 7;
    } else {
        assert(0);

    }
}
} // namespace SOH

/*
in ResourceMgr_LoadAudioSample we used to have
--------------
    if (cachedCustomSFs.find(path) != cachedCustomSFs.end())
        return cachedCustomSFs[path];

    SoundFontSample* cSample = ReadCustomSample(path);

    if (cSample != nullptr)
        return cSample;
--------------
before the rest of the standard sample reading, this is the ReadCustomSample code we used to have

extern "C" SoundFontSample* ReadCustomSample(const char* path) {

    if (!ExtensionCache.contains(path))
        return nullptr;

    ExtensionEntry entry = ExtensionCache[path];

    auto sampleRaw = Ship::Context::GetInstance()->GetResourceManager()->LoadFile(entry.path);
    uint32_t* strem = (uint32_t*)sampleRaw->Buffer.get();
    uint8_t* strem2 = (uint8_t*)strem;

    SoundFontSample* sampleC = new SoundFontSample;

    if (entry.ext == "wav") {
        drwav_uint32 channels;
        drwav_uint32 sampleRate;
        drwav_uint64 totalPcm;
        drmp3_int16* pcmData =
            drwav_open_memory_and_read_pcm_frames_s16(strem2, sampleRaw->BufferSize, &channels, &sampleRate, &totalPcm,
NULL); sampleC->size = totalPcm; sampleC->sampleAddr = (uint8_t*)pcmData; sampleC->codec = CODEC_S16;

        sampleC->loop = new AdpcmLoop;
        sampleC->loop->start = 0;
        sampleC->loop->end = sampleC->size - 1;
        sampleC->loop->count = 0;
        sampleC->sampleRateMagicValue = 'RIFF';
        sampleC->sampleRate = sampleRate;

        cachedCustomSFs[path] = sampleC;
        return sampleC;
    } else if (entry.ext == "mp3") {
        drmp3_config mp3Info;
        drmp3_uint64 totalPcm;
        drmp3_int16* pcmData =
            drmp3_open_memory_and_read_pcm_frames_s16(strem2, sampleRaw->BufferSize, &mp3Info, &totalPcm, NULL);

        sampleC->size = totalPcm * mp3Info.channels * sizeof(short);
        sampleC->sampleAddr = (uint8_t*)pcmData;
        sampleC->codec = CODEC_S16;

        sampleC->loop = new AdpcmLoop;
        sampleC->loop->start = 0;
        sampleC->loop->end = sampleC->size;
        sampleC->loop->count = 0;
        sampleC->sampleRateMagicValue = 'RIFF';
        sampleC->sampleRate = mp3Info.sampleRate;

        cachedCustomSFs[path] = sampleC;
        return sampleC;
    }

    return nullptr;
}

*/
