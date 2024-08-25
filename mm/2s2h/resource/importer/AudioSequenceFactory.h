#pragma once

#include "Resource.h"
#include "ResourceFactoryBinary.h"
#include "ResourceFactoryXML.h"
#include "BinaryWriter.h"

namespace SOH {
class ResourceFactoryBinaryAudioSequenceV2 : public Ship::ResourceFactoryBinary {
  public:
    std::shared_ptr<Ship::IResource> ReadResource(std::shared_ptr<Ship::File> file) override;
};

class ResourceFactoryXMLAudioSequenceV0 : public Ship::ResourceFactoryXML {
  public:
    std::shared_ptr<Ship::IResource> ReadResource(std::shared_ptr<Ship::File> file) override;

  private:
    int8_t MediumStrToInt(const char* str);
    int8_t CachePolicyToInt(const char* str);
    static void WriteInsnNoArg(Ship::BinaryWriter* writer, uint8_t opcode);
    template<typename T> static void WriteInsnOneArg(Ship::BinaryWriter* writer, uint8_t opcode, T arg);
    template <typename T1, typename T2>
    static void WriteInsnTwoArg(Ship::BinaryWriter* writer, uint8_t opcode, T1 arg1, T2 arg2);
    static void WriteMuteBhv(Ship::BinaryWriter* writer, uint8_t arg);
    static void WriteMuteScale(Ship::BinaryWriter* writer, uint8_t arg);
    static void WriteInitchan(Ship::BinaryWriter* writer, uint16_t channels);
    static void WriteLdchan(Ship::BinaryWriter* writer, uint8_t channel, uint16_t offset);
    static void WriteVolSHeader(Ship::BinaryWriter* writer, uint8_t vol);
    static void WriteVolCHeader(Ship::BinaryWriter* writer, uint8_t vol);
    static void WriteTempo(Ship::BinaryWriter* writer, uint8_t tempo);
    template <typename T> static void WriteDelay(Ship::BinaryWriter* writer, T delay);
    template <typename T> static void WriteLDelay(Ship::BinaryWriter* writer, T delay);
    static void WriteJump(Ship::BinaryWriter* writer, uint16_t offset);
    static void WriteDisablecan(Ship::BinaryWriter* writer, uint16_t channels);
    static void WriteNoshort(Ship::BinaryWriter* writer);
    static void WriteLdlayer(Ship::BinaryWriter* writer, uint8_t layer, uint16_t offset);
    static void WritePan(Ship::BinaryWriter* writer, uint8_t pan);
    static void WriteBend(Ship::BinaryWriter* writer, uint8_t bend);
    static void WriteInstrument(Ship::BinaryWriter* writer, uint8_t instrument);
    static void WriteTranspose(Ship::BinaryWriter* writer, int8_t transpose);
    template <typename T> static void WriteNotedv(Ship::BinaryWriter* writer, uint8_t note, T delay, uint8_t velocity);

};

} // namespace SOH
