#include "ActorBehavior.h"
#include "public/bridge/consolevariablebridge.h"
#include "2s2h/CustomMessage/CustomMessage.h"
#include "2s2h/ShipUtils.h"

extern "C" {
#include "variables.h"
void Player_TalkWithPlayer(PlayState* play, Actor* actor);
}

void Rando::ActorBehavior::InitEnJgameTsnBehavior() {
    COND_VB_SHOULD(VB_GIVE_ITEM_FROM_OFFER, IS_RANDO, {
        GetItemId* getItemId = va_arg(args, GetItemId*);
        Actor* actor = va_arg(args, Actor*);
        if (actor->id == ACTOR_EN_JGAME_TSN &&
            !RANDO_SAVE_CHECKS[RC_GREAT_BAY_COAST_FISHERMAN_MINIGAME].cycleObtained) {
            *should = false;
            Player* player = GET_PLAYER(gPlayState);
            actor->parent = &player->actor;
            player->talkActor = actor;
            player->talkActorDistance = actor->xzDistToPlayer;
            player->exchangeItemAction = PLAYER_IA_MINUS1;
            Player_TalkWithPlayer(gPlayState, actor);
        }
    });

    // Fisherman "Want to play the jumping game for a prize?"
    COND_ID_HOOK(OnOpenText, 0x1096, IS_RANDO, [](u16* textId, bool* loadFromMessageTable) {
        auto randoSaveCheck = RANDO_SAVE_CHECKS[RC_GREAT_BAY_COAST_FISHERMAN_MINIGAME];

        auto entry = CustomMessage::LoadVanillaMessageTableEntry(*textId);
        entry.msg = LOCALIZED("Want to try my %rjumping game%w for %p20 Rupees%w? Win, and I'll give you "
                              "{{article}}%r{{itemName}}%w!\x19\xA8",
                              "Tu veux essayer mon %rjeu de saut%w pour %p20 Rubis%w? Si tu gagnes,je te donne "
                              "{{article}}%r{{itemName}}%w!\x19\xA8",
                              "Für nur %p20 Rubine%w darfst du an einem %rSpiel%w teilnehmen, bei dem es "
                              "{{article}}%r{{itemName}}%w zu gewinnen gibt!\x19\xA8",
                              "TODO_JAPANESE", "TODO_SPANISH");

        std::string itemName;
        std::string article;
        // The same-cycle repeat reward is a purple Rupee
        if (randoSaveCheck.cycleObtained) {
            article = "";
            itemName = LOCALIZED("50 Rupees", "50 Rubis", "50 Rubine", "TODO_JAPANESE", "TODO_SPANISH");
        } else {
            const auto& item = Rando::StaticData::Items[randoSaveCheck.randoItemId];
            article = LOCALIZED(item.articleEng, item.articleFre, item.articleGer2, item.articleJpn, item.articleSpa);
            if (!Ship_IsCStringEmpty(article.c_str()) &&
                article != "l'") { // Special case handling with l' french article
                article += " ";
            }
            itemName = LOCALIZED(item.nameEng, item.nameFre, item.nameGer, item.nameJpn, item.nameSpa);
        }
        CustomMessage::Replace(&entry.msg, "{{article}}", article);
        CustomMessage::Replace(&entry.msg, "{{itemName}}", itemName);

        CustomMessage::ReplaceSpecialChars(&entry.msg);
        CustomMessage::LoadCustomMessageIntoFont(entry);
        *loadFromMessageTable = false;
    });
}
