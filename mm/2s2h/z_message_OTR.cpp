#include "BenPort.h"
#include <libultraship/libultraship.h>
#include "2s2h/resource/type/Scene.h"
#include <utils/StringHelper.h>
#include "global.h"
#include "2s2h/resource/type/TextMM.h"
#include <message_data_static.h>

extern "C" MessageTableEntry* sMessageTableNES;
extern "C" MessageTableEntry* sMessageTableCredits;

extern "C" MessageTableEntry* sJPMessageEntryTablePtr;

MessageTableEntry* OTRMessage_LoadTable(const char* filePath, bool isNES) {
    auto file = std::static_pointer_cast<SOH::TextMM>(
        Ship::Context::GetInstance()->GetResourceManager()->LoadResource(filePath));

    if (file == nullptr)
        return nullptr;

    // Allocate room for an additional message
    // OTRTODO: Should not be malloc'ing here. It's fine for now since we check elsewhere that the message table is
    // already null.
    MessageTableEntry* table = (MessageTableEntry*)malloc(sizeof(MessageTableEntry) * (file->messages.size() + 1));

    for (size_t i = 0; i < file->messages.size(); i++) {
        uint8_t offset = isNES ? 11 : 12;
        table[i].textId = file->messages[i].id;
        table[i].typePos = (file->messages[i].textboxType << 4) | file->messages[i].textboxYPos;
        table[i].segment = (const char*)malloc(file->messages[i].msg.size() + offset);

        if (isNES) {
            auto segment = (char*)table[i].segment;
            segment[0] = file->messages[i].textboxType;
            segment[1] = file->messages[i].textboxYPos;
            segment[2] = (u8)file->messages[i].icon;
            segment[3] = (file->messages[i].nextMessageID & 0xFF00) >> 8;
            segment[4] = (file->messages[i].nextMessageID & 0x00FF);
            segment[5] = (file->messages[i].firstItemCost & 0xFF00) >> 8;
            segment[6] = (file->messages[i].firstItemCost & 0x00FF);
            segment[7] = (file->messages[i].secondItemCost & 0xFF00) >> 8;
            segment[8] = (file->messages[i].secondItemCost & 0x00FF);
            segment[9] = 0xFF;
            segment[10] = 0xFF;
        } else {
            uint16_t* segment = (uint16_t*)table[i].segment;
            segment[0] = (file->messages[i].textboxType << 8) | file->messages[i].textboxYPos;
            segment[1] = file->messages[i].icon;
            segment[2] = file->messages[i].nextMessageID;
            segment[3] = file->messages[i].firstItemCost;
            segment[4] = file->messages[i].secondItemCost;
            segment[5] = 0xFFFF;
        }

        memcpy((void*)(&table[i].segment[offset]), file->messages[i].msg.c_str(), file->messages[i].msg.size());

        table[i].msgSize = file->messages[i].msg.size() + offset;

        // if (isNES && file->messages[i].id == 0xFFFC)
        //_message_0xFFFC_nes = (char*)file->messages[i].msg.c_str();
    }

    return table;
}

// File select screen does not have access to an initialised PlayState, so cannot use MessageContext's messageEntryTable
extern "C" void OTRJPFontMessage_Init() {
    if (sJPMessageEntryTablePtr == NULL) {
        sJPMessageEntryTablePtr = OTRMessage_LoadTable("text/message_data_static_jp/message_data_static_jp", false);
    }
}

extern "C" void OTRMessage_Init(PlayState* play, bool isJP) {
    // OTRTODO: Added a lot of null checks here so that we don't malloc the table multiple times causing a memory leak.
    // We really ought to fix the implementation such that we aren't malloc'ing new tables.
    // Once we fix the implementation, remove these NULL checks.
    // if (play->msgCtx.messageEntryTableNes == NULL) {
    // OTRTODO:
    if (isJP) {
        OTRJPFontMessage_Init();
    } else {
        sMessageTableNES = OTRMessage_LoadTable("text/message_data_static/message_data_static", true);
    }
    //}

    auto file2 = std::static_pointer_cast<SOH::TextMM>(Ship::Context::GetInstance()->GetResourceManager()->LoadResource(
        "text/staff_message_data_static/staff_message_data_static"));
    sMessageTableCredits = (MessageTableEntry*)malloc(sizeof(MessageTableEntry) * file2->messages.size());

    for (size_t i = 0; i < file2->messages.size(); i++) {
        sMessageTableCredits[i].textId = file2->messages[i].id;
        sMessageTableCredits[i].typePos = (file2->messages[i].textboxType << 4) | file2->messages[i].textboxYPos;
        sMessageTableCredits[i].segment = file2->messages[i].msg.c_str();
        sMessageTableCredits[i].msgSize = file2->messages[i].msg.size();
    }
}
