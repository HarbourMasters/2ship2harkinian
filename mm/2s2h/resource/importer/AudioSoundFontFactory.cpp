#include "2s2h/resource/importer/AudioSoundFontFactory.h"
#include "2s2h/resource/type/AudioSoundFont.h"
#include "spdlog/spdlog.h"
#include "libultraship/libultraship.h"
#include "StringHelper.h"

namespace SOH {
std::shared_ptr<Ship::IResource> ResourceFactoryBinaryAudioSoundFontV2::ReadResource(std::shared_ptr<Ship::File> file) {
    if (!FileHasValidFormatAndReader(file)) {
        return nullptr;
    }

    auto audioSoundFont = std::make_shared<AudioSoundFont>(file->InitData);
    auto reader = std::get<std::shared_ptr<Ship::BinaryReader>>(file->Reader);

    audioSoundFont->soundFont.fntIndex = reader->ReadInt32();
    audioSoundFont->medium = reader->ReadInt8();
    audioSoundFont->cachePolicy = reader->ReadInt8();

    audioSoundFont->data1 = reader->ReadUInt16();
    audioSoundFont->soundFont.sampleBankId1 = audioSoundFont->data1 >> 8;
    audioSoundFont->soundFont.sampleBankId2 = audioSoundFont->data1 & 0xFF;

    audioSoundFont->data2 = reader->ReadUInt16();
    audioSoundFont->data3 = reader->ReadUInt16();

    uint32_t drumCount = reader->ReadUInt32();
    audioSoundFont->soundFont.numDrums = drumCount;

    uint32_t instrumentCount = reader->ReadUInt32();
    audioSoundFont->soundFont.numInstruments = instrumentCount;

    uint32_t soundEffectCount = reader->ReadUInt32();
    audioSoundFont->soundFont.numSfx = soundEffectCount;

    // 🥁 DRUMS 🥁
    audioSoundFont->drums.reserve(audioSoundFont->soundFont.numDrums);
    audioSoundFont->drumAddresses.reserve(audioSoundFont->soundFont.numDrums);
    for (uint32_t i = 0; i < audioSoundFont->soundFont.numDrums; i++) {
        Drum drum;
        drum.releaseRate = reader->ReadUByte();
        drum.pan = reader->ReadUByte();
        drum.loaded = reader->ReadUByte();
        drum.loaded = 0; // this was always getting set to zero in ResourceMgr_LoadAudioSoundFont

        uint32_t envelopeCount = reader->ReadUInt32();
        audioSoundFont->drumEnvelopeCounts.push_back(envelopeCount);
        std::vector<AdsrEnvelope> drumEnvelopes;
        drumEnvelopes.reserve(audioSoundFont->drumEnvelopeCounts[i]);
        for (uint32_t j = 0; j < audioSoundFont->drumEnvelopeCounts.back(); j++) {
            AdsrEnvelope env;

            int16_t delay = reader->ReadInt16();
            int16_t arg = reader->ReadInt16();

            env.delay = BE16SWAP(delay);
            env.arg = BE16SWAP(arg);

            drumEnvelopes.push_back(env);
        }
        audioSoundFont->drumEnvelopeArrays.push_back(drumEnvelopes);
        drum.envelope = audioSoundFont->drumEnvelopeArrays.back().data();

        bool hasSample = reader->ReadInt8();
        std::string sampleFileName = reader->ReadString();
        drum.sound.tuning = reader->ReadFloat();

        if (sampleFileName.empty()) {
            drum.sound.sample = nullptr;
        } else {
            auto res = Ship::Context::GetInstance()->GetResourceManager()->LoadResourceProcess(sampleFileName.c_str());
            drum.sound.sample = static_cast<Sample*>(res ? res->GetRawPointer() : nullptr);
        }

        audioSoundFont->drums.push_back(drum);
        // BENTODO clean this up in V3.
        if (drum.sound.sample == nullptr) {
            audioSoundFont->drumAddresses.push_back(nullptr);
        } else {
            audioSoundFont->drumAddresses.push_back(&audioSoundFont->drums.back());
        }
    }
    audioSoundFont->soundFont.drums = audioSoundFont->drumAddresses.data();

    // 🎺🎻🎷🎸🎹 INSTRUMENTS 🎹🎸🎷🎻🎺
    audioSoundFont->instruments.reserve(audioSoundFont->soundFont.numInstruments);
    for (uint32_t i = 0; i < audioSoundFont->soundFont.numInstruments; i++) {
        Instrument instrument;

        uint8_t isValidEntry = reader->ReadUByte();
        instrument.loaded = reader->ReadUByte();
        instrument.loaded = 0; // this was always getting set to zero in ResourceMgr_LoadAudioSoundFont

        instrument.normalRangeLo = reader->ReadUByte();
        instrument.normalRangeHi = reader->ReadUByte();
        instrument.releaseRate = reader->ReadUByte();

        uint32_t envelopeCount = reader->ReadInt32();
        audioSoundFont->instrumentEnvelopeCounts.push_back(envelopeCount);
        std::vector<AdsrEnvelope> instrumentEnvelopes;
        for (uint32_t j = 0; j < audioSoundFont->instrumentEnvelopeCounts.back(); j++) {
            AdsrEnvelope env;

            int16_t delay = reader->ReadInt16();
            int16_t arg = reader->ReadInt16();

            env.delay = BE16SWAP(delay);
            env.arg = BE16SWAP(arg);

            instrumentEnvelopes.push_back(env);
        }
        audioSoundFont->instrumentEnvelopeArrays.push_back(instrumentEnvelopes);
        instrument.envelope = audioSoundFont->instrumentEnvelopeArrays.back().data();

        bool hasLowNoteSoundFontEntry = reader->ReadInt8();
        if (hasLowNoteSoundFontEntry) {
            bool hasSampleRef = reader->ReadInt8();
            std::string sampleFileName = reader->ReadString();
            instrument.lowNotesSound.tuning = reader->ReadFloat();
            auto res = Ship::Context::GetInstance()->GetResourceManager()->LoadResourceProcess(sampleFileName.c_str());
            instrument.lowNotesSound.sample = static_cast<Sample*>(res ? res->GetRawPointer() : nullptr);
        } else {
            instrument.lowNotesSound.sample = nullptr;
            instrument.lowNotesSound.tuning = 0;
        }

        bool hasNormalNoteSoundFontEntry = reader->ReadInt8();
        if (hasNormalNoteSoundFontEntry) {
            bool hasSampleRef = reader->ReadInt8();
            std::string sampleFileName = reader->ReadString();
            instrument.normalNotesSound.tuning = reader->ReadFloat();
            auto res = Ship::Context::GetInstance()->GetResourceManager()->LoadResourceProcess(sampleFileName.c_str());
            instrument.normalNotesSound.sample = static_cast<Sample*>(res ? res->GetRawPointer() : nullptr);
        } else {
            instrument.normalNotesSound.sample = nullptr;
            instrument.normalNotesSound.tuning = 0;
        }

        bool hasHighNoteSoundFontEntry = reader->ReadInt8();
        if (hasHighNoteSoundFontEntry) {
            bool hasSampleRef = reader->ReadInt8();
            std::string sampleFileName = reader->ReadString();
            instrument.highNotesSound.tuning = reader->ReadFloat();
            auto res = Ship::Context::GetInstance()->GetResourceManager()->LoadResourceProcess(sampleFileName.c_str());
            instrument.highNotesSound.sample = static_cast<Sample*>(res ? res->GetRawPointer() : nullptr);
        } else {
            instrument.highNotesSound.sample = nullptr;
            instrument.highNotesSound.tuning = 0;
        }

        audioSoundFont->instruments.push_back(instrument);
        audioSoundFont->instrumentAddresses.push_back(isValidEntry ? &audioSoundFont->instruments.back() : nullptr);
    }
    audioSoundFont->soundFont.instruments = audioSoundFont->instrumentAddresses.data();

    // 🔊 SOUND EFFECTS 🔊
    audioSoundFont->soundEffects.reserve(audioSoundFont->soundFont.numSfx);
    for (uint32_t i = 0; i < audioSoundFont->soundFont.numSfx; i++) {
        SoundFontSound soundEffect;

        bool hasSFEntry = reader->ReadInt8();
        if (hasSFEntry) {
            bool hasSampleRef = reader->ReadInt8();
            std::string sampleFileName = reader->ReadString();
            soundEffect.tuning = reader->ReadFloat();
            auto res = Ship::Context::GetInstance()->GetResourceManager()->LoadResourceProcess(sampleFileName.c_str());
            soundEffect.sample = static_cast<Sample*>(res ? res->GetRawPointer() : nullptr);
        }

        audioSoundFont->soundEffects.push_back(soundEffect);
    }
    audioSoundFont->soundFont.soundEffects = audioSoundFont->soundEffects.data();

    return audioSoundFont;
}

int8_t SOH::ResourceFactoryXMLSoundFontV0::MediumStrToInt(const char* str) {
    if (!strcmp("Ram", str)) {
        return 0;
    } else if (!strcmp("Unk", str)) {
        return 1;
    } else if (!strcmp("Cart", str)) {
        return 2;
    } else if (!strcmp("Disk", str)) {
        return 3;
        //4 is skipped
    } else if (!strcmp("RamUnloaded", str)) {
        return 5;
    } else {
        throw std::runtime_error(StringHelper::Sprintf("Bad medium value. Got %s, expected Ram, Unk, Cart, or Disk.", str));
    }
}

int8_t ResourceFactoryXMLSoundFontV0::CachePolicyToInt(const char* str) {
    if (!strcmp("Temporary", str)) {
        return 0;
    } else if (!strcmp("Persistent", str)) {
        return 1;
    } else if (!strcmp("Either", str)) {
        return 2;
    } else if (!strcmp("Permanent", str)) {
        return 3;
    } else {
        throw std::runtime_error(
            StringHelper::Sprintf("Bad cache policy value. Got %s, expected Temporary, Persistent, Either, or Permanent.", str));
    }
}

void ResourceFactoryXMLSoundFontV0::ParseDrums(AudioSoundFont* soundFont, tinyxml2::XMLElement* element) {
    element = (tinyxml2::XMLElement*)element->FirstChildElement();
    // No drums
    if (element == nullptr) {
        soundFont->soundFont.drums = nullptr;
        soundFont->soundFont.numDrums = 0;
        return;
    }
    do {
        Drum drum;
        std::vector<AdsrEnvelope> envelopes;
        drum.releaseRate = element->IntAttribute("ReleaseRate");
        drum.pan = element->IntAttribute("Pan");
        drum.loaded = element->IntAttribute("Loaded");
        drum.sound.tuning = element->FloatAttribute("Tuning");
        const char* sampleStr = element->Attribute("SampleRef");
        if (sampleStr != nullptr && sampleStr[0] != 0) {
            auto res = Ship::Context::GetInstance()->GetResourceManager()->LoadResourceProcess(sampleStr);
            drum.sound.sample = static_cast<Sample*>(res ? res->GetRawPointer() : nullptr);
        } else {
            drum.sound.sample = nullptr;
        }
        element = (tinyxml2::XMLElement*)element->FirstChildElement();
        if (!strcmp(element->Name(), "Envelopes")) {
            //element = (tinyxml2::XMLElement*)element->FirstChildElement();
            unsigned int envCount = 0;
            envelopes = ParseEnvelopes(soundFont, element, &envCount);
            element = (tinyxml2::XMLElement*)element->Parent();
            soundFont->drumEnvelopeArrays.push_back(envelopes);
            drum.envelope = soundFont->drumEnvelopeArrays.back().data();
        } else {
            drum.envelope = nullptr;
        }

        // BENTODO the binary importer does this not sure why... @jack or @kenix?
        soundFont->drums.push_back(drum);
        //if (drum.sound.sample == nullptr) {
        //    soundFont->drumAddresses.push_back(nullptr);
        //} else {
            soundFont->drumAddresses.push_back(&soundFont->drums.back());
        //}

        
        element = element->NextSiblingElement();
    } while (element != nullptr);
    soundFont->soundFont.numDrums = soundFont->drumAddresses.size();
    soundFont->soundFont.drums = soundFont->drumAddresses.data();
}

