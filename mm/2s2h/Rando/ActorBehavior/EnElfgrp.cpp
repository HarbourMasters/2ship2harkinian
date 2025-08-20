#include "ActorBehavior.h"
#include "public/bridge/consolevariablebridge.h"
#include "2s2h/ShipUtils.h"
#include "2s2h/CustomMessage/CustomMessage.h"

extern "C" {
#include "variables.h"
#include "functions.h"

#include "overlays/actors/ovl_En_Elfgrp/z_en_elfgrp.h"
}

void ApplyClockTownGreatFairyHint(u16* textId, bool* loadFromMessageTable) {
    CustomMessage::Entry entry = {
        .msg = LOCALIZED("%wPlease, find the Stray Fairy who's lost! We will reward you with {article1}%g{item1}%w and "
                         "maybe even {article2}%g{item2}%w if you are worthy.",
                         "%wS'il te plaît,trouve la Fée Perdue! Nous te récompenserons avec {article1}%g{item1}%w et "
                         "peut-être même {article2}%g{item2}%w si tu en es digne.",
                         "%wBitte finde die %reine%w Verirrte Fee, die irgendwo in Termina weilt. Wir würden dir auch "
                         "%g{{article1}}{{item1}}%w geben und vielleicht sogar %g{{article2}}{{item2}}%w, falls du "
                         "dessen würdig bist.",
                         "TODO_JAPANESE", "TODO_SPANISH")
    };

    auto& randoStaticItem1 = Rando::StaticData::Items[RANDO_SAVE_CHECKS[RC_CLOCK_TOWN_GREAT_FAIRY].randoItemId];
    std::string article1 =
        LOCALIZED(randoStaticItem1.articleEng, randoStaticItem1.articleFre, randoStaticItem1.articleGer,
                  randoStaticItem1.articleJpn, randoStaticItem1.articleSpa);
    if (!Ship_IsCStringEmpty(article1.c_str())) {
        article1 += " ";
    }
    std::string item1 = LOCALIZED(randoStaticItem1.nameEng, randoStaticItem1.nameFre, randoStaticItem1.nameGer,
                                  randoStaticItem1.nameJpn, randoStaticItem1.nameSpa);
    CustomMessage::Replace(&entry.msg, "{article1}", article1);
    CustomMessage::Replace(&entry.msg, "{item1}", item1);

    auto& randoStaticItem2 = Rando::StaticData::Items[RANDO_SAVE_CHECKS[RC_CLOCK_TOWN_GREAT_FAIRY_ALT].randoItemId];
    std::string article2 =
        LOCALIZED(randoStaticItem2.articleEng, randoStaticItem2.articleFre, randoStaticItem2.articleGer,
                  randoStaticItem2.articleJpn, randoStaticItem2.articleSpa);
    if (!Ship_IsCStringEmpty(article2.c_str())) {
        article2 += " ";
    }
    std::string item2 = LOCALIZED(randoStaticItem2.nameEng, randoStaticItem2.nameFre, randoStaticItem2.nameGer,
                                  randoStaticItem2.nameJpn, randoStaticItem2.nameSpa);
    CustomMessage::Replace(&entry.msg, "{article2}", article2);
    CustomMessage::Replace(&entry.msg, "{item2}", item2);

    CustomMessage::ReplaceSpecialChars(&entry.msg);
    CustomMessage::LoadCustomMessageIntoFont(entry);
    *loadFromMessageTable = false;
}

void ApplyGreatFairyHint(u16* textId, bool* loadFromMessageTable, RandoCheckId randoCheckId) {
    CustomMessage::Entry entry = {
        .msg = LOCALIZED(
            "%wPlease, find the Stray Fairies who match our color! We will reward you with {article}%g{item}%w",
            "%wS'il te plaît,trouve les Fées Perdues de notre couleur! Nous te récompenserons avec {article}%g{item}%w",
            "%wBitte, finde die Verirrten Feen die von %rgleicher Farbe%w wie wir sind. Wir würden dir auch "
            "%g{{article}}{{item}}%w geben.",
            "TODO_JAPANESE", "TODO_SPANISH")
    };

    auto& randoStaticItem = Rando::StaticData::Items[RANDO_SAVE_CHECKS[randoCheckId].randoItemId];
    std::string article = LOCALIZED(randoStaticItem.articleEng, randoStaticItem.articleFre, randoStaticItem.articleGer,
                                    randoStaticItem.articleJpn, randoStaticItem.articleSpa);
    if (!Ship_IsCStringEmpty(article.c_str())) {
        article += " ";
    }
    std::string item = LOCALIZED(randoStaticItem.nameEng, randoStaticItem.nameFre, randoStaticItem.nameGer,
                                 randoStaticItem.nameJpn, randoStaticItem.nameSpa);
    CustomMessage::Replace(&entry.msg, "{article}", article);
    CustomMessage::Replace(&entry.msg, "{item}", item);

    CustomMessage::ReplaceSpecialChars(&entry.msg);
    CustomMessage::LoadCustomMessageIntoFont(entry);
    *loadFromMessageTable = false;
}

