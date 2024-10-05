#include "2s2h/resource/importer/AudioSampleFactory.h"
#include "2s2h/resource/type/AudioSample.h"
#include "2s2h/resource/importer/AudioSoundFontFactory.h"
#include "StringHelper.h"
#include "libultraship/libultraship.h"
#define DR_WAV_IMPLEMENTATION
#include "2s2h/Enhancements/Audio/dr_libs/dr_wav.h"

#define DR_MP3_IMPLEMENTATION
#include "2s2h/Enhancements/Audio/dr_libs/dr_mp3.h"

#define DR_FLAC_IMPLEMENTATION
#include "2s2h/Enhancements/Audio/dr_libs/dr_flac.h"

#include "vorbis/vorbisfile.h"

struct OggFileData {
    void* data;
    size_t pos;
    size_t size;
};

static size_t VorbisReadCallback(void* out, size_t size, size_t elems, void* src) {
    OggFileData* data = static_cast<OggFileData*>(src);
    size_t toRead = size * elems;

    if (toRead > data->size - data->pos) {
        toRead = data->size - data->pos;
    }

    memcpy(out, static_cast<uint8_t*>(data->data) + data->pos, toRead);
    data->pos += toRead;

    return toRead / size;
}

static int VorbisSeekCallback(void* src, ogg_int64_t pos, int whence) {
    OggFileData* data = static_cast<OggFileData*>(src);
    size_t newPos;

    switch (whence) {
        case SEEK_SET:
            newPos = pos;
            break;
        case SEEK_CUR:
            newPos = data->pos + pos;
            break;
        case SEEK_END:
            newPos = data->size + pos;
            break;
        default:
            return -1;
    }
    if (newPos > data->size) {
        return -1;
    }
    data->pos = newPos;
    return 0;
}

static int VorbisCloseCallback([[maybe_unused]] void* src) {
    return 0;
}

static long VorbisTellCallback(void* src) {
    OggFileData* data = static_cast<OggFileData*>(src);
    return data->pos;
}

static const ov_callbacks vorbisCallbacks = {
    VorbisReadCallback,
    VorbisSeekCallback,
    VorbisCloseCallback,
    VorbisTellCallback,
};

static void Mp3DecoderWorker(std::shared_ptr<SOH::AudioSample> audioSample, std::shared_ptr<Ship::File> sampleFile) {
    drmp3 mp3;
    drwav_uint64 numFrames;
    drmp3_bool32 ret =
        drmp3_init_memory(&mp3, sampleFile->Buffer.get()->data(), sampleFile->Buffer.get()->size(), nullptr);
    numFrames = drmp3_get_pcm_frame_count(&mp3);
    drwav_uint64 channels = mp3.channels;
    drwav_uint64 sampleRate = mp3.sampleRate;

    audioSample->audioSampleData.reserve(numFrames * channels * 2); // 2 for sizeof(s16)
    drmp3_read_pcm_frames_s16(&mp3, numFrames, (int16_t*)audioSample->audioSampleData.data());
    audioSample->sample.sampleAddr = audioSample->audioSampleData.data();
}

static void FlacDecoderWorker(std::shared_ptr<SOH::AudioSample> audioSample, std::shared_ptr<Ship::File> sampleFile) {
    drflac* flac = drflac_open_memory(sampleFile->Buffer.get()->data(), sampleFile->Buffer.get()->size(), nullptr);
    drflac_uint64 numFrames = flac->totalPCMFrameCount;
    audioSample->audioSampleData.reserve(numFrames * flac->channels * 2);
    drflac_read_pcm_frames_s16(flac, numFrames, (int16_t*)audioSample->audioSampleData.data());
    audioSample->sample.sampleAddr = audioSample->audioSampleData.data();
}

static void OggDecoderWorker(std::shared_ptr<SOH::AudioSample> audioSample, std::shared_ptr<Ship::File> sampleFile) {
    OggVorbis_File vf;
    char dataBuff[4096];
    long read = 0;
    size_t pos = 0;

    OggFileData fileData = {
        .data = sampleFile->Buffer.get()->data(),
        .pos = 0,
        .size = sampleFile->Buffer.get()->size(),
    };
    int ret = ov_open_callbacks(&fileData, &vf, nullptr, 0, vorbisCallbacks);

    vorbis_info* vi = ov_info(&vf, -1);

    uint64_t numFrames = ov_pcm_total(&vf, -1);
    uint64_t sampleRate = vi->rate;
    uint64_t numChannels = vi->channels;
    int bitStream = 0;
    size_t toRead = numFrames * numChannels * 2;
    audioSample->audioSampleData.reserve(toRead);
    do {
        read = ov_read(&vf, dataBuff, 4096, 0, 2, 1, &bitStream);
        memcpy(audioSample->audioSampleData.data() + pos, dataBuff, read);
        pos += read;
    } while (read != 0);
    audioSample->sample.sampleAddr = audioSample->audioSampleData.data();
    ov_clear(&vf);
}

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
    memset(audioSample->loop.state, 0, sizeof(audioSample->loop.state));
    if (audioSample->sample.codec == 0 || audioSample->sample.codec == 3) {
        tinyxml2::XMLElement* loopElement = child->FirstChildElement();
        size_t i = 0;
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
    // Compressed files can take a really long time to decode (~250ms per).
    // This worked when we tested it (09/04/2024) (Works on my machine)
    if (customFormatStr != nullptr && strcmp(customFormatStr, "wav") == 0) {
        drwav wav;
        drwav_uint64 numFrames;

        drwav_bool32 ret =
            drwav_init_memory(&wav, sampleFile->Buffer.get()->data(), sampleFile->Buffer.get()->size(), nullptr);

        drwav_get_length_in_pcm_frames(&wav, &numFrames);

        audioSample->tuning = (wav.sampleRate * wav.channels) / 32000.0f;
        audioSample->audioSampleData.reserve(numFrames * wav.channels * 2);

        drwav_read_pcm_frames_s16(&wav, numFrames, (int16_t*)audioSample->audioSampleData.data());
    } else if (customFormatStr != nullptr && strcmp(customFormatStr, "mp3") == 0) {
        std::thread fileDecoderThread = std::thread(Mp3DecoderWorker, audioSample, sampleFile);
        fileDecoderThread.detach();
        return audioSample;
    } else if (customFormatStr != nullptr && strcmp(customFormatStr, "ogg") == 0) {
        std::thread fileDecoderThread = std::thread(OggDecoderWorker, audioSample, sampleFile);
        fileDecoderThread.detach();
        return audioSample;
    } else if (customFormatStr != nullptr && strcmp(customFormatStr, "flac") == 0) {
        std::thread fileDecoderThread = std::thread(FlacDecoderWorker, audioSample, sampleFile);
        fileDecoderThread.detach();
        return audioSample;
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