 void SOH::ResourceFactoryXMLSoundFontV0::ParseInstruments(AudioSoundFont* soundFont, tinyxml2::XMLElement* element) {
     element = element->FirstChildElement();
     do {
         Instrument instrument = { 0 };
         unsigned int envCount = 0;
         std::vector<AdsrEnvelope> envelopes;

         int isValid = element->BoolAttribute("IsValid");
         instrument.loaded = element->IntAttribute("Loaded");
         instrument.normalRangeLo = element->IntAttribute("NormalRangeLo");
         instrument.normalRangeHi = element->IntAttribute("NormalRangeHi");
         instrument.releaseRate = element->IntAttribute("ReleseRate"); // BENTODO fix the spelling
         tinyxml2::XMLElement* instrumentElement = element->FirstChildElement();
         tinyxml2::XMLElement* instrumentElementCopy = instrumentElement;
         if (instrumentElement != nullptr && !strcmp(instrumentElement->Name(), "Envelopes")) {
             envelopes = ParseEnvelopes(soundFont, instrumentElement, &envCount);
             soundFont->instrumentEnvelopeCounts.push_back(envCount);
             soundFont->instrumentEnvelopeArrays.push_back(envelopes);
             instrument.envelope = soundFont->instrumentEnvelopeArrays.back().data();
             instrumentElement = instrumentElement->NextSiblingElement();
         } 
         if (instrumentElement != nullptr && !strcmp("LowNotesSound", instrumentElement->Name())) {
             instrument.lowNotesSound.tuning = instrumentElement->FloatAttribute("Tuning");
             const char* sampleStr = instrumentElement->Attribute("SampleRef");
             if (sampleStr != nullptr && sampleStr[0] != 0) {
                 auto res = Ship::Context::GetInstance()->GetResourceManager()->LoadResourceProcess(sampleStr);
                 instrument.lowNotesSound.sample = static_cast<Sample*>(res ? res->GetRawPointer() : nullptr);
             }
             instrumentElement = instrumentElement->NextSiblingElement();
         } 
         if (instrumentElement != nullptr && !strcmp("NormalNotesSound", instrumentElement->Name())) {
             instrument.normalNotesSound.tuning = instrumentElement->FloatAttribute("Tuning");
             const char* sampleStr = instrumentElement->Attribute("SampleRef");
             if (sampleStr != nullptr && sampleStr[0] != 0) {
                 auto res = Ship::Context::GetInstance()->GetResourceManager()->LoadResourceProcess(sampleStr);
                 instrument.normalNotesSound.sample = static_cast<Sample*>(res ? res->GetRawPointer() : nullptr);
             }
             instrumentElement = instrumentElement->NextSiblingElement();
         } 
         if (instrumentElement != nullptr && !strcmp("HighNotesSound", instrumentElement->Name())) {
             instrument.highNotesSound.tuning = instrumentElement->FloatAttribute("Tuning");
             const char* sampleStr = instrumentElement->Attribute("SampleRef");
             if (sampleStr != nullptr && sampleStr[0] != 0) {
                 auto res = Ship::Context::GetInstance()->GetResourceManager()->LoadResourceProcess(sampleStr);
                 instrument.highNotesSound.sample = static_cast<Sample*>(res ? res->GetRawPointer() : nullptr);
             }
             instrumentElement = instrumentElement->NextSiblingElement();
         }
         element = instrumentElementCopy;
         element = (tinyxml2::XMLElement*)element->Parent();
         element = element->NextSiblingElement();
         soundFont->instruments.push_back(instrument);
         soundFont->instrumentAddresses.push_back(isValid ? &soundFont->instruments.back() : nullptr);
    } while (element != nullptr);
    soundFont->soundFont.instruments = soundFont->instrumentAddresses.data();
    soundFont->soundFont.numInstruments = soundFont->instrumentAddresses.size();
 }


void SOH::ResourceFactoryXMLSoundFontV0::ParseSfxTable(AudioSoundFont* soundFont, tinyxml2::XMLElement* element) {
    element = (tinyxml2::XMLElement*)element->FirstChildElement();

    while (element != nullptr) {
        SoundFontSound sound;
        sound.tuning = element->FloatAttribute("Tuning");
        const char* sampleStr = element->Attribute("SampleRef");
        if (sampleStr != nullptr && sampleStr[0] != 0) {
            auto res = Ship::Context::GetInstance()->GetResourceManager()->LoadResourceProcess(sampleStr);
            sound.sample = static_cast<Sample*>(res ? res->GetRawPointer() : nullptr);
        }
        element = element->NextSiblingElement();
        soundFont->soundEffects.push_back(sound);
    }
    soundFont->soundFont.soundEffects = soundFont->soundEffects.data();
    soundFont->soundFont.numSfx = soundFont->soundEffects.size();
}


