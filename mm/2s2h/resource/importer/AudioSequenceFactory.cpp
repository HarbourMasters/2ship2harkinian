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

    audioSequence->sequence.seqDataSize = reader->ReadUInt32();
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
static void WriteInsnOneArg(Ship::BinaryWriter* writer, uint8_t opcode, T arg) {
    static_assert(std::is_fundamental<T>::value);
    writer->Write(opcode);
    writer->Write(arg);
}

template <typename T1, typename T2>
static void WriteInsnTwoArg(Ship::BinaryWriter* writer, uint8_t opcode, T1 arg1, T2 arg2) {
    static_assert(std::is_fundamental<T1>::value && std::is_fundamental<T2>::value);
    writer->Write(opcode);
    writer->Write(arg1);
    writer->Write(arg2);
}

template <typename T1, typename T2, typename T3>
static void WriteInsnThreeArg(Ship::BinaryWriter* writer, uint8_t opcode, T1 arg1, T2 arg2, T3 arg3) {
    static_assert(std::is_fundamental<T1>::value && std::is_fundamental<T2>::value);
    writer->Write(opcode);
    writer->Write(arg1);
    writer->Write(arg2);
    writer->Write(arg3);
}

static void WriteInsnNoArg(Ship::BinaryWriter* writer, uint8_t opcode) {
    writer->Write(opcode);
}


static void WriteLegato(Ship::BinaryWriter* writer) {
    WriteInsnNoArg(writer, 0xC4);
}

static void WriteNoLegato(Ship::BinaryWriter* writer) {
    WriteInsnNoArg(writer, 0xC5);
}

static void WriteMuteBhv(Ship::BinaryWriter* writer, uint8_t arg) {
    WriteInsnOneArg(writer, 0xD3, arg);
}

static void WriteMuteScale(Ship::BinaryWriter* writer, uint8_t arg) {
    WriteInsnOneArg(writer, 0xD5, arg);
}

static void WriteInitchan(Ship::BinaryWriter* writer, uint16_t channels) {
    WriteInsnOneArg(writer, 0xD7, channels);
}

static void WriteLdchan(Ship::BinaryWriter* writer, uint8_t channel, uint16_t offset) {
    WriteInsnOneArg(writer, 0x90 | channel, offset);
}

static void WriteVolSHeader(Ship::BinaryWriter* writer, uint8_t vol) {
    WriteInsnOneArg(writer, 0xDB, vol);
}

static void WriteVolCHeader(Ship::BinaryWriter* writer, uint8_t vol) {
    WriteInsnOneArg(writer, 0xDF, vol);
}

static void WriteTempo(Ship::BinaryWriter* writer, uint8_t tempo) {
    WriteInsnOneArg(writer, 0xDD, tempo);
}

static void WriteJump(Ship::BinaryWriter* writer, uint16_t offset) {
    WriteInsnOneArg(writer, 0xFB, offset);
}

static void WriteDisablecan(Ship::BinaryWriter* writer, uint16_t channels) {
    WriteInsnOneArg(writer, 0xD6, channels);
}

static void WriteNoshort(Ship::BinaryWriter* writer) {
    WriteInsnNoArg(writer, 0xC4);
}

static void WriteLdlayer(Ship::BinaryWriter* writer, uint8_t layer, uint16_t offset) {
    WriteInsnOneArg(writer, 0x88 | layer, offset);
}

static void WritePan(Ship::BinaryWriter* writer, uint8_t pan) {
    WriteInsnOneArg(writer, 0xDD, pan);
}

static void WriteBend(Ship::BinaryWriter* writer, uint8_t bend) {
    WriteInsnOneArg(writer, 0xD3, bend);
}

static void WriteInstrument(Ship::BinaryWriter* writer, uint8_t instrument) {
    WriteInsnOneArg(writer, 0xC1, instrument);
}

static void WriteTranspose(Ship::BinaryWriter* writer, int8_t transpose) {
    WriteInsnOneArg(writer, 0xC2, transpose);
}

static void WriteDelay(Ship::BinaryWriter* writer, uint16_t delay) {
    if (delay > 0x7F) {
        WriteInsnOneArg(writer, 0xFD, static_cast<uint16_t>(delay | 0x8000));
    } else {
        WriteInsnOneArg(writer, 0xFD, static_cast<uint8_t>(delay));
    }
}

