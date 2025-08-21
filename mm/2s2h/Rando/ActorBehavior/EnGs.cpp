#include "ActorBehavior.h"
#include "public/bridge/consolevariablebridge.h"
#include "2s2h/ShipUtils.h"
#include "2s2h/CustomMessage/CustomMessage.h"

#include <vector>

extern "C" {
#include "functions.h"
#include "variables.h"

#include "overlays/actors/ovl_En_Gs/z_en_gs.h"
}

#define FIRST_GS_MESSAGE 0x20D1
#define SECOND_GS_MESSAGE 0x20C0
// TODO HATO: LOCALIZED FLAVOUR
std::vector<std::string> flavorText = {
    "Good luck on your journey ...",
    "I hope you find what you're looking for ...",
    "... Evil is afoot",
    "Beware the moon's gaze",
    " .. It's dangerous to go alone",
};

s32 GetNormalizedCost() {
    s32 obtainedChecks = 0;
    s32 maxChecks = 0;
    for (auto& [randoCheckId, _] : Rando::StaticData::Checks) {
        RandoSaveCheck saveCheck = RANDO_SAVE_CHECKS[randoCheckId];
        if (saveCheck.shuffled) {
            maxChecks++;
            if (saveCheck.obtained) {
                obtainedChecks++;
            }
        }
    }

    return MAX(10, MIN(250, 10 + (obtainedChecks * (250 - 10)) / (maxChecks)));
}

RandoCheckId GetRandomCheck(bool repeatableOnlyObtained = false) {
    Player* player = GET_PLAYER(gPlayState);
    if (player->talkActor == nullptr || player->talkActor->id != ACTOR_EN_GS) {
        return RC_UNKNOWN;
    }
    EnGs* enGs = (EnGs*)player->talkActor;

    std::vector<RandoCheckId> availableChecks;
    for (auto& [randoCheckId, _] : Rando::StaticData::Checks) {
        RandoSaveCheck saveCheck = RANDO_SAVE_CHECKS[randoCheckId];
        if (saveCheck.shuffled && Rando::StaticData::Items[saveCheck.randoItemId].randoItemType != RITYPE_JUNK &&
            (!repeatableOnlyObtained || !saveCheck.obtained)) {
            availableChecks.push_back(randoCheckId);
        }
    }

    if (availableChecks.empty()) {
        return RC_UNKNOWN;
    }

    if (repeatableOnlyObtained) {
        Ship_Random_Seed(gGameState->frames);
    } else {
        uint32_t seed = gPlayState->sceneId + enGs->actor.home.pos.x + enGs->actor.home.pos.z;
        Ship_Random_Seed(gSaveContext.save.shipSaveInfo.rando.finalSeed + seed);
    }
    return availableChecks[Ship_Random(0, availableChecks.size() - 1)];
}

