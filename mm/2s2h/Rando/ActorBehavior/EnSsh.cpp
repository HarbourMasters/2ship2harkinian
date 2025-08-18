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
        .msg = LOCALIZED(
            "Make me...normal again...I'll give you %g{{article}}{{item}}%w...Please...help me...\xE0",
            "Rends-moi...normal à nouveau... Je te donnerai %g{{article}}{{item}}%w...S'il te plaît...aide-moi...\xE0",
            "TODO_GERMAN", "TODO_JAPANESE", "TODO_SPANISH")
    };

    auto& randoStaticItem =
        Rando::StaticData::Items[RANDO_SAVE_CHECKS[RC_SWAMP_SPIDER_HOUSE_MASK_OF_TRUTH].randoItemId];

    std::string article = LOCALIZED(randoStaticItem.articleEng, randoStaticItem.articleFre, randoStaticItem.articleGer,
                                    randoStaticItem.articleJpn, randoStaticItem.articleSpa);
    if (!Ship_IsCStringEmpty(article.c_str())) {
        article += " ";
    }

    std::string name = LOCALIZED(randoStaticItem.nameEng, randoStaticItem.nameFre, randoStaticItem.nameGer,
                                 randoStaticItem.nameJpn, randoStaticItem.nameSpa);

    CustomMessage::Replace(&entry.msg, "{{article}}", article);
    CustomMessage::Replace(&entry.msg, "{{item}}", name);

    CustomMessage::LoadCustomMessageIntoFont(entry);
    *loadFromMessageTable = false;
}

void ApplyOceanSpiderHouseHint(u16* textId, bool* loadFromMessageTable) {
    CustomMessage::Entry entry = {
        .msg =
            LOCALIZED("Huh? How'd I get up here... Why do I have %g{{article}}{{item}}%w in my pocket...?\xE0",
                      "Hein? Comment je suis arrivé ici... Pourquoi j'ai %g{{article}}{{item}}%w dans ma poche...?\xE0",
                      "TODO_GERMAN", "TODO_JAPANESE", "TODO_SPANISH")
    };

    auto& randoStaticItem = Rando::StaticData::Items[RANDO_SAVE_CHECKS[RC_OCEAN_SPIDER_HOUSE_WALLET].randoItemId];

    std::string article = LOCALIZED(randoStaticItem.articleEng, randoStaticItem.articleFre, randoStaticItem.articleGer,
                                    randoStaticItem.articleJpn, randoStaticItem.articleSpa);
    if (!Ship_IsCStringEmpty(article.c_str())) {
        article += " ";
    }

    std::string name = LOCALIZED(randoStaticItem.nameEng, randoStaticItem.nameFre, randoStaticItem.nameGer,
                                 randoStaticItem.nameJpn, randoStaticItem.nameSpa);

    CustomMessage::Replace(&entry.msg, "{{article}}", article);
    CustomMessage::Replace(&entry.msg, "{{item}}", name);

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
