#include "2s2h/resource/importer/AudioSequenceFactory.h"
#include "2s2h/resource/type/AudioSequence.h"
#include "BinaryWriter.h"
#include "StringHelper.h"
#include "libultraship/libultraship.h"
#include "BinaryWriter.h"
#include <type_traits>


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

template <typename T>
void ResourceFactoryXMLAudioSequenceV0::WriteInsnOneArg(Ship::BinaryWriter* writer, uint8_t opcode, T arg) {
    static_assert(std::is_fundamental<T>::value);
    writer->Write(opcode);
    writer->Write(arg);
}

template <typename T1, typename T2>
void ResourceFactoryXMLAudioSequenceV0::WriteInsnTwoArg(Ship::BinaryWriter* writer, uint8_t opcode, T1 arg1, T2 arg2) {
    static_assert(std::is_fundamental<T1>::value && std::is_fundamental<T2>::value);
    writer->Write(opcode);
    writer->Write(arg1);
    writer->Write(arg2);
}

template <typename T1, typename T2, typename T3>
void ResourceFactoryXMLAudioSequenceV0::WriteInsnThreeArg(Ship::BinaryWriter* writer, uint8_t opcode, T1 arg1, T2 arg2, T3 arg3) {
    static_assert(std::is_fundamental<T1>::value && std::is_fundamental<T2>::value);
    writer->Write(opcode);
    writer->Write(arg1);
    writer->Write(arg2);
    writer->Write(arg3);
}

void ResourceFactoryXMLAudioSequenceV0::WriteInsnNoArg(Ship::BinaryWriter* writer, uint8_t opcode) {
    writer->Write(opcode);
}

void ResourceFactoryXMLAudioSequenceV0::WriteMuteBhv(Ship::BinaryWriter* writer, uint8_t arg) {
    WriteInsnOneArg(writer, 0xD3, arg);
}

void ResourceFactoryXMLAudioSequenceV0::WriteMuteScale(Ship::BinaryWriter* writer, uint8_t arg) {
    WriteInsnOneArg(writer, 0xD5, arg);
}

void ResourceFactoryXMLAudioSequenceV0::WriteInitchan(Ship::BinaryWriter* writer, uint16_t channels) {
    WriteInsnOneArg(writer, 0xD7, channels);
}

void ResourceFactoryXMLAudioSequenceV0::WriteLdchan(Ship::BinaryWriter* writer, uint8_t channel, uint16_t offset) {
    WriteInsnOneArg(writer, 0x90 | channel, offset);
}

void ResourceFactoryXMLAudioSequenceV0::WriteVolSHeader(Ship::BinaryWriter* writer, uint8_t vol) {
    WriteInsnOneArg(writer, 0xDB, vol);
}

void ResourceFactoryXMLAudioSequenceV0::WriteVolCHeader(Ship::BinaryWriter* writer, uint8_t vol) {
    WriteInsnOneArg(writer, 0xDF, vol);
}

void ResourceFactoryXMLAudioSequenceV0::WriteTempo(Ship::BinaryWriter* writer, uint8_t tempo) {
    WriteInsnOneArg(writer, 0xDD, tempo);
}

void ResourceFactoryXMLAudioSequenceV0::WriteJump(Ship::BinaryWriter* writer, uint16_t offset) {
    WriteInsnOneArg(writer, 0xFB, offset);
}

void ResourceFactoryXMLAudioSequenceV0::WriteDisablecan(Ship::BinaryWriter* writer, uint16_t channels) {
    WriteInsnOneArg(writer, 0xD6, channels);
}

void ResourceFactoryXMLAudioSequenceV0::WriteNoshort(Ship::BinaryWriter* writer) {
    WriteInsnNoArg(writer, 0xC4);
}

void ResourceFactoryXMLAudioSequenceV0::WriteLdlayer(Ship::BinaryWriter* writer, uint8_t layer, uint16_t offset) {
    WriteInsnOneArg(writer, 0x88 | layer, offset);
}

void ResourceFactoryXMLAudioSequenceV0::WritePan(Ship::BinaryWriter* writer, uint8_t pan) {
    WriteInsnOneArg(writer, 0xDD, pan);
}

void ResourceFactoryXMLAudioSequenceV0::WriteBend(Ship::BinaryWriter* writer, uint8_t bend) {
    WriteInsnOneArg(writer, 0xD3, bend);
}