void Rando::ActorBehavior::InitEnGsBehavior() {
    bool shouldRegister =
        IS_RANDO && (RANDO_SAVE_OPTIONS[RO_HINTS_GOSSIP_STONES] || RANDO_SAVE_OPTIONS[RO_HINTS_PURCHASEABLE]);

    COND_VB_SHOULD(VB_GS_CONSIDER_MASK_OF_TRUTH_EQUIPPED, shouldRegister, { *should = true; });

    // Override the message ID so that we can control the text
    COND_VB_SHOULD(VB_GS_CONTINUE_TEXTBOX, shouldRegister, {
        *should = false;
        Message_ContinueTextbox(gPlayState, SECOND_GS_MESSAGE);
    });

    COND_ID_HOOK(OnOpenText, FIRST_GS_MESSAGE, shouldRegister, [](u16* textId, bool* loadFromMessageTable) {
        auto entry = CustomMessage::LoadVanillaMessageTableEntry(*textId);
        if (RANDO_SAVE_OPTIONS[RO_HINTS_GOSSIP_STONES]) {
            RandoCheckId randoCheckId = GetRandomCheck();
            if (randoCheckId == RC_UNKNOWN) {
                return;
            }

            entry.autoFormat = false;
            auto& saveCheck = RANDO_SAVE_CHECKS[randoCheckId];
            const auto& item = Rando::StaticData::Items[saveCheck.randoItemId];

            std::string article =
                LOCALIZED(item.articleEng, item.articleFre, item.articleGer, item.articleJpn, item.articleSpa);
            if (!Ship_IsCStringEmpty(article.c_str()) &&
                article != "l'") { // Special case handling with l' french article
                article += " ";
            }

            // Set French adjective agreement based on article
            std::string frenchHiddenAdjective = CustomMessage::GetFrenchAdjectiveAgreement(article, "caché", "cachée");

            entry.msg = LOCALIZED("They say {{article}}%g{{itemName}}%w is hidden at %y{{location}}%w.",
                                  "Selon moi, {{article}}%g{{itemName}} %west {{caché}} à %y{{location}}%w.",
                                  "Man erzählt sich, dass {{article}}%g{{itemName}}%w %y{{location}}%w versteckt sei.",
                                  "TODO_JAPANESE", "TODO_SPANISH");

            std::string itemName = LOCALIZED(item.nameEng, item.nameFre, item.nameGer, item.nameJpn, item.nameSpa);
            CustomMessage::Replace(&entry.msg, "{{article}}", article);
            CustomMessage::Replace(&entry.msg, "{{itemName}}", itemName);
            CustomMessage::Replace(&entry.msg, "{{caché}}", frenchHiddenAdjective);
            CustomMessage::Replace(&entry.msg, "{{location}}",
                                   Ship_GetSceneName(Rando::StaticData::Checks[randoCheckId].sceneId));
            // Replace colors and special charcters before line break calculation
            CustomMessage::ReplaceColorChars(&entry.msg);
            CustomMessage::ReplaceSpecialChars(&entry.msg);
            CustomMessage::AddLineBreaks(&entry.msg);
            if (RANDO_SAVE_OPTIONS[RO_HINTS_PURCHASEABLE]) {
                entry.msg += "\x10...\x13\x12";
            }
        } else {
            entry.msg = "";
        }
        if (RANDO_SAVE_OPTIONS[RO_HINTS_PURCHASEABLE]) {
            entry.msg +=
                LOCALIZED("Trade %r{{rupees}} Rupees%w for a hint?\x02\x11\xC2No\x11Yes",
                          "Échanger %r{{rupees}} Rubis%w contre un indice?\x02\x11\xC2Non\x11Oui",
                          "Brauchst du einen Hinweis? Nur %r{{rupees}} Rubine%w!\x02\x11\xC2Zu teuer!\x11\Abgemacht!",
                          "TODO_JAPANESE", "TODO_SPANISH");
            s32 cost = GetNormalizedCost();
            CustomMessage::Replace(&entry.msg, "{{rupees}}", std::to_string(cost));
            CustomMessage::ReplaceColorChars(&entry.msg);
            CustomMessage::ReplaceSpecialChars(&entry.msg);
        }
        CustomMessage::EnsureMessageEnd(&entry.msg);
        CustomMessage::ReplaceSpecialChars(&entry.msg);
        CustomMessage::LoadCustomMessageIntoFont(entry);
        *loadFromMessageTable = false;
    });

    COND_ID_HOOK(OnOpenText, SECOND_GS_MESSAGE, shouldRegister, [](u16* textId, bool* loadFromMessageTable) {
        MessageContext* msgCtx = &gPlayState->msgCtx;
        auto entry = CustomMessage::LoadVanillaMessageTableEntry(*textId);
        if (RANDO_SAVE_OPTIONS[RO_HINTS_PURCHASEABLE]) {
            if (msgCtx->choiceIndex == 1) {
                s32 cost = GetNormalizedCost();
                RandoCheckId randoCheckId = GetRandomCheck(true);
                if (gSaveContext.save.saveInfo.playerData.rupees < cost) {
                    entry.msg = LOCALIZED("Foolish... You don't have enough rupees...",
                                          "Imprudent...Tu n'as pas assez de rubis...",
                                          "Narr... Du hast nicht genügend Rubine...", "TODO_JAPANESE", "TODO_SPANISH");
                } else if (randoCheckId == RC_UNKNOWN) {
                    entry.msg =
                        LOCALIZED("I have no more hints for you...", "Je n'ai plus d'indices pour toi...",
                                  "Ich habe keine weiteren Hinweise für dich...", "TODO_JAPANESE", "TODO_SPANISH");
                } else {
                    RandoSaveCheck saveCheck = RANDO_SAVE_CHECKS[randoCheckId];

                    const auto& item = Rando::StaticData::Items[saveCheck.randoItemId];
                    std::string article =
                        LOCALIZED(item.articleEng, item.articleFre, item.articleGer, item.articleJpn, item.articleSpa);
                    if (!Ship_IsCStringEmpty(article.c_str()) &&
                        article != "l'") { // Special case handling with l' french article
                        article += " ";
                    }

                    // Set French adjective agreement based on article
                    std::string frenchHiddenAdjective =
                        CustomMessage::GetFrenchAdjectiveAgreement(article, "caché", "cachée");

                    entry.msg = LOCALIZED(
                        "Wise choice... They say {{article}}%g{{itemName}}%w is hidden at %y{{location}}%w.",
                        "Sage décision... Selon moi, {{article}}%g{{itemName}} %west {{caché}} à %y{{location}}%w.",
                        "Kluge Entscheidung... Man erzählt sich, dass {{article}}%g{{itemName}}%w %y{{location}}%w "
                        "versteckt sei.",
                        "TODO_JAPANESE", "TODO_SPANISH");

                    std::string itemName =
                        LOCALIZED(item.nameEng, item.nameFre, item.nameGer, item.nameJpn, item.nameSpa);
                    CustomMessage::Replace(&entry.msg, "{{article}}", article);
                    CustomMessage::Replace(&entry.msg, "{{itemName}}", itemName);
                    CustomMessage::Replace(&entry.msg, "{{caché}}", frenchHiddenAdjective);
                    CustomMessage::Replace(&entry.msg, "{{location}}",
                                           Ship_GetSceneName(Rando::StaticData::Checks[randoCheckId].sceneId));
                    gSaveContext.rupeeAccumulator -= cost;
                    cost *= 2;
                }
            } else {
                entry.msg =
                    LOCALIZED("Foolish... Come back later when you have more sense.",
                              "Imprudent... Reviens plus tard quand tu seras plus sage.",
                              "Narr... Komm später wieder wenn du bei Verstand bist.", "TODO_JAPANESE", "TODO_SPANISH");
            }
        } else {
            entry.msg = flavorText[Ship_Random(0, flavorText.size() - 1)];
        }
        CustomMessage::ReplaceSpecialChars(&entry.msg);
        CustomMessage::LoadCustomMessageIntoFont(entry);
        *loadFromMessageTable = false;
    });

    // Four Gossip Stone Grottos Heart Piece item grant behavior override
    COND_VB_SHOULD(VB_GIVE_ITEM_FROM_OFFER, IS_RANDO, {
        GetItemId* item = va_arg(args, GetItemId*);
        Actor* refActor = va_arg(args, Actor*);
        Player* player = GET_PLAYER(gPlayState);

        if (refActor->id != ACTOR_EN_GS || *item != GI_HEART_PIECE) {
            return;
        }

        *should = false;

        refActor->parent = &player->actor;
    });
}