template <typename T> 
static void WriteLDelay(Ship::BinaryWriter* writer, T delay) {
    WriteInsnOneArg(writer, 0xC0, delay);
}

template <typename T>
static void WriteNotedv(Ship::BinaryWriter* writer, uint8_t note, T delay,
                                                    uint8_t velocity) {
    WriteInsnTwoArg(writer, note, delay, velocity);
}

static void WriteNotedvg(Ship::BinaryWriter* writer, uint8_t note, uint16_t delay, uint8_t velocity, uint8_t gateTime) {
    if (delay > 0x7F) {
        WriteInsnThreeArg(writer, note, static_cast<uint16_t>(delay | 0x8000), velocity, gateTime);
    } else {
        WriteInsnThreeArg(writer, note, static_cast<uint8_t>(delay), velocity, gateTime);
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
        // setting numFonts to -1 tells the game's audio engine the sound font to used is CRC64 encoded in the font indicies.
        sequence->sequence.numFonts = -1;
        if (path != nullptr) {
            sequence->sequenceData = *seqFile->Buffer.get();
            sequence->sequence.seqData = sequence->sequenceData.data();
            sequence->sequence.seqDataSize = seqFile->Buffer.get()->size();
        }
        else {
            unsigned int length = child->UnsignedAttribute("Length");
            bool looped = child->BoolAttribute("Looped", true);
            Ship::BinaryWriter writer = Ship::BinaryWriter();
            writer.SetEndianness(Ship::Endianness::Big);
            // Placeholder off is the offset of the instruction to be replaced. The second variable is the target adress
            // of what we need to load of jump to
            uint16_t loopPoint;
            uint16_t channelPlaceholderOff, channelStart;
            uint16_t layerPlaceholderOff, layerStart;

            // 1 second worth of ticks can be found by using `ticks = 60 / (bpm * 48)`
            // Get the number of ticks per second and then divide the length by this number to get the number of ticks for the song.
            constexpr uint8_t TEMPO = 1;
            constexpr float TEMPO_F = TEMPO;
            // Use floats for this first calculation so we can round up
            float delayF = length / (60.0f / (TEMPO_F * 48.0f));
            // Convert to u16. This way this value is encoded changes depending on the value.
            // It can be at most 0xFFFF so store it in a u16 for now.
            uint16_t delay;
            if(delayF >= 65535.0f) {
                delay = 0x7FFF;
            } else {
                delay = delayF;
            }
            
            // Write seq header

            // These two values are always the same in OOT and MM
            WriteMuteBhv(&writer, 0x20);
            WriteMuteScale(&writer, 0x32);

            // We only have one channel
            WriteInitchan(&writer, 1);
            // Store the current position so we can write the address of the channel when we are ready.
            channelPlaceholderOff = writer.GetBaseAddress();
            // Store the current position so we can loop here after the song ends.
            loopPoint = writer.GetBaseAddress();
            WriteLdchan(&writer, 0, 0); // Fill in the actual address later

            WriteVolSHeader(&writer, 127); // Max volume
            WriteTempo(&writer, TEMPO);

            WriteDelay(&writer, delay);
            if (looped) {
                WriteJump(&writer, loopPoint);
            }
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
            WriteDelay(&writer, delay);
            writer.Write(static_cast<uint8_t>(0xFF));

            layerStart = writer.GetBaseAddress();
            writer.Seek(layerPlaceholderOff, Ship::SeekOffsetType::Start);
            WriteLdlayer(&writer, 0, layerStart);
            writer.Seek(layerStart, Ship::SeekOffsetType::Start);

            // Note layer
            WriteTranspose(&writer, 0);
            WriteLegato(&writer);
            WriteNotedvg(&writer, 39, 0x7FFF - 1, static_cast<uint8_t>(0x7F), static_cast<uint8_t>(1));

            writer.Write(static_cast<uint8_t>(0xFF));

            sequence->sequenceData = writer.ToVector();
            sequence->sequence.seqData = sequence->sequenceData.data();
            sequence->sequence.seqDataSize = writer.ToVector().size();
        }
    }

    return sequence;
}
} // namespace SOH
