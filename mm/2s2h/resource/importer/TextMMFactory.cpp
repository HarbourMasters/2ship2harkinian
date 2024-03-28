#include "2s2h/resource/importer/TextMMFactory.h"
#include "2s2h/resource/type/TextMM.h"
#include "spdlog/spdlog.h"

namespace SOH {
std::shared_ptr<LUS::IResource> ResourceFactoryBinaryTextMMV0::ReadResource(std::shared_ptr<LUS::File> file) {
    if (!FileHasValidFormatAndReader(file)) {
        return nullptr;
    }

    auto text = std::make_shared<TextMM>(file->InitData);
    auto reader = std::get<std::shared_ptr<LUS::BinaryReader>>(file->Reader);

    const uint32_t msgCount = reader->ReadUInt32();
    text->messages.reserve(msgCount);

    for (uint32_t i = 0; i < msgCount; i++) {
        MessageEntryMM entry;
        entry.id = reader->ReadUInt16();
        entry.textboxType = reader->ReadUByte();
        entry.textboxYPos = reader->ReadUByte();
        entry.icon = reader->ReadUByte();
        entry.nextMessageID = reader->ReadUInt16();
        entry.firstItemCost = reader->ReadUInt16();
        entry.secondItemCost = reader->ReadUInt16();
        entry.msg = reader->ReadString();

        text->messages.push_back(entry);
    }

    return text;
}

std::shared_ptr<LUS::IResource> ResourceFactoryXMLTextMMV0::ReadResource(std::shared_ptr<LUS::File> file) {
    if (!FileHasValidFormatAndReader(file)) {
        return nullptr;
    }

    auto text = std::make_shared<TextMM>(file->InitData);
    auto reader = std::get<std::shared_ptr<tinyxml2::XMLDocument>>(file->Reader)->FirstChildElement();

    auto child = reader->FirstChildElement();

    while (child != nullptr) {
        std::string childName = child->Name();

        if (childName == "TextEntry") {
            MessageEntryMM entry;
            entry.id = child->IntAttribute("ID");
            entry.textboxType = child->IntAttribute("TextboxType");
            entry.textboxYPos = child->IntAttribute("TextboxYPos");

            // BENTODO: MM Unique Fields

            entry.msg = child->Attribute("Message");
            entry.msg += "\x2";

            text->messages.push_back(entry);
            int bp = 0;
        }

        child = child->NextSiblingElement();
    }

    return text;
}
} // namespace LUS
