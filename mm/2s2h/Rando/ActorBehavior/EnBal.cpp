#include "ActorBehavior.h"
#include "public/bridge/consolevariablebridge.h"
#include "2s2h/CustomMessage/CustomMessage.h"
#include "2s2h/ShipUtils.h"

extern "C" {
#include "variables.h"
#include "overlays/actors/ovl_En_Bal/z_en_bal.h"
void EnBal_SetupTalk(EnBal* enBal);
}

std::map<int16_t, std::vector<RandoCheckId>> tingleMap = {
    { SCENE_BACKTOWN, { RC_CLOCK_TOWN_NORTH_TINGLE_MAP_01, RC_CLOCK_TOWN_NORTH_TINGLE_MAP_02 } },
    { SCENE_24KEMONOMITI, { RC_ROAD_TO_SOUTHERN_SWAMP_TINGLE_MAP_01, RC_ROAD_TO_SOUTHERN_SWAMP_TINGLE_MAP_02 } },
    { SCENE_17SETUGEN, { RC_TWIN_ISLANDS_TINGLE_MAP_01, RC_TWIN_ISLANDS_TINGLE_MAP_02 } },
    { SCENE_17SETUGEN2, { RC_TWIN_ISLANDS_TINGLE_MAP_01, RC_TWIN_ISLANDS_TINGLE_MAP_02 } },
    { SCENE_ROMANYMAE, { RC_MILK_ROAD_TINGLE_MAP_01, RC_MILK_ROAD_TINGLE_MAP_02 } },
    { SCENE_30GYOSON, { RC_GREAT_BAY_COAST_TINGLE_MAP_01, RC_GREAT_BAY_COAST_TINGLE_MAP_02 } },
    { SCENE_IKANA, { RC_IKANA_CANYON_TINGLE_MAP_01, RC_IKANA_CANYON_TINGLE_MAP_02 } }
};

void OnOpenShopText(u16* textId, bool* loadFromMessageTable) {
    RandoCheckId randoCheckId1 = tingleMap[gPlayState->sceneId][0];
    RandoCheckId randoCheckId2 = tingleMap[gPlayState->sceneId][1];

    auto entry = CustomMessage::LoadVanillaMessageTableEntry(*textId);
    entry.autoFormat = false;

    entry.msg = LOCALIZED("\x02\xC3{article1}%y{item1}%w\x01 {price1} Rupees\x11"
                          "\x02{article2}%y{item2}%w\x01 {price2} Rupees\x11"
                          "\x02No thanks",
                          "\x02\xC3{article1}%y{item1}%w\x01 {price1} Rubis\x11"
                          "\x02{article2}%y{item2}%w\x01 {price2} Rubis\x11"
                          "\x02Non merci",
                          "TODO_GERMAN", "TODO_JAPANESE", "TODO_SPANISH");

    const auto& item1 = Rando::StaticData::Items[RANDO_SAVE_CHECKS[randoCheckId1].randoItemId];
    const auto& item2 = Rando::StaticData::Items[RANDO_SAVE_CHECKS[randoCheckId2].randoItemId];
    std::string article1 =
        LOCALIZED(item1.articleEng, item1.articleFre, item1.articleGer, item1.articleJpn, item1.articleSpa);
    if (!Ship_IsCStringEmpty(article1.c_str())) {
        article1 += " ";
    }
    std::string itemName1 = LOCALIZED(item1.nameEng, item1.nameFre, item1.nameGer, item1.nameJpn, item1.nameSpa);
    std::string article2 =
        LOCALIZED(item2.articleEng, item2.articleFre, item2.articleGer, item2.articleJpn, item2.articleSpa);
    if (!Ship_IsCStringEmpty(article2.c_str())) {
        article2 += " ";
    }
    std::string itemName2 = LOCALIZED(item2.nameEng, item2.nameFre, item2.nameGer, item2.nameJpn, item2.nameSpa);

    CustomMessage::Replace(&entry.msg, "{article1}", article1);
    CustomMessage::Replace(&entry.msg, "{item1}", itemName1);
    CustomMessage::Replace(&entry.msg, "{article2}", article2);
    CustomMessage::Replace(&entry.msg, "{item2}", itemName2);
    CustomMessage::Replace(&entry.msg, "{price1}", std::to_string(RANDO_SAVE_CHECKS[randoCheckId1].price));
    CustomMessage::Replace(&entry.msg, "{price2}", std::to_string(RANDO_SAVE_CHECKS[randoCheckId2].price));
    CustomMessage::ReplaceSpecialChars(&entry.msg);
    CustomMessage::EnsureMessageEnd(&entry.msg);
    CustomMessage::LoadCustomMessageIntoFont(entry);
    *loadFromMessageTable = false;
};