void ResourceFactoryXMLAudioSequenceV0::WriteInstrument(Ship::BinaryWriter* writer, uint8_t instrument) {
    WriteInsnOneArg(writer, 0xC1, instrument);
}

void ResourceFactoryXMLAudioSequenceV0::WriteTranspose(Ship::BinaryWriter* writer, int8_t transpose) {
    WriteInsnOneArg(writer, 0xC2, transpose);
}

template <typename T> void ResourceFactoryXMLAudioSequenceV0::WriteDelay(Ship::BinaryWriter* writer, T delay) {
    WriteInsnOneArg(writer, 0xFD, delay);
}

template <typename T> void ResourceFactoryXMLAudioSequenceV0::WriteLDelay(Ship::BinaryWriter* writer, T delay) {
    WriteInsnOneArg(writer, 0xC0, delay);
}

template <typename T>
void ResourceFactoryXMLAudioSequenceV0::WriteNotedv(Ship::BinaryWriter* writer, uint8_t note, T delay,
                                                    uint8_t velocity) {
    WriteInsnTwoArg(writer, note, delay, velocity);
}
template <typename T>
void ResourceFactoryXMLAudioSequenceV0::WriteNotedvg(Ship::BinaryWriter* writer, uint8_t note, T delay, uint8_t velocity, uint8_t gateTime) {
    WriteInsnThreeArg(writer, note, delay, velocity, gateTime);
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
            Ship::BinaryWriter writer = Ship::BinaryWriter();
            writer.SetEndianness(Ship::Endianness::Big);
            // Placeholder off is the offset of the instruction to be replaced. The second variable is the target adress
            // of what we need to load of jump to
            uint16_t loopPoint;
            uint16_t channelPlaceholderOff, channelStart;
            uint16_t layerPlaceholderOff, layerStart;

            // Write seq header
            WriteMuteBhv(&writer, 0x20);
            WriteMuteScale(&writer, 0x32);
            WriteInitchan(&writer, 1);
            channelPlaceholderOff = writer.GetBaseAddress();
            loopPoint = writer.GetBaseAddress();
            WriteLdchan(&writer, 0, 0); // Fill in the actual address later
            WriteVolSHeader(&writer, 127); // Max volume
            WriteTempo(&writer, 0xA); // 1 BPM

            WriteDelay(&writer, static_cast<uint8_t>(0xFFFF));
            WriteJump(&writer, loopPoint);
            WriteDisablecan(&writer, 1);
            writer.Write(static_cast<uint8_t>(0xFF));

            // Fill in the ldchan from before
            channelStart = writer.GetBaseAddress();
            writer.Seek(channelPlaceholderOff, Ship::SeekOffsetType::Start);
            WriteLdchan(&writer, 0, channelStart);
            writer.Seek(channelStart, Ship::SeekOffsetType::Start);

            // Channel header
            WriteNoshort(&writer);
            layerPlaceholderOff = writer.GetBaseAddress();
            WriteLdlayer(&writer, 0, 0);
            WritePan(&writer, 64);
            WriteVolCHeader(&writer, 127); // Max volume
            WriteBend(&writer, 0);
            WriteInstrument(&writer, 0);
            // Max delay. ~113 minutes.
            WriteDelay(&writer, static_cast<uint8_t>(0xFFFF));
            writer.Write(static_cast<uint8_t>(0xFF));

            layerStart = writer.GetBaseAddress();
            writer.Seek(layerPlaceholderOff, Ship::SeekOffsetType::Start);
            WriteLdlayer(&writer, 0, layerStart);
            writer.Seek(layerStart, Ship::SeekOffsetType::Start);

            // Note layer
            WriteTranspose(&writer, 0);
            WriteNotedv(&writer, 39 + 0x40, static_cast<uint16_t>((0xFFFF) - 1), 127); // todo move the +40 into the function
            //WriteNotedvg(&writer, 39, static_cast<uint8_t>(0xC0), static_cast<uint8_t>(0x5F), static_cast<uint8_t>(0x2A));
            //WriteLDelay(&writer, static_cast<uint16_t>(0xc0));
            writer.Write(static_cast<uint8_t>(0xFF));

            sequence->sequenceData = writer.ToVector();
            sequence->sequence.seqData = sequence->sequenceData.data();
            sequence->sequence.numFonts = -1;
            sequence->sequence.seqDataSize = writer.ToVector().size();
        }
    }
    return sequence;
}
} // namespace SOH
