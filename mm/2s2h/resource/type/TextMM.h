#pragma once

#include <cstdint>
#include <vector>
#include "Resource.h"
#include <libultraship/libultra/types.h>

namespace LUS {
// TODO: we've moved away from using classes for this stuff

class MessageEntryMM {
  public:
    uint16_t id;
    uint8_t textboxType;
    uint8_t textboxYPos;
    uint8_t icon;
    uint16_t nextMessageID;
    uint16_t firstItemCost;
    uint16_t secondItemCost;
    uint32_t segmentId;
    uint32_t msgOffset;
    std::string msg;
};

class TextMM : public Resource<MessageEntryMM> {
public:
  using Resource::Resource;

    TextMM() : Resource(std::shared_ptr<ResourceInitData>()) {
  }

    MessageEntryMM* GetPointer();
    size_t GetPointerSize();

    std::vector<MessageEntryMM> messages;
};
}; // namespace LUS
