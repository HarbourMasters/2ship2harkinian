#include "ActorBehavior.h"
#include "public/bridge/consolevariablebridge.h"
#include "2s2h/CustomMessage/CustomMessage.h"
#include "2s2h/ShipUtils.h"

extern "C" {
#include "variables.h"
#include "src/overlays/actors/ovl_En_Kujiya/z_en_kujiya.h"
void EnKujiya_Wait(EnKujiya* enKujiya, PlayState* play);
}

void Rando::ActorBehavior::InitEnKujiyaBehavior() {
    COND_VB_SHOULD(VB_GIVE_LOTTERY_WINNINGS, IS_RANDO, {
        EnKujiya* refActor = va_arg(args, EnKujiya*);

        RANDO_SAVE_CHECKS[RC_CLOCK_TOWN_WEST_LOTTERY].eligible = true;
        refActor->actionFunc = EnKujiya_Wait;
        *should = false;
    });

    COND_ID_HOOK(OnOpenText, 0x2b5c, IS_RANDO, [](u16* textId, bool* loadFromMessageTable) {
        auto entry = CustomMessage::LoadVanillaMessageTableEntry(*textId);
        entry.msg = LOCALIZED("Step right up! For a measly %p10 Rupees%w, your dreams could come true!\x11\x13\x12"
                              "Guess all three numbers to win {article}%p{itemName}%w!\x19",
                              "Approchez!Pour seulement %p10 Rubis%w,votre rêve peut devenir réalité!\x11\x13\x12"
                              "Trouvez les trois numéros pour gagner {article}%p{itemName}%w!\x19",
                              "TODO_GERMAN", "TODO_JAPANESE", "TODO_SPANISH");
        RandoItemId randoItemId = RANDO_SAVE_CHECKS[RC_CLOCK_TOWN_WEST_LOTTERY].randoItemId;
        const auto& item = Rando::StaticData::Items[randoItemId];
        std::string article =
            LOCALIZED(item.articleEng, item.articleFre, item.articleGer, item.articleJpn, item.articleSpa);
        if (!Ship_IsCStringEmpty(article.c_str())) {
            article += " ";
        }
        std::string itemName = LOCALIZED(item.nameEng, item.nameFre, item.nameGer, item.nameJpn, item.nameSpa);
        CustomMessage::Replace(&entry.msg, "{article}", article);
        CustomMessage::Replace(&entry.msg, "{itemName}", itemName);

        CustomMessage::LoadCustomMessageIntoFont(entry);
        *loadFromMessageTable = false;
    });

    COND_ID_HOOK(OnOpenText, 0x2b66, IS_RANDO, [](u16* textId, bool* loadFromMessageTable) {
        auto entry = CustomMessage::LoadVanillaMessageTableEntry(*textId);
        entry.msg = LOCALIZED("Congratulations! You win the jackpot:{article}%p{itemName}%w!\x19",
                              "Félicitations! Vous remportez le gros lot:{article}%p{itemName}%w!\x19", "TODO_GERMAN",
                              "TODO_JAPANESE", "TODO_SPANISH");
        RandoItemId randoItemId = RANDO_SAVE_CHECKS[RC_CLOCK_TOWN_WEST_LOTTERY].randoItemId;
        const auto& item = Rando::StaticData::Items[randoItemId];
        std::string article =
            LOCALIZED(item.articleEng, item.articleFre, item.articleGer, item.articleJpn, item.articleSpa);
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
}