void OnOpenCantGetText(u16* textId, bool* loadFromMessageTable) {
    auto entry = CustomMessage::LoadVanillaMessageTableEntry(*textId);
    entry.msg = LOCALIZED("I'm sorry, but it seems I cannot sell this to you now.",
                          "Je suis désolé, mais il semble que je ne puisse pas te vendre ceci maintenant.",
                          "TODO_GERMAN", "TODO_JAPANESE", "TODO_SPANISH");

    CustomMessage::ReplaceSpecialChars(&entry.msg);
    CustomMessage::LoadCustomMessageIntoFont(entry);
    *loadFromMessageTable = false;
};

void Rando::ActorBehavior::InitEnBalBehavior() {
    bool shouldRegister = IS_RANDO && RANDO_SAVE_OPTIONS[RO_SHUFFLE_TINGLE_SHOPS];

    COND_VB_SHOULD(VB_NOT_AFFORD_TINGLE_MAP, shouldRegister, {
        EnBal* enBal = va_arg(args, EnBal*);
        s32* price = va_arg(args, s32*);
        auto randoCheckId = tingleMap[gPlayState->sceneId][gPlayState->msgCtx.choiceIndex];

        *price = RANDO_SAVE_CHECKS[randoCheckId].price;

        *should = gSaveContext.save.saveInfo.playerData.rupees < *price;
    });

    COND_VB_SHOULD(VB_ALREADY_HAVE_TINGLE_MAP, shouldRegister, {
        EnBal* enBal = va_arg(args, EnBal*);

        auto randoCheckId = tingleMap[gPlayState->sceneId][gPlayState->msgCtx.choiceIndex];

        if (Rando::IsItemObtainable(RANDO_SAVE_CHECKS[randoCheckId].randoItemId, randoCheckId)) {
            *should = false;
        } else {
            *should = true;
        }
    });

    COND_VB_SHOULD(VB_TINGLE_GIVE_MAP_UNLOCK, shouldRegister, {
        EnBal* enBal = va_arg(args, EnBal*);
        RANDO_SAVE_CHECKS[tingleMap[gPlayState->sceneId][gPlayState->msgCtx.choiceIndex]].eligible = true;
        Message_StartTextbox(gPlayState, 0x1D17, &enBal->picto.actor);
        enBal->textId = 0x1D17;
        EnBal_SetupTalk(enBal);
        *should = false;
    });

    COND_ID_HOOK(OnOpenText, 0x1D11, shouldRegister, OnOpenShopText);
    COND_ID_HOOK(OnOpenText, 0x1D12, shouldRegister, OnOpenShopText);
    COND_ID_HOOK(OnOpenText, 0x1D13, shouldRegister, OnOpenShopText);
    COND_ID_HOOK(OnOpenText, 0x1D14, shouldRegister, OnOpenShopText);
    COND_ID_HOOK(OnOpenText, 0x1D15, shouldRegister, OnOpenShopText);
    COND_ID_HOOK(OnOpenText, 0x1D16, shouldRegister, OnOpenShopText);
    COND_ID_HOOK(OnOpenText, 0x1D09, shouldRegister, OnOpenCantGetText);

    COND_VB_SHOULD(VB_HAVE_MAGIC_FOR_TINGLE, shouldRegister, { *should = true; });
}
