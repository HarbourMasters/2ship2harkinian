#include "ActorBehavior.h"
#include "public/bridge/consolevariablebridge.h"
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipUtils.h"
#include "2s2h/CustomMessage/CustomMessage.h"

extern "C" {
#include "variables.h"
#include "functions.h"
}

void ApplySwampSpiderHouseHint(u16* textId, bool* loadFromMessageTable) {
    CustomMessage::Entry entry = {
        .msg = LOCALIZED("Make me...normal again...I'll give you {{article}}%g{{itemName}}%w...Please...help me...\xE0",
                         "Rends-moi...normal à nouveau... Je te donnerai {{article}}%g{{itemName}}%w...S'il te "
                         "plaît...aide-moi...\xE0",
                         "Verwandle mich... zurück... Ich werde dir... {{article}}%g{{itemName}}%w geben... Bitte... "
                         "Hilf mir...\xE0",
                         "TODO_JAPANESE", "TODO_SPANISH")
    };

    auto& randoStaticItem =
        Rando::StaticData::Items[RANDO_SAVE_CHECKS[RC_SWAMP_SPIDER_HOUSE_MASK_OF_TRUTH].randoItemId];

    std::string article = LOCALIZED(randoStaticItem.articleEng, randoStaticItem.articleFre, randoStaticItem.articleGer2,
                                    randoStaticItem.articleJpn, randoStaticItem.articleSpa);
    if (!Ship_IsCStringEmpty(article.c_str()) && article != "l'") { // Special case handling with l' french article
        article += " ";
    }

    std::string name = LOCALIZED(randoStaticItem.nameEng, randoStaticItem.nameFre, randoStaticItem.nameGer,
                                 randoStaticItem.nameJpn, randoStaticItem.nameSpa);

    CustomMessage::Replace(&entry.msg, "{{article}}", article);
    CustomMessage::Replace(&entry.msg, "{{itemName}}", name);

    CustomMessage::LoadCustomMessageIntoFont(entry);
    *loadFromMessageTable = false;
}

void ApplyOceanSpiderHouseHint(u16* textId, bool* loadFromMessageTable) {
    CustomMessage::Entry entry = {
        .msg = LOCALIZED(
            "Huh? How'd I get up here... Why do I have {{article}}%g{{itemName}}%w in my pocket...?\xE0",
            "Hein? Comment je suis arrivé ici... Pourquoi j'ai {{article}}%g{{itemName}}%w dans ma poche...?\xE0",
            "Huch? Wie bin ich denn hier hin gekommen... Warum habe ich {{article}}%g{{itemName}}%w in meiner "
            "Tasche...?\xE0",
            "TODO_JAPANESE", "TODO_SPANISH")
    };

    auto& randoStaticItem = Rando::StaticData::Items[RANDO_SAVE_CHECKS[RC_OCEAN_SPIDER_HOUSE_WALLET].randoItemId];

    std::string article = LOCALIZED(randoStaticItem.articleEng, randoStaticItem.articleFre, randoStaticItem.articleGer2,
                                    randoStaticItem.articleJpn, randoStaticItem.articleSpa);
    if (!Ship_IsCStringEmpty(article.c_str()) && article != "l'") { // Special case handling with l' french article
        article += " ";
    }

    std::string name = LOCALIZED(randoStaticItem.nameEng, randoStaticItem.nameFre, randoStaticItem.nameGer,
                                 randoStaticItem.nameJpn, randoStaticItem.nameSpa);

    CustomMessage::Replace(&entry.msg, "{{article}}", article);
    CustomMessage::Replace(&entry.msg, "{{itemName}}", name);

    CustomMessage::ReplaceSpecialChars(&entry.msg);
    CustomMessage::LoadCustomMessageIntoFont(entry);
    *loadFromMessageTable = false;
}

void Rando::ActorBehavior::InitEnSshBehavior() {
    bool shouldRegister = IS_RANDO && RANDO_SAVE_OPTIONS[RO_HINTS_SPIDER_HOUSES];

    // "Recruiting Soldiers..." Posters around Clock Town
    COND_ID_HOOK(OnOpenText, 0x915, shouldRegister, ApplySwampSpiderHouseHint);
    COND_ID_HOOK(OnOpenText, 0x1130, shouldRegister, ApplyOceanSpiderHouseHint);
    COND_ID_HOOK(OnOpenText, 0x1131, shouldRegister, ApplyOceanSpiderHouseHint);

    COND_ID_HOOK(ShouldActorInit, ACTOR_EN_SSH, IS_RANDO, [](Actor* actor, bool* should) {
        // Skip first dialog
        SET_WEEKEVENTREG(WEEKEVENTREG_TALKED_SWAMP_SPIDER_HOUSE_MAN);
    });
}
