#include "ActorBehavior.h"
#include "public/bridge/consolevariablebridge.h"
#include "2s2h/CustomMessage/CustomMessage.h"
#include "2s2h/ShipUtils.h"

extern "C" {
#include "variables.h"
#include "src/overlays/actors/ovl_En_Gb2/z_en_gb2.h"

void Player_TalkWithPlayer(PlayState* play, Actor* actor);
}

void Rando::ActorBehavior::InitEnGb2Behavior() {
    COND_VB_SHOULD(VB_GIVE_ITEM_FROM_OFFER, IS_RANDO, {
        GetItemId* item = va_arg(args, GetItemId*);
        Actor* refActor = va_arg(args, Actor*);
        Player* player = GET_PLAYER(gPlayState);

        // Do not override the vanilla Purple rupee grant
        if (refActor->id != ACTOR_EN_GB2 || *item != GI_HEART_PIECE) {
            return;
        }

        if (!RANDO_SAVE_CHECKS[RC_IKANA_CANYON_GHOST_HUT_PIECE_OF_HEART].cycleObtained) {
            RANDO_SAVE_CHECKS[RC_IKANA_CANYON_GHOST_HUT_PIECE_OF_HEART].eligible = true;
        }

        *should = false;

        refActor->parent = &player->actor;
        player->talkActor = refActor;
        player->talkActorDistance = refActor->xzDistToPlayer;
        player->exchangeItemAction = PLAYER_IA_MINUS1;
        Player_TalkWithPlayer(gPlayState, refActor);
        /*
         * This actor sets MSGMODE_TEXT_CLOSING state and expects GI to set it back to MSGMODE_TEXT_START. Because the
         * GI is skipped, we manually start the textbox to prevent the player from being able to move during dialog.
         */
        Message_StartTextbox(gPlayState, 0x14DE, refActor);
    });

    COND_ID_HOOK(OnOpenText, 0x14D1, IS_RANDO, [](u16* textId, bool* loadFromMessageTable) {
        if (RANDO_SAVE_CHECKS[RC_IKANA_CANYON_GHOST_HUT_PIECE_OF_HEART].obtained) {
            return;
        }

        const auto& item = Rando::StaticData::Items[RANDO_SAVE_CHECKS[RC_IKANA_CANYON_GHOST_HUT_PIECE_OF_HEART].randoItemId];
        std::string article = LOCALIZED(item.articleEng, item.articleFre, item.articleGer, item.articleJpn, item.articleSpa);
        if (!Ship_IsCStringEmpty(article.c_str())) {
            article += " ";
        }
        std::string checkItemName = LOCALIZED(item.nameEng, item.nameFre, item.nameGer, item.nameJpn, item.nameSpa);

        auto entry = CustomMessage::LoadVanillaMessageTableEntry(*textId);
        entry.msg = LOCALIZED(
            "If you are seeking the one who is\n"
            "%rstrong%w than you are, you may find\n"
            "{article}%g{{itemName}}%w here...\n"
            "\x10"
            "from a group of spirits plagued by\n"
            "lingering regrets.\xE0",
            "Si tu cherches celui qui est\n"
            "%rplus fort%w que toi, tu trouveras peut-être\n"
            "{article}%g{{itemName}}%w ici...\n"
            "\x10"
            "parmi des esprits tourmentés par\n"
            "des regrets persistants.\xE0",
            "TODO_GERMAN", "TODO_JAPANESE", "TODO_SPANISH");

        CustomMessage::Replace(&entry.msg, "{article}", article);
        CustomMessage::Replace(&entry.msg, "{{itemName}}", checkItemName);
        CustomMessage::ReplaceSpecialChars(&entry.msg);
        CustomMessage::LoadCustomMessageIntoFont(entry);
        *loadFromMessageTable = false;
    });
}
