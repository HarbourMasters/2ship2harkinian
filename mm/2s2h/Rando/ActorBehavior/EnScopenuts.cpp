#include "ActorBehavior.h"
#include "public/bridge/consolevariablebridge.h"
#include "2s2h/CustomMessage/CustomMessage.h"
#include "2s2h/ShipUtils.h"

extern "C" {
#include "variables.h"
#include "overlays/actors/ovl_En_Scopenuts/z_en_scopenuts.h"

void Player_TalkWithPlayer(PlayState* play, Actor* actor);
void func_80BCB980(EnScopenuts* enScopenuts, PlayState* play);
}

void Rando::ActorBehavior::InitEnScopenutsBehavior() {
    COND_VB_SHOULD(VB_GIVE_ITEM_FROM_OFFER, IS_RANDO, {
        GetItemId* item = va_arg(args, GetItemId*);
        Actor* actor = va_arg(args, Actor*);
        Player* player = GET_PLAYER(gPlayState);
        if (actor->id == ACTOR_EN_SCOPENUTS) {
            EnScopenuts* enScopenuts = (EnScopenuts*)actor;
            *should = false;
            SET_WEEKEVENTREG(WEEKEVENTREG_53_02);
            actor->parent = &player->actor;
            player->talkActor = actor;
            player->talkActorDistance = actor->xzDistToPlayer;
            player->exchangeItemAction = PLAYER_IA_MINUS1;
            Player_TalkWithPlayer(gPlayState, actor);
            enScopenuts->actionFunc = func_80BCB980;
        }
    });

    COND_ID_HOOK(OnOpenText, 0x1631, IS_RANDO, [](u16* textId, bool* loadFromMessageTable) {
        auto entry = CustomMessage::LoadVanillaMessageTableEntry(*textId);
        RandoItemId randoItemId = RANDO_SAVE_CHECKS[RC_TERMINA_FIELD_GROTTO_SCRUB].randoItemId;
        const auto& item = Rando::StaticData::Items[randoItemId];
        entry.msg = LOCALIZED(
            "Please! I'll sell you {article}%y{itemName}%w if you just keep this place a secret...\xE0",
            "S'il te plaît! Je te vends {article}%y{itemName}%w si tu gardes cet endroit secret...\xE0",
            "Oh, bitte! Ich verkaufe dir {article}%y{{itemName}}%w, aber bitte behalte dieses Geheimnis für dich!\xE0",
            "TODO_JAPANESE", "TODO_SPANISH");

        std::string article =
            LOCALIZED(item.articleEng, item.articleFre, item.articleGer2, item.articleJpn, item.articleSpa);
        if (!Ship_IsCStringEmpty(article.c_str())) {
            article += " ";
        }
        std::string itemName = LOCALIZED(item.nameEng, item.nameFre, item.nameGer, item.nameJpn, item.nameSpa);
        CustomMessage::Replace(&entry.msg, "{article}", article);
        CustomMessage::Replace(&entry.msg, "{itemName}", itemName);
        CustomMessage::ReplaceSpecialChars(&entry.msg);
        CustomMessage::LoadCustomMessageIntoFont(entry);
        *loadFromMessageTable = false;
    });

    COND_VB_SHOULD(VB_SCOPENUTS_CONSIDER_FIRST_CYCLE, IS_RANDO, { *should = false; });
}
