#include "ActorBehavior.h"
#include "public/bridge/consolevariablebridge.h"
#include "2s2h/CustomMessage/CustomMessage.h"

extern "C" {
#include "variables.h"
#include "overlays/actors/ovl_En_Minifrog/z_en_minifrog.h"
}


// TODOs: update text, logic, fix postLimbDraw crash

void EnMinifrog_OnOpenText(u16* textId, bool* loadFromMessageTable){

    // Need to change depending on frog
    RandoSaveCheck frogCheck = RANDO_SAVE_CHECKS[RC_CLOCK_TOWN_LAUNDRY_FROG];
    RandoItemId riFrogCheck = Rando::ConvertItem(frogCheck.randoItemId, RC_CLOCK_TOWN_LAUNDRY_FROG);

    auto entry = CustomMessage::LoadVanillaMessageTableEntry(*textId);

    entry.msg = "I understand. Have %y{{article}} {{item}}%w!\x13\x10";

    std::string itemName = Rando::StaticData::Items[riFrogCheck].name;
    std::string itemArticle = Rando::StaticData::Items[riFrogCheck].article;

    CustomMessage::ReplaceColorChars(&entry.msg);
    CustomMessage::Replace(&entry.msg, "{{item}}", itemName);
    CustomMessage::Replace(&entry.msg, "{{article}}", itemArticle);
    entry.autoFormat = false;
    // CustomMessage::EnsureMessageEnd(&entry.msg);
    CustomMessage::LoadCustomMessageIntoFont(entry);
    *loadFromMessageTable = false;
}

void EnMinifrog_OnOpenText2(u16* textId, bool* loadFromMessageTable){

    // Need to change depending on frog
    RandoSaveCheck frogCheck = RANDO_SAVE_CHECKS[RC_CLOCK_TOWN_LAUNDRY_FROG];
    RandoItemId riFrogCheck = Rando::ConvertItem(frogCheck.randoItemId, RC_CLOCK_TOWN_LAUNDRY_FROG);

    auto entry = CustomMessage::LoadVanillaMessageTableEntry(*textId);

    LUSLOG_DEBUG("%s", entry.msg.c_str());

    entry.msg = "Find me again and I will\x11 definitely go to the mountains. \x11Have %y{{article}} {{item}}%w!";

    std::string itemName = Rando::StaticData::Items[riFrogCheck].name;
    std::string itemArticle = Rando::StaticData::Items[riFrogCheck].article;

    CustomMessage::ReplaceColorChars(&entry.msg);
    CustomMessage::Replace(&entry.msg, "{{item}}", itemName);
    CustomMessage::Replace(&entry.msg, "{{article}}", itemArticle);
    entry.autoFormat = false;
    // CustomMessage::EnsureMessageEnd(&entry.msg);
    CustomMessage::LoadCustomMessageIntoFont(entry);
    *loadFromMessageTable = false;
}

// static u16 sIsFrogReturnedFlags[] = {
//     0,                  // FROG_YELLOW
//     WEEKEVENTREG_32_40, // FROG_CYAN
//     WEEKEVENTREG_32_80, // FROG_PINK
//     WEEKEVENTREG_33_01, // FROG_BLUE
//     WEEKEVENTREG_33_02, // FROG_WHITE
// };

void Rando::ActorBehavior::InitEnMinifrogBehavior() {

    COND_VB_SHOULD(VB_SPAWN_FROG, IS_RANDO && RANDO_SAVE_OPTIONS[RO_SHUFFLE_FROGS], {
        EnMinifrog* enMinifrog = va_arg(args, EnMinifrog*);
        switch (enMinifrog->frogIndex) {
            case 0:
                break;
            case 1:
                *should = RANDO_SAVE_CHECKS[RC_WOODFALL_TEMPLE_GEKKO_FROG].cycleObtained;
                break;
            case 2:
                *should = RANDO_SAVE_CHECKS[RC_GREAT_BAY_TEMPLE_GEKKO_FROG].cycleObtained;
                break;
            case 3:
                *should = RANDO_SAVE_CHECKS[RC_SOUTHERN_SWAMP_FROG].cycleObtained;
                break;
            case 4:
                *should = RANDO_SAVE_CHECKS[RC_CLOCK_TOWN_LAUNDRY_FROG].cycleObtained;
                break;
        }
    });

    COND_VB_SHOULD(VB_FROG_SET_RETURN_FLAG, IS_RANDO && RANDO_SAVE_OPTIONS[RO_SHUFFLE_FROGS], {
        EnMinifrog* enMinifrog = va_arg(args, EnMinifrog*);
        *should = true;
        switch (enMinifrog->frogIndex) {
            case 0:
                break;
            case 1:
                RANDO_SAVE_CHECKS[RC_WOODFALL_TEMPLE_GEKKO_FROG].eligible = true;
                break;
            case 2:
                RANDO_SAVE_CHECKS[RC_GREAT_BAY_TEMPLE_GEKKO_FROG].eligible = true;
                break;
            case 3:
                RANDO_SAVE_CHECKS[RC_SOUTHERN_SWAMP_FROG].eligible = true;
                break;
            case 4:
                RANDO_SAVE_CHECKS[RC_CLOCK_TOWN_LAUNDRY_FROG].eligible = true;
                break;
        }
    });
    
    COND_ID_HOOK(OnOpenText, 0x0D85, IS_RANDO && RANDO_SAVE_OPTIONS[RO_SHUFFLE_FROGS], EnMinifrog_OnOpenText);
    
    COND_ID_HOOK(OnOpenText, 0x0D88, IS_RANDO && RANDO_SAVE_OPTIONS[RO_SHUFFLE_FROGS], EnMinifrog_OnOpenText2);
}
