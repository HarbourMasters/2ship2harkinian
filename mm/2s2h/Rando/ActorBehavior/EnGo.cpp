#include "ActorBehavior.h"
#include "2s2h/CustomMessage/CustomMessage.h"
#include "2s2h/Rando/Logic/Logic.h"
#include "2s2h/Rando/StaticData/StaticData.h"

extern "C" {
#include "variables.h"
#include "overlays/actors/ovl_En_Go/z_en_go.h"

void Player_TalkWithPlayer(PlayState* play, Actor* actor);
void func_80837B60(PlayState* play, Player* player);
s32 func_80832558(PlayState* play, Player* player, PlayerFuncD58 arg2);
}

static std::vector<u8> skipCmds = {};
static bool freePowderKegGrantActive = false;

void Rando::ActorBehavior::InitEnGoBehavior() {
    COND_ID_HOOK(OnActorInit, ACTOR_EN_GO, IS_RANDO, [](Actor* actor) { skipCmds.clear(); });

    // Medigoron - Scripted Actors
    COND_VB_SHOULD(VB_EXEC_MSG_EVENT, IS_RANDO, {
        u32 cmdId = va_arg(args, u32);
        Actor* actor = va_arg(args, Actor*);
        MsgScript* script = va_arg(args, MsgScript*);
        Player* player = GET_PLAYER(gPlayState);

        if (actor->id != ACTOR_EN_GO || ENGO_GET_TYPE(actor) != ENGO_MEDIGORON) {
            return;
        }

        if (!skipCmds.empty() && cmdId == skipCmds.at(0)) {
            skipCmds.erase(skipCmds.begin());
            *should = false;

            return;
        }

        if (cmdId == MSCRIPT_CMD_BRANCH_ON_ITEM) {
            u16 itemId = MSCRIPT_GET_16(script, 1);
            s16 skip = MSCRIPT_GET_16(script, 3);

            s16 jumpTarget = 0x009A;
            if (!RANDO_SAVE_CHECKS[RC_GORON_VILLAGE_MEDIGORON].cycleObtained) {
                jumpTarget = 0x008C;
            }

            s16 offsetAdjustment = (skip == 0x009A - 0x002B || skip == 0x008C - 0x002B) ? 0x002B : 0x0041;
            s16 skipOffset = jumpTarget - offsetAdjustment;

            script[3] = skipOffset >> 8;
            script[4] = skipOffset & 0xFF;
            return;
        }

        if (cmdId == MSCRIPT_CMD_14 || cmdId == MSCRIPT_CMD_15) { // MSCRIPT_BEGIN_TEXT/MSCRIPT_CONTINUE_TEXT

            u16 textId = MSCRIPT_GET_16(script, 1);

            // Only override behavior if the player has yet to discover powder kegs
            if (textId != 0x0C8C || HAS_ITEM(ITEM_POWDER_KEG)) {
                return;
            }

            *should = false;
            gPlayState->msgCtx.choiceIndex = 0;

            skipCmds.clear();
            skipCmds.push_back(MSCRIPT_CMD_12); // MSCRIPT_AWAIT_TEXT

            return;
        }

        // Identify text choice branch at 0x004E by skip offset values
        if (cmdId == MSCRIPT_CMD_05) { // MSCRIPT_BRANCH_ON_TEXT_CHOICE
            s16 skipChoice1 = MSCRIPT_GET_16(script, 1);
            s16 skipChoice2 = MSCRIPT_GET_16(script, 3);
            s16 skipChoice3 = MSCRIPT_GET_16(script, 5);

            switch (skipChoice1, skipChoice2, skipChoice3) {
                case (0x0, 0x00AD - 0x00A8, 0x0):
                    // Skip item grant and additional dialogue if player hasn't found a Keg yet
                    if (!HAS_ITEM(ITEM_POWDER_KEG)) {
                        s16 skipOffset = 0x00AE - 0x00A8;

                        script[1] = script[3] = script[5] = skipOffset >> 8;
                        script[2] = script[4] = script[6] = skipOffset & 0xFF;
                        break;
                    }

                    // Restore overwritten script default values
                    script[1] = script[2] = script[5] = script[6] = 0x0;
                    script[3] = (0x00AD - 0x00A8) >> 8;
                    script[4] = (0x00AD - 0x00A8) & 0xFF;
            }

            return;
        }

        if (cmdId == MSCRIPT_CMD_06) { // MSCRIPT_OFFER_ITEM
            if (freePowderKegGrantActive) {
                *should = false;

                EnGo* enGo = (EnGo*)actor;
                Player* player = GET_PLAYER(gPlayState);
                player->talkActor = &enGo->actor;

                if (!RANDO_SAVE_CHECKS[RC_GORON_VILLAGE_MEDIGORON].cycleObtained) {
                    RANDO_SAVE_CHECKS[RC_GORON_VILLAGE_MEDIGORON].eligible = true;
                }

                s16 skipOffset = 0x00C3 - 0x00BD;
                script[3] = skipOffset >> 8;
                script[4] = skipOffset & 0xFF;
                return;
            }

            // Restore overwritten script default values
            s16 skipOffset = 0;
            script[3] = skipOffset >> 8;
            script[4] = skipOffset & 0xFF;
            return;
        }
    });

    COND_ID_HOOK(OnOpenText, 0x0C81, IS_RANDO, [](u16* textId, bool* loadFromMessageTable) {
        auto entry = CustomMessage::LoadVanillaMessageTableEntry(*textId);
        Audio_PlaySfx(NA_SE_EN_GOLON_WAKE_UP); // Original script plays this as part of the text.
        entry.msg = LOCALIZED("Want a %rPowder Keg%w?\n"
                              "The Goron Elder said I can't sell my\n"
                              "a %rPowder Kegs%w to anyone new,\n"
                              "\x12"
                              "but I can give you one to\n"
                              "blow up the boulder near the track.\x19",
                              "Tu veux un %rBaril de Poudre%w?\n"
                              "Le Doyen Goron a dit que je ne peux\n"
                              "pas vendre de %rBarils de Poudre%w\n"
                              "aux nouveaux,"
                              "\x12"
                              "mais je peux t'en donner un pour\n"
                              "faire exploser le rocher près de la\n"
                              "piste.\x19",
                              "TODO_GERMAN", "TODO_JAPANESE", "TODO_SPANISH");

        CustomMessage::ReplaceSpecialChars(&entry.msg);
        CustomMessage::LoadCustomMessageIntoFont(entry);
        *loadFromMessageTable = false;
    });

    COND_ID_HOOK(OnOpenText, 0x0C83, IS_RANDO, [](u16* textId, bool* loadFromMessageTable) {
        auto entry = CustomMessage::LoadVanillaMessageTableEntry(*textId);
        Audio_PlaySfx(NA_SE_EN_GOLON_WAKE_UP); // Original script plays this as part of the text.
        entry.msg = LOCALIZED("If you can %rdestroy%w the boulder\n"
                              "that blocks the entrance to the\n"
                              "%rGoron Racetrack%w near here ...\n"
                              "\x12"
                              "using a %rPowder Keg%w I'm about\n"
                              "to give you, then I'll give you\n"
                              "{itemArticle}%g{randoItem}%w.\x19",
                              "Si tu peux %rdétruire%w le rocher\n"
                              "qui bloque l'entrée de la\n"
                              "%rPiste Goron%w près d'ici...\n"
                              "\x12"
                              "avec un %rBaril de Poudre%w que je vais\n"
                              "te donner, alors je te donnerai\n"
                              "{itemArticle}%g{randoItem}%w.\x19",
                              "TODO_GERMAN", "TODO_JAPANESE", "TODO_SPANISH");

        const auto& item = Rando::StaticData::Items[RANDO_SAVE_CHECKS[RC_GORON_VILLAGE_MEDIGORON].randoItemId];
        std::string itemArticle =
            LOCALIZED(item.articleEng, item.articleFre, item.articleGer, item.articleJpn, item.articleSpa);
        if (!Ship_IsCStringEmpty(itemArticle.c_str())) {
            itemArticle += " ";
        }
        std::string randoItemName = LOCALIZED(item.nameEng, item.nameFre, item.nameGer, item.nameJpn, item.nameSpa);
        std::string powderKegArticle = LOCALIZED("the", "la", "TODO_GERMAN", "TODO_JAPANESE", "TODO_SPANISH");
        if (!Ship_IsCStringEmpty(powderKegArticle.c_str())) {
            powderKegArticle += " ";
        }
        CustomMessage::Replace(&entry.msg, "{itemArticle}", itemArticle);
        CustomMessage::Replace(&entry.msg, "{randoItem}", randoItemName);
        CustomMessage::Replace(&entry.msg, "{article}", powderKegArticle);
        CustomMessage::ReplaceSpecialChars(&entry.msg);
        CustomMessage::LoadCustomMessageIntoFont(entry);
        *loadFromMessageTable = false;
    });

    COND_ID_HOOK(OnOpenText, 0x0C86, IS_RANDO, [](u16* textId, bool* loadFromMessageTable) {
        freePowderKegGrantActive = true;

        auto entry = CustomMessage::LoadVanillaMessageTableEntry(*textId);
        Audio_PlaySfx(NA_SE_EN_GOLON_VOICE_EATFULL); // Original script plays this as part of the text.
        entry.msg = LOCALIZED("I heard you cleared the boulder\n"
                              "near the racetrack! Here's the\n"
                              "reward the Elder set aside for \n"
                              "anyone who could do it.",
                              "J'ai entendu que tu as dégagé le rocher\n"
                              "près de la piste! Voici la récompense\n"
                              "que le Doyen a réservée à celui\n"
                              "qui y arriverait.",
                              "TODO_GERMAN", "TODO_JAPANESE", "TODO_SPANISH");

        CustomMessage::ReplaceSpecialChars(&entry.msg);
        CustomMessage::EnsureMessageEnd(&entry.msg);
        CustomMessage::LoadCustomMessageIntoFont(entry);
        *loadFromMessageTable = false;
    });

    COND_ID_HOOK(OnOpenText, 0x0C87, IS_RANDO, [](u16* textId, bool* loadFromMessageTable) {
        if (freePowderKegGrantActive) {
            freePowderKegGrantActive = false;

            auto entry = CustomMessage::LoadVanillaMessageTableEntry(*textId);
            Audio_PlaySfx(NA_SE_EN_GOLON_VOICE_GENERAL); // Original script plays this as part of the text.
            entry.msg = LOCALIZED("Come back if you're interested in\n"
                                  "my %rPowder Kegs%w.",
                                  "Reviens me voir si tu veux\n"
                                  "des %rBarils de Poudre%w.",
                                  "TODO_GERMAN", "TODO_JAPANESE", "TODO_SPANISH");

            CustomMessage::ReplaceSpecialChars(&entry.msg);
            CustomMessage::EnsureMessageEnd(&entry.msg);
            CustomMessage::LoadCustomMessageIntoFont(entry);
            *loadFromMessageTable = false;
        }
    });

    COND_ID_HOOK(OnOpenText, 0x0C8D, IS_RANDO, [](u16* textId, bool* loadFromMessageTable) {
        if (!HAS_ITEM(ITEM_POWDER_KEG)) {
            auto entry = CustomMessage::LoadVanillaMessageTableEntry(*textId);
            Audio_PlaySfx(NA_SE_EN_GOLON_VOICE_GENERAL); // Original script plays this as part of the text.
            entry.msg = LOCALIZED("Sorry, but the Goron Elder revoked\n"
                                  "my ability to certify you to carry\n"
                                  "my %rPowder Kegs%w. You'll have to find\n"
                                  "one somewhere else first, goro.",
                                  "Désolé,mais le Doyen Goron a retiré\n"
                                  "mon droit de te certifier pour porter\n"
                                  "des %rBarils de Poudre%w. Tu devras en trouver\n"
                                  "ailleurs d'abord.",
                                  "TODO_GERMAN", "TODO_JAPANESE", "TODO_SPANISH");

            CustomMessage::ReplaceSpecialChars(&entry.msg);
            CustomMessage::EnsureMessageEnd(&entry.msg);
            CustomMessage::LoadCustomMessageIntoFont(entry);
            *loadFromMessageTable = false;
        }
    });
}