// Handles the Great Fairy checks
void Rando::ActorBehavior::InitEnElfgrpBehavior() {
    COND_VB_SHOULD(VB_GIVE_ITEM_FROM_STRAY_FAIRY_MANAGER, IS_RANDO, {
        *should = false;

        EnElfgrp* elfgrp = va_arg(args, EnElfgrp*);

        switch (elfgrp->type) {
            case ENELFGRP_TYPE_POWER:
                if (!RANDO_SAVE_CHECKS[RC_WOODFALL_GREAT_FAIRY].cycleObtained) {
                    RANDO_SAVE_CHECKS[RC_WOODFALL_GREAT_FAIRY].eligible = true;
                }
                break;
            case ENELFGRP_TYPE_WISDOM:
                if (!RANDO_SAVE_CHECKS[RC_SNOWHEAD_GREAT_FAIRY].cycleObtained) {
                    RANDO_SAVE_CHECKS[RC_SNOWHEAD_GREAT_FAIRY].eligible = true;
                }
                break;
            case ENELFGRP_TYPE_COURAGE:
                if (!RANDO_SAVE_CHECKS[RC_GREAT_BAY_GREAT_FAIRY].cycleObtained) {
                    RANDO_SAVE_CHECKS[RC_GREAT_BAY_GREAT_FAIRY].eligible = true;
                }
                break;
            case ENELFGRP_TYPE_KINDNESS:
                if (!RANDO_SAVE_CHECKS[RC_IKANA_GREAT_FAIRY].cycleObtained) {
                    RANDO_SAVE_CHECKS[RC_IKANA_GREAT_FAIRY].eligible = true;
                }
                break;
            default: // ENELFGRP_TYPE_MAGIC
                if (!RANDO_SAVE_CHECKS[RC_CLOCK_TOWN_GREAT_FAIRY].cycleObtained) {
                    RANDO_SAVE_CHECKS[RC_CLOCK_TOWN_GREAT_FAIRY].eligible = true;
                }
                // In the game this uses an `else`, but in rando we are okay with both of these happening at the same
                // time
                if ((INV_CONTENT(ITEM_MASK_DEKU) == ITEM_MASK_DEKU || INV_CONTENT(ITEM_MASK_ZORA) == ITEM_MASK_ZORA ||
                     INV_CONTENT(ITEM_MASK_GORON) == ITEM_MASK_GORON) &&
                    !RANDO_SAVE_CHECKS[RC_CLOCK_TOWN_GREAT_FAIRY_ALT].cycleObtained) {
                    RANDO_SAVE_CHECKS[RC_CLOCK_TOWN_GREAT_FAIRY_ALT].eligible = true;
                }
                break;
        }
    });

    COND_VB_SHOULD(VB_GIVE_ITEM_FROM_GREAT_FAIRY, IS_RANDO, { *should = false; });

    COND_VB_SHOULD(VB_GREAT_FAIRY_GIVE_DOUBLE_DEFENSE_HEARTS, IS_RANDO, { *should = false; });

    COND_ID_HOOK(OnOpenText, 0x578, IS_RANDO, ApplyClockTownGreatFairyHint);
    COND_ID_HOOK(OnOpenText, 0x580, IS_RANDO, ApplyClockTownGreatFairyHint);

    COND_ID_HOOK(OnOpenText, 0x582, IS_RANDO, [](u16* textId, bool* loadFromMessageTable) {
        ApplyGreatFairyHint(textId, loadFromMessageTable, RC_WOODFALL_GREAT_FAIRY);
    });
    COND_ID_HOOK(OnOpenText, 0x583, IS_RANDO, [](u16* textId, bool* loadFromMessageTable) {
        ApplyGreatFairyHint(textId, loadFromMessageTable, RC_WOODFALL_GREAT_FAIRY);
    });
    COND_ID_HOOK(OnOpenText, 0x585, IS_RANDO, [](u16* textId, bool* loadFromMessageTable) {
        ApplyGreatFairyHint(textId, loadFromMessageTable, RC_SNOWHEAD_GREAT_FAIRY);
    });
    COND_ID_HOOK(OnOpenText, 0x586, IS_RANDO, [](u16* textId, bool* loadFromMessageTable) {
        ApplyGreatFairyHint(textId, loadFromMessageTable, RC_SNOWHEAD_GREAT_FAIRY);
    });
    COND_ID_HOOK(OnOpenText, 0x588, IS_RANDO, [](u16* textId, bool* loadFromMessageTable) {
        ApplyGreatFairyHint(textId, loadFromMessageTable, RC_GREAT_BAY_GREAT_FAIRY);
    });
    COND_ID_HOOK(OnOpenText, 0x589, IS_RANDO, [](u16* textId, bool* loadFromMessageTable) {
        ApplyGreatFairyHint(textId, loadFromMessageTable, RC_GREAT_BAY_GREAT_FAIRY);
    });
    COND_ID_HOOK(OnOpenText, 0x58B, IS_RANDO, [](u16* textId, bool* loadFromMessageTable) {
        ApplyGreatFairyHint(textId, loadFromMessageTable, RC_IKANA_GREAT_FAIRY);
    });
    COND_ID_HOOK(OnOpenText, 0x58C, IS_RANDO, [](u16* textId, bool* loadFromMessageTable) {
        ApplyGreatFairyHint(textId, loadFromMessageTable, RC_IKANA_GREAT_FAIRY);
    });
}