 std::vector<AdsrEnvelope> SOH::ResourceFactoryXMLSoundFontV0::ParseEnvelopes(AudioSoundFont* soundFont,
                                                                           tinyxml2::XMLElement* element,
                                                                           unsigned int* count) {
    std::vector<AdsrEnvelope> envelopes;
    unsigned int total = 0;
    element = element->FirstChildElement("Envelope");

    while (element != nullptr) {
        AdsrEnvelope env = {
            .delay = (s16)element->IntAttribute("Delay"),
            .arg = (s16)element->IntAttribute("Arg"),
        };
        env.delay = BSWAP16(env.delay);
        env.arg = BSWAP16(env.arg);
        envelopes.emplace_back(env);
        element = element->NextSiblingElement("Envelope");
        total++;
    } 
    *count = total;
    return envelopes;
}

std::shared_ptr<Ship::IResource> ResourceFactoryXMLSoundFontV0::ReadResource(std::shared_ptr<Ship::File> file) {
    if (!FileHasValidFormatAndReader(file)) {
        return nullptr;
    }

    auto audioSoundFont = std::make_shared<AudioSoundFont>(file->InitData);
    auto child =
        std::get<std::shared_ptr<tinyxml2::XMLDocument>>(file->Reader)->FirstChildElement();
    // Header data
    memset(&audioSoundFont->soundFont, 0, sizeof(audioSoundFont->soundFont));
    audioSoundFont->soundFont.fntIndex = child->IntAttribute("Num", 0);
    if (audioSoundFont->soundFont.fntIndex == 1) {
        int bp = 0;
    }
    const char* mediumStr = child->Attribute("Medium");
    audioSoundFont->medium = MediumStrToInt(mediumStr);

    const char* cachePolicyStr = child->Attribute("CachePolicy");
    audioSoundFont->cachePolicy = CachePolicyToInt(cachePolicyStr);

    audioSoundFont->data1 = child->IntAttribute("Data1");
    audioSoundFont->data2 = child->IntAttribute("Data2");
    audioSoundFont->data3 = child->IntAttribute("Data3");

    audioSoundFont->soundFont.sampleBankId1 = audioSoundFont->data1 >> 8;
    audioSoundFont->soundFont.sampleBankId2 = audioSoundFont->data1 & 0xFF;

    child = (tinyxml2::XMLElement*)child->FirstChildElement();

    while (child != nullptr) {
        const char* name = child->Name();

        if (!strcmp(name, "Drums")) {
            ParseDrums(audioSoundFont.get(), child);
        } else if (!strcmp(name, "Instruments")) {
            ParseInstruments(audioSoundFont.get(), child);
        } else if (!strcmp(name, "SfxTable")) {
            ParseSfxTable(audioSoundFont.get(), child);
        }
        child = child->NextSiblingElement();
    }

    return audioSoundFont;
}


} // namespace SOH
