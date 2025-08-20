#include "ActorBehavior.h"
#include "public/bridge/consolevariablebridge.h"
#include "2s2h/CustomMessage/CustomMessage.h"
#include "2s2h/ShipUtils.h"

extern "C" {
#include "variables.h"
#include "overlays/actors/ovl_En_Geg/z_en_geg.h"
}

void Rando::ActorBehavior::InitEnGegBehavior() {
    COND_ID_HOOK(OnActorInit, ACTOR_EN_GEG, IS_RANDO, [](Actor* actor) { SET_WEEKEVENTREG(WEEKEVENTREG_35_40); });

    COND_VB_SHOULD(VB_GIVE_DON_GERO_MASK, IS_RANDO, {
        EnGeg* refActor = va_arg(args, EnGeg*);

        if (refActor == nullptr || refActor->actor.id != ACTOR_EN_GEG) {
            return;
        }
        SET_WEEKEVENTREG(WEEKEVENTREG_61_01);
        refActor->unk_496 = 0xD75;
        Message_StartTextbox(gPlayState, 0xD75, &refActor->actor);

        *should = false;
    });

    COND_ID_HOOK(OnOpenText, 0xd75, IS_RANDO, [](u16* textId, bool* loadFromMessageTable) {
        auto entry = CustomMessage::LoadVanillaMessageTableEntry(*textId);
        RandoItemId randoItemId = RANDO_SAVE_CHECKS[RC_MOUNTAIN_VILLAGE_DON_GERO_MASK].randoItemId;
        const auto& item = Rando::StaticData::Items[randoItemId];
        std::string article =
            LOCALIZED(item.articleEng, item.articleFre, item.articleGer2, item.articleJpn, item.articleSpa);
        if (!Ship_IsCStringEmpty(article.c_str())) {
            article += " ";
        }
        std::string itemName = LOCALIZED(item.nameEng, item.nameFre, item.nameGer, item.nameJpn, item.nameSpa);
        entry.msg =
            LOCALIZED("I could tell you really wanted {article}%y{itemName}%w! I'm going back to Goron Village.\xE0",
                      "Je savais que tu voulais vraiment {article}%y{itemName}%w!Je retourne au Village Goron.\xE0",
                      "Ich habe gemerkt, dass du {article}%y{{itemName}}%w wirklich haben wolltest! Ich kehre ins Dorf "
                      "zurück.\xE0",
                      "TODO_JAPANESE", "TODO_SPANISH");

        CustomMessage::Replace(&entry.msg, "{article}", article);
        CustomMessage::Replace(&entry.msg, "{itemName}", itemName);
        CustomMessage::ReplaceSpecialChars(&entry.msg);
        CustomMessage::LoadCustomMessageIntoFont(entry);
        *loadFromMessageTable = false;
    });
}