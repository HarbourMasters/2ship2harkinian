#include "ActorBehavior.h"
#include "2s2h/CustomMessage/CustomMessage.h"
#include "2s2h/ShipUtils.h"

extern "C" {
#include "variables.h"
#include "overlays/actors/ovl_En_Kgy/z_en_kgy.h"
}

void Rando::ActorBehavior::InitEnKgyBehavior() {
    COND_VB_SHOULD(VB_SMITHY_START_UPGRADING_SWORD, IS_RANDO, {
        EnKgy* refActor = va_arg(args, EnKgy*);
        RandoSaveCheck& randoGildedSwordSaveCheck = RANDO_SAVE_CHECKS[RC_MOUNTAIN_VILLAGE_SMITHY_GILDED_SWORD];
        RandoSaveCheck& randoRazorSwordSaveCheck = RANDO_SAVE_CHECKS[RC_MOUNTAIN_VILLAGE_SMITHY_RAZOR_SWORD];

        if (randoRazorSwordSaveCheck.cycleObtained) {
            randoGildedSwordSaveCheck.eligible = true;

            // Normally this bit is set to zero when you get your sword back. The DoNotResetRazorSword enhancement uses
            // this bit, so we need to clear it
            gSaveContext.save.saveInfo.permanentSceneFlags[SCENE_KAJIYA].unk_14 &= ~4;
        } else {
            randoRazorSwordSaveCheck.eligible = true;
            // Skip ahead to the textbox that normally plays after receiving the sword
            refActor->actor.textId = 0xC52;
        }

        *should = false;
    });

    COND_VB_SHOULD(VB_SMITHY_CHECK_FOR_RAZOR_SWORD, IS_RANDO, {
        RandoSaveCheck& randoRazorSwordSaveCheck = RANDO_SAVE_CHECKS[RC_MOUNTAIN_VILLAGE_SMITHY_RAZOR_SWORD];
        *should = randoRazorSwordSaveCheck.cycleObtained;
    });

    COND_VB_SHOULD(VB_SMITHY_CHECK_FOR_GILDED_SWORD, IS_RANDO, {
        RandoSaveCheck& randoGildedSwordSaveCheck = RANDO_SAVE_CHECKS[RC_MOUNTAIN_VILLAGE_SMITHY_GILDED_SWORD];
        *should = randoGildedSwordSaveCheck.cycleObtained;
    });

    // "If you want your sword sharpened..." (Razor Sword upgrade)
    COND_ID_HOOK(OnOpenText, 0xc3b, IS_RANDO, [](u16* textId, bool* loadFromMessageTable) {
        auto entry = CustomMessage::LoadVanillaMessageTableEntry(*textId);

        RandoSaveCheck& randoRazorSwordSaveCheck = RANDO_SAVE_CHECKS[RC_MOUNTAIN_VILLAGE_SMITHY_RAZOR_SWORD];
        if (!randoRazorSwordSaveCheck.cycleObtained) {
            const auto& item = Rando::StaticData::Items[randoRazorSwordSaveCheck.randoItemId];
            std::string article =
                LOCALIZED(item.articleEng, item.articleFre, item.articleGer2, item.articleJpn, item.articleSpa);
            if (!Ship_IsCStringEmpty(article.c_str()) &&
                article != "l'") { // Special case handling with l' french article
                article += " ";
            }
            std::string itemName = LOCALIZED(item.nameEng, item.nameFre, item.nameGer, item.nameJpn, item.nameSpa);
            entry.msg =
                LOCALIZED("\nIf you want {{article}}%y{{itemName}}%w, it will cost you %p100 Rupees%w.\n\x10"
                          "So, do we have a deal?\n\xC2%gI'll buy it\nNo thanks\xBF",
                          "\\nSi tu veux {{article}}%y{{itemName}}%w, ça te coûtera %p100 Rubis%w.\\n\\x10"
                          "Alors, marché conclu ?\\n\\xC2%gJ'achète!\\nNon merci\\xBF",
                          "\nWenn du {{article}}%y{{itemName}}%w haben willst, kostet dich das %p100 Rubine%w.\n\x10"
                          "Kommen wir ins Geschäft?\n\xC2%gDas will ich!\nNein, danke!\xBF",
                          "TODO_JAPANESE", "TODO_SPANISH");
            CustomMessage::Replace(&entry.msg, "{{article}}", article);
            CustomMessage::Replace(&entry.msg, "{{itemName}}", itemName);
        }

        CustomMessage::ReplaceSpecialChars(&entry.msg);
        CustomMessage::LoadCustomMessageIntoFont(entry);
        *loadFromMessageTable = false;
    });

    // "With gold dust I can forge the strongest of swords"
    COND_ID_HOOK(OnOpenText, 0xc3d, IS_RANDO, [](u16* textId, bool* loadFromMessageTable) {
        auto entry = CustomMessage::LoadVanillaMessageTableEntry(*textId);
        const auto& itemData =
            Rando::StaticData::Items[RANDO_SAVE_CHECKS[RC_MOUNTAIN_VILLAGE_SMITHY_GILDED_SWORD].randoItemId];
        std::string itemName =
            LOCALIZED(itemData.nameEng, itemData.nameFre, itemData.nameGer, itemData.nameJpn, itemData.nameSpa);
        std::string article = LOCALIZED(itemData.articleEng, itemData.articleFre, itemData.articleGer2,
                                        itemData.articleJpn, itemData.articleSpa);
        if (!Ship_IsCStringEmpty(article.c_str())) {
            article += " ";
        }
        entry.msg = LOCALIZED(
            "Want to know a secret? If you bring me some Gold Dust, I can offer you {{article}}%r{{itemName}}%w.\xE0",
            "Tu veux savoir un secret? Si tu m'apportes de la Poudre d'Or, je peux t'offrir "
            "{{article}}%r{{itemName}}%w.\xE0",
            "Zwar ist es ein Geheimnis, aber pass auf: Bringst du mir etwas %rGoldstaub%w, kann ich dir "
            "{{article}}%r{{itemName}}%w "
            "machen.\xE0",
            "TODO_JAPANESE", "TODO_SPANISH");

        CustomMessage::Replace(&entry.msg, "{{article}}", article);
        CustomMessage::Replace(&entry.msg, "{{itemName}}", itemName);
        CustomMessage::ReplaceSpecialChars(&entry.msg);
        CustomMessage::LoadCustomMessageIntoFont(entry);
        *loadFromMessageTable = false;
    });

    // "Reforge your sword?"
    COND_ID_HOOK(OnOpenText, 0xc3e, IS_RANDO, [](u16* textId, bool* loadFromMessageTable) {
        auto entry = CustomMessage::LoadVanillaMessageTableEntry(*textId);
        entry.msg = LOCALIZED("Back for more?\n\xC2%gYes\nNo", "T'en reveux?\n\xC2%gCarrément\nNon en fait",
                              "Brauchst du mehr?\n\xC2%gJa\nNein", "TODO_JAPANESE", "TODO_SPANISH");

        CustomMessage::ReplaceSpecialChars(&entry.msg);
        CustomMessage::LoadCustomMessageIntoFont(entry);
        *loadFromMessageTable = false;
    });

    // "Come back tomorrow morning"
    COND_ID_HOOK(OnOpenText, 0xc42, IS_RANDO, [](u16* textId, bool* loadFromMessageTable) {
        auto entry = CustomMessage::LoadVanillaMessageTableEntry(*textId);
        entry.msg = LOCALIZED("Thanks for your business.\x19", "Merci pour ton achat.\x19",
                              "Danke für deinen Besuch! Du hast einen guten Handel abgeschlossen.\x19", "TODO_JAPANESE",
                              "TODO_SPANISH");

        CustomMessage::ReplaceSpecialChars(&entry.msg);
        CustomMessage::LoadCustomMessageIntoFont(entry);
        *loadFromMessageTable = false;
    });

    // "Your sword has already been reforged! Unless..."
    COND_ID_HOOK(OnOpenText, 0xc45, IS_RANDO, [](u16* textId, bool* loadFromMessageTable) {
        auto entry = CustomMessage::LoadVanillaMessageTableEntry(*textId);
        entry.msg = LOCALIZED("Did you bring the %rGold Dust%w?\x19", "As-tu apporté la %rPoudre d'Or%w?\x19",
                              "Hast du den %rGoldstaub%w dabei?\x19", "TODO_JAPANESE", "TODO_SPANISH");

        CustomMessage::ReplaceSpecialChars(&entry.msg);
        CustomMessage::LoadCustomMessageIntoFont(entry);
        *loadFromMessageTable = false;
    });

    // "We can use it to reforge your sword"
    COND_ID_HOOK(OnOpenText, 0xc46, IS_RANDO, [](u16* textId, bool* loadFromMessageTable) {
        auto entry = CustomMessage::LoadVanillaMessageTableEntry(*textId);
        entry.msg = LOCALIZED(
            "That's it, alright. I'll just take that off your hands and give you this. Don't tell anyone!\x19",
            "C'est ça,parfait. Je prends ça et je te donne ceci. Ne le dis à personne!\x19",
            "Wenn das kein %rGoldstaub%w ist! Und sogar die feinste Qualität! Ich behalte den mal und gebe dir das "
            "hier. Weil du es bist, werde ich dir nichts berechnen. Aber erzähle niemandem davon!\x19",
            "TODO_JAPANESE", "TODO_SPANISH");

        CustomMessage::ReplaceSpecialChars(&entry.msg);
        CustomMessage::LoadCustomMessageIntoFont(entry);
        *loadFromMessageTable = false;
    });

    // "Gold dust is the prize for winning the Goron race in spring?"
    COND_ID_HOOK(OnOpenText, 0xc49, IS_RANDO, [](u16* textId, bool* loadFromMessageTable) {
        auto entry = CustomMessage::LoadVanillaMessageTableEntry(*textId);
        entry.msg =
            LOCALIZED("Huh? You say that gold dust can be found at %r{{location}}%w?\x19",
                      "Hein?Tu dis que la Poudre d'Or se trouve à %r{{location}}%w?\x19",
                      "Wie? Was?!? Du meinst, dass %r{{location}}%w eine Flasche voll %rGoldstaub%w zu finden ist?\x19",
                      "TODO_JAPANESE", "TODO_SPANISH");
        RandoCheckId randoCheckId = Rando::FindItemPlacement(RI_BOTTLE_GOLD_DUST);
        // TODO HATO: SCENE LOCALIZATION
        CustomMessage::Replace(&entry.msg, "{{location}}",
                               Ship_GetSceneName(Rando::StaticData::Checks[randoCheckId].sceneId));

        CustomMessage::ReplaceSpecialChars(&entry.msg);
        CustomMessage::LoadCustomMessageIntoFont(entry);
        *loadFromMessageTable = false;
    });

    // "Gold dust happens to be first prize at the racetrack"
    COND_ID_HOOK(OnOpenText, 0xc4b, IS_RANDO, [](u16* textId, bool* loadFromMessageTable) {
        auto entry = CustomMessage::LoadVanillaMessageTableEntry(*textId);
        entry.msg = LOCALIZED("Gold dust can be found at %p{{location}}%w.\x10\Bring me that, and "
                              "{{article}}%r{{itemName}}%w is all yours.\xE0",
                              "La Poudre d'Or se trouve à %p{{location}}%w.\x10\Apporte-la moi,et "
                              "{{article}}%r{{itemName}}%w est à toi.\xE0",
                              "%rGoldstaub%w kann %p{{location}}%w gefunden werden.\x10"
                              "Bring mir etwas davon und {{article}}%r{{itemName}}%w gehört dir.\xE0",
                              "TODO_JAPANESE", "TODO_SPANISH");
        RandoCheckId randoCheckId = Rando::FindItemPlacement(RI_BOTTLE_GOLD_DUST);
        const auto& item =
            Rando::StaticData::Items[RANDO_SAVE_CHECKS[RC_MOUNTAIN_VILLAGE_SMITHY_GILDED_SWORD].randoItemId];
        std::string article =
            LOCALIZED(item.articleEng, item.articleFre, item.articleGer, item.articleJpn, item.articleSpa);
        if (!Ship_IsCStringEmpty(article.c_str()) && article != "l'") { // Special case handling with l' french article
            article += " ";
        }
        std::string itemName = LOCALIZED(item.nameEng, item.nameFre, item.nameGer, item.nameJpn, item.nameSpa);
        CustomMessage::Replace(&entry.msg, "{{article}}", article);
        CustomMessage::Replace(&entry.msg, "{{itemName}}", itemName);
        // TODO HATO: SCENE LOCALIZATION
        CustomMessage::Replace(&entry.msg, "{{location}}",
                               Ship_GetSceneName(Rando::StaticData::Checks[randoCheckId].sceneId));

        CustomMessage::ReplaceSpecialChars(&entry.msg);
        CustomMessage::LoadCustomMessageIntoFont(entry);
        *loadFromMessageTable = false;
    });

    // "Your sword is already as strong as I can make it!"
    COND_ID_HOOK(OnOpenText, 0xc4c, IS_RANDO, [](u16* textId, bool* loadFromMessageTable) {
        auto entry = CustomMessage::LoadVanillaMessageTableEntry(*textId);
        entry.msg = LOCALIZED("Hey, what is this? I'm not made of Randomizer Checks!\x19",
                              "Hé,c'est quoi ça? Je ne suis pas fait de Checks de Rando!\x19",
                              "Hey! Hey! Was soll das? Das waren alle meine Randomizer Checks!\x19", "TODO_JAPANESE",
                              "TODO_SPANISH");

        CustomMessage::ReplaceSpecialChars(&entry.msg);
        CustomMessage::LoadCustomMessageIntoFont(entry);
        *loadFromMessageTable = false;
    });
}
