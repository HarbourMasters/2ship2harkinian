#include "2s2h/resource/importer/TextMMFactory.h"
#include "2s2h/resource/type/TextMM.h"
#include "spdlog/spdlog.h"

namespace LUS {
std::shared_ptr<IResource>
TextMMFactory::ReadResource(std::shared_ptr<ResourceInitData> initData, std::shared_ptr<BinaryReader> reader) {
    auto resource = std::make_shared<TextMM>(initData);
    std::shared_ptr<ResourceVersionFactory> factory = nullptr;

    switch (resource->GetInitData()->ResourceVersion) {
        case 0:
	    factory = std::make_shared<TextMMFactoryV0>();
	    break;
        default:
            // VERSION NOT SUPPORTED
            break;
    }

    if (factory == nullptr) {
        SPDLOG_ERROR("Failed to load Text with version {}", resource->GetInitData()->ResourceVersion);
	return nullptr;
    }

    factory->ParseFileBinary(reader, resource);

    return resource;
}

std::shared_ptr<IResource> TextMMFactory::ReadResourceXML(std::shared_ptr<ResourceInitData> initData,
                                                          tinyxml2::XMLElement* reader) {
    auto resource = std::make_shared<TextMM>(initData);
    std::shared_ptr<ResourceVersionFactory> factory = nullptr;

    switch (resource->GetInitData()->ResourceVersion) {
        case 0:
            factory = std::make_shared<TextMMFactoryV0>();
            break;
    }

    if (factory == nullptr) {
        SPDLOG_ERROR("Failed to load Text with version {}", resource->GetInitData()->ResourceVersion);
        return nullptr;
    }

    factory->ParseFileXML(reader, resource);

    return resource;
}

void LUS::TextMMFactoryV0::ParseFileBinary(std::shared_ptr<BinaryReader> reader, std::shared_ptr<IResource> resource) {
    std::shared_ptr<TextMM> text = std::static_pointer_cast<TextMM>(resource);
    ResourceVersionFactory::ParseFileBinary(reader, text);

    uint32_t msgCount = reader->ReadUInt32();
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
}
void TextMMFactoryV0::ParseFileXML(tinyxml2::XMLElement* reader, std::shared_ptr<IResource> resource) {
    std::shared_ptr<TextMM> txt = std::static_pointer_cast<TextMM>(resource);

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

            txt->messages.push_back(entry);
            int bp = 0;
        }

        child = child->NextSiblingElement();
    }
}

} // namespace LUS
