#include "2s2h/resource/importer/AudioSequenceFactory.h"
#include "2s2h/resource/type/AudioSequence.h"
#include "BinaryWriter.h"
#include "spdlog/spdlog.h"
#include "StringHelper.h"
#include "libultraship/libultraship.h"


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
    bool streamed = child->BoolAttribute("Streamed");

    
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
    std::shared_ptr<Ship::File> seqFile;
    if (path != nullptr) {
        initData->Path = path;
        initData->IsCustom = false;
        initData->ByteOrder = Ship::Endianness::Native;
        seqFile = Ship::Context::GetInstance()->GetResourceManager()->GetArchiveManager()->LoadFile(path, initData);
    }

    if (!streamed) {
        sequence->sequenceData = *seqFile->Buffer.get();
        sequence->sequence.seqData = sequence->sequenceData.data();
    } else {
        if (path != nullptr) {
            sequence->sequenceData = *seqFile->Buffer.get();
            sequence->sequence.seqData = sequence->sequenceData.data();
            sequence->sequence.numFonts = -1;
            sequence->sequence.seqDataSize = seqFile->Buffer.get()->size();
        }
        else {
            // Build the sequence manually
        }
        #if 0
        Ship::BinaryWriter writer = Ship::BinaryWriter();

        drwav wav;
        drwav_uint64 numFrames;
        
        drwav_bool32 ret = drwav_init_memory(&wav, seqFile->Buffer.get()->data(), seqFile->Buffer.get()->size(), nullptr);

        drwav_get_length_in_pcm_frames(&wav, &numFrames);

        sequence->sampleRate = wav.fmt.sampleRate;
        sequence->numChannels = wav.fmt.channels;
        sequence->customSeqData = std::make_unique<short[]>(numFrames);
        drwav_read_pcm_frames_s16(&wav, numFrames, sequence->customSeqData.get());
        
        sequence->sequence.seqData = (char*)sequence->customSeqData.get();
        // mutebhv 0x20
        writer.Write((uint8_t)0xD3); // 0x0
        writer.Write((uint8_t)0x20); // 0x1

        // mutescale 50
        writer.Write((uint8_t)0xD5); // 0x2
        writer.Write((uint8_t)50);   // 0x3

        // initchan 1 (only channel 1)
        writer.Write((uint8_t)0xD7); // 0x4
        writer.Write((uint16_t)1);   // 0x5

        // ldchan 1, placeholder
        writer.Write((uint8_t)0x90 | 1); // 0x7
        writer.Write((uint16_t)0x0000);// 0x8 Fill it in later

        //vol 100
        writer.Write((uint8_t)0xDF);    //0xA
        writer.Write((uint8_t)100);     //0xB

        // tempo
        writer.Write((uint8_t)0xDD);    //0xC
        writer.Write((uint8_t)120);     //0xD

        // delay 0x540
        writer.Write((uint8_t)0xFD);    //0xE
        writer.Write((uint8_t)0x7F);    //0xF  

        // freechan 1 (only channel 1)
        writer.Write((uint8_t)0xD5);   //0x10
        writer.Write((uint16_t)1);     //0x11
        writer.Write(0xFF);            //0x12

        // Go back and write the channel address
        size_t curPos = writer.GetBaseAddress();
        writer.Seek(0x8, Ship::SeekOffsetType::Start);
        writer.Write((uint16_t)curPos);
        writer.Seek(curPos, Ship::SeekOffsetType::Start);



        // Write a marker to show this is a streamed audio file
        memcpy(sequence->sequence.fonts, "07151129", sizeof("07151129"));
        drwav_uninit(&wav);
        #endif
    }
    return sequence;
}
} // namespace SOH
