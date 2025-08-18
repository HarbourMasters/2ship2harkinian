#include "ActorBehavior.h"
#include "public/bridge/consolevariablebridge.h"
#include "2s2h/CustomMessage/CustomMessage.h"

extern "C" {
#include "variables.h"
#include "overlays/actors/ovl_En_Fsn/z_en_fsn.h"
#include "overlays/actors/ovl_En_GirlA/z_en_girla.h"
#include "overlays/actors/ovl_En_Ossan/z_en_ossan.h"
#include "overlays/actors/ovl_En_Sob1/z_en_sob1.h"
#include "overlays/actors/ovl_En_Trt/z_en_trt.h"

void EnGirlA_Update2(EnGirlA* enGirlA, PlayState* play);
void EnGirlA_DoNothing(EnGirlA* enGirlA, PlayState* play);
void EnGirlA_SetupAction(EnGirlA* enGirlA, EnGirlAActionFunc action);
}

#define RANDO_DESC_TEXT_ID 0x083F
#define RANDO_CHOICE_TEXT_ID 0x0840

static const std::vector<std::string> flavorTextsEng = {
    "Buy it, you won't regret it!",   "A must-have for any adventurer!", "A great gift for a friend!",
    "One of a kind, don't miss out!", "A great deal for the price!",     "On sale for a limited time!",
    "Get it while it's hot!",         "Don't miss out on this deal!",
};

static const std::vector<std::string> flavorTextsFre = {
    "Achetez-le, vous ne le regretterez pas!", "Un indispensable pour tout aventurier!",
    "Un excellent cadeau pour un ami!",        "Unique en son genre, ne le ratez pas!",
    "Une excellente affaire pour le prix!",    "En vente pour une durée limitée!",
    "Prenez-le tant qu'il est chaud!",         "Ne ratez pas cette offre!",
};

static const std::vector<std::string> flavorTextsGer = {
    "Kaufen Sie es, Sie werden es nicht bereuen!", "Ein Muss für jeden Abenteurer!",
    "Ein tolles Geschenk für einen Freund!",       "Einzigartig, verpassen Sie es nicht!",
    "Ein großartiges Angebot für den Preis!",      "Nur für begrenzte Zeit im Angebot!",
    "Holen Sie es sich, solange es heiß ist!",     "Verpassen Sie dieses Angebot nicht!",
};

static const std::vector<std::string> flavorTextsSpa = {
    "¡Cómpralo, no te arrepentirás!",      "¡Imprescindible para cualquier aventurero!",
    "¡Un gran regalo para un amigo!",      "¡Único, no te lo pierdas!",
    "¡Una gran oferta por el precio!",     "¡En oferta por tiempo limitado!",
    "¡Consíguelo mientras esté caliente!", "¡No te pierdas esta oferta!",
};

static const std::vector<std::string> flavorTextsJpn = {
    "買って損はしません！",   "冒険者必携のアイテム！",       "友達への素敵なプレゼント！",
    "一点物、お見逃しなく！", "この価格でこの品質は超お得！", "期間限定セール中！",
    "今が買い時です！",       "このチャンスをお見逃しなく！",
};

void EnGirlA_RandoDrawFunc(Actor* actor, PlayState* play) {
    EnGirlA* enGirlA = (EnGirlA*)actor;

    auto randoSaveCheck = RANDO_SAVE_CHECKS[actor->world.rot.z];

    Matrix_RotateYS(enGirlA->rotY, MTXMODE_APPLY);

    Rando::DrawItem(randoSaveCheck.randoItemId, actor);
}

void EnGirlA_RandoBought(PlayState* play, EnGirlA* enGirlA) {
    enGirlA->isOutOfStock = true;
    enGirlA->actor.draw = NULL;
}

void EnGirlA_RandoRestock(PlayState* play, EnGirlA* enGirlA) {
    auto randoSaveCheck = RANDO_SAVE_CHECKS[enGirlA->actor.world.rot.z];

    if (Rando::IsItemObtainable(randoSaveCheck.randoItemId, (RandoCheckId)enGirlA->actor.world.rot.z)) {
        enGirlA->isOutOfStock = false;
        enGirlA->actor.draw = EnGirlA_RandoDrawFunc;
    }
}

s32 EnGirlA_RandoCanBuyFunc(PlayState* play, EnGirlA* enGirlA) {
    if (gSaveContext.save.saveInfo.playerData.rupees < play->msgCtx.unk1206C) {
        return CANBUY_RESULT_NEED_RUPEES;
    }

    auto randoSaveCheck = RANDO_SAVE_CHECKS[enGirlA->actor.world.rot.z];

    if (!Rando::IsItemObtainable(randoSaveCheck.randoItemId, (RandoCheckId)enGirlA->actor.world.rot.z)) {
        return CANBUY_RESULT_CANNOT_GET_NOW;
    }

    return CANBUY_RESULT_SUCCESS_2;
}

void EnGirlA_RandoBuyFunc(PlayState* play, EnGirlA* enGirlA) {
    auto& randoSaveCheck = RANDO_SAVE_CHECKS[enGirlA->actor.world.rot.z];
    RandoItemId randoItemId = Rando::ConvertItem(randoSaveCheck.randoItemId, (RandoCheckId)enGirlA->actor.world.rot.z);
    randoSaveCheck.obtained = true;
    Rupees_ChangeBy(-play->msgCtx.unk1206C);
    Rando::GiveItem(randoItemId);
}

void EnGirlA_RandoBuyFanfareFunc(PlayState* play, EnGirlA* enGirlA) {
    // No-op, if we made it here something went wrong
}

void EnGirlA_RandoInit(EnGirlA* enGirlA, PlayState* play) {
    enGirlA->actor.flags &= ~ACTOR_FLAG_10;
    enGirlA->actor.textId = RANDO_DESC_TEXT_ID;
    enGirlA->choiceTextId = RANDO_CHOICE_TEXT_ID;

    enGirlA->boughtFunc = EnGirlA_RandoBought;
    enGirlA->restockFunc = EnGirlA_RandoRestock;
    enGirlA->canBuyFunc = EnGirlA_RandoCanBuyFunc;
    enGirlA->buyFunc = EnGirlA_RandoBuyFunc;
    enGirlA->buyFanfareFunc = EnGirlA_RandoBuyFanfareFunc;

    enGirlA->actor.flags &= ~ACTOR_FLAG_TARGETABLE;
    Actor_SetScale(&enGirlA->actor, 0.25f);
    enGirlA->actor.shape.yOffset = 24.0f;
    enGirlA->actor.shape.shadowScale = 4.0f;
    enGirlA->actor.floorHeight = enGirlA->actor.home.pos.y;
    enGirlA->actor.gravity = 0.0f;
    EnGirlA_SetupAction(enGirlA, EnGirlA_DoNothing);
    enGirlA->isInitialized = true;
    enGirlA->mainActionFunc = EnGirlA_Update2;
    enGirlA->isSelected = false;
    enGirlA->rotY = 0;
    enGirlA->initialRotY = enGirlA->actor.shape.rot.y;

    auto randoSaveCheck = RANDO_SAVE_CHECKS[enGirlA->actor.world.rot.z];

    if (!Rando::IsItemObtainable(randoSaveCheck.randoItemId, (RandoCheckId)enGirlA->actor.world.rot.z) &&
        randoSaveCheck.obtained) {
        enGirlA->isOutOfStock = true;
        enGirlA->actor.draw = NULL;
    } else {
        enGirlA->isOutOfStock = false;
        enGirlA->actor.draw = EnGirlA_RandoDrawFunc;
    }
}

void renameStolenBombBag(u16* textId, bool* loadFromMessageTable) {
    auto randoSaveCheck = RANDO_SAVE_CHECKS[RC_BOMB_SHOP_ITEM_04_OR_CURIOSITY_SHOP_ITEM];
    auto randoStaticItem = Rando::StaticData::Items[randoSaveCheck.randoItemId];
    auto entry = CustomMessage::LoadVanillaMessageTableEntry(*textId);

    entry.msg = LOCALIZED(
        "Tonight's special, stolen from the Bomb Shop: %r{{itemName}}%w. Check it out!\x19\xA8",
        "Spécial de ce soir, volé au Magasin de Bombes: %r{{itemName}}%w. Jetes-y un œil!\x19\xA8",
        "Heute Abend spezial, gestohlen aus dem Bomben-Shop: %r{{itemName}}%w. Schau es dir an!\x19\xA8",
        "今夜の特別品、爆弾屋から盗んだ: %r{{itemName}}%w。チェックしてみて！\x19\xA8",
        "Especial de esta noche, robado de la Tienda de Bombas: %r{{itemName}}%w. ¡Échale un vistazo!\x19\xA8");

    std::string itemName = LOCALIZED(randoStaticItem.nameEng, randoStaticItem.nameFre, randoStaticItem.nameGer,
                                     randoStaticItem.nameJpn, randoStaticItem.nameSpa);

    CustomMessage::Replace(&entry.msg, "{{itemName}}", itemName);
    CustomMessage::ReplaceSpecialChars(&entry.msg);
    CustomMessage::LoadCustomMessageIntoFont(entry);
    *loadFromMessageTable = false;
}

void renameSpecialBargain(u16* textId, bool* loadFromMessageTable) {
    auto randoSaveCheck = RANDO_SAVE_CHECKS[RC_CURIOSITY_SHOP_SPECIAL_ITEM];
    auto randoStaticItem = Rando::StaticData::Items[randoSaveCheck.randoItemId];
    auto entry = CustomMessage::LoadVanillaMessageTableEntry(*textId);

    entry.msg = LOCALIZED("Tonight's bargain: %r{{itemName}}%w. Check it out!\x19\xA8",
                          "L'affaire de ce soir: %r{{itemName}}%w. Jetes-y un œil!\x19\xA8",
                          "Heute Abend Schnäppchen: %r{{itemName}}%w. Schau es dir an!\x19\xA8",
                          "今夜のバーゲン: %r{{itemName}}%w。チェックしてみて！\x19\xA8",
                          "Ganga de esta noche: %r{{itemName}}%w. ¡Échale un vistazo!\x19\xA8");

    std::string itemName = LOCALIZED(randoStaticItem.nameEng, randoStaticItem.nameFre, randoStaticItem.nameGer,
                                     randoStaticItem.nameJpn, randoStaticItem.nameSpa);

    CustomMessage::Replace(&entry.msg, "{{itemName}}", itemName);
    CustomMessage::ReplaceSpecialChars(&entry.msg);
    CustomMessage::LoadCustomMessageIntoFont(entry);
    *loadFromMessageTable = false;
}

void ReplaceCannotBuyMessage(u16* textId, bool* loadFromMessageTable) {
    CustomMessage::Entry entry = {
        .msg = LOCALIZED("Sorry, you can't buy this right now.\xE0",
                         "Désolé, vous ne pouvez pas acheter ceci maintenant.\xE0",
                         "Entschuldigung, Sie können das gerade nicht kaufen.\xE0",
                         "すみません、今これは買えません。\xE0", "Lo siento, no puedes comprar esto ahora.\xE0"),
    };
    CustomMessage::ReplaceSpecialChars(&entry.msg);
    CustomMessage::LoadCustomMessageIntoFont(entry);
    *loadFromMessageTable = false;
}

RandoCheckId IdentifyShopItem(Actor* actor) {
    switch (gPlayState->sceneId) {
        case SCENE_8ITEMSHOP:
            switch (actor->params) {
                case 10:
                case 18:
                    return RC_TRADING_POST_SHOP_ITEM_01;
                case 5:
                case 14:
                    return RC_TRADING_POST_SHOP_ITEM_02;
                case 6:
                case 17:
                    return RC_TRADING_POST_SHOP_ITEM_03;
                case 3:
                case 11:
                    return RC_TRADING_POST_SHOP_ITEM_04;
                case 7:
                case 16:
                    return RC_TRADING_POST_SHOP_ITEM_05;
                case 8:
                case 12:
                    return RC_TRADING_POST_SHOP_ITEM_06;
                case 9:
                case 15:
                    return RC_TRADING_POST_SHOP_ITEM_07;
                case 4:
                case 13:
                    return RC_TRADING_POST_SHOP_ITEM_08;
            }
            break;
        case SCENE_BOMYA:
            switch (actor->params) {
                case 26:
                    return RC_BOMB_SHOP_ITEM_01;
                case 25:
                    return RC_BOMB_SHOP_ITEM_02;
                case 24: // After saving Bomb Shop lady
                    return RC_BOMB_SHOP_ITEM_04_OR_CURIOSITY_SHOP_ITEM;
                case 23:
                    return RC_BOMB_SHOP_ITEM_03;
            }
            break;
        case SCENE_WITCH_SHOP:
            switch (actor->params) {
                case 2:
                    return RC_HAGS_POTION_SHOP_ITEM_01;
                case 1:
                    return RC_HAGS_POTION_SHOP_ITEM_02;
                case 0:
                    return RC_HAGS_POTION_SHOP_ITEM_03;
            }
            break;
        case SCENE_GORONSHOP:
            switch (actor->params) {
                case 30:
                case 33:
                    return RC_GORON_SHOP_ITEM_01;
                case 31:
                case 34:
                    return RC_GORON_SHOP_ITEM_02;
                case 32:
                case 35:
                    return RC_GORON_SHOP_ITEM_03;
            }
            break;
        case SCENE_BANDROOM:
            switch (actor->params) {
                case 27:
                    return RC_ZORA_SHOP_ITEM_01;
                case 28:
                    return RC_ZORA_SHOP_ITEM_02;
                case 29:
                    return RC_ZORA_SHOP_ITEM_03;
            }
            break;
        case SCENE_AYASHIISHOP:
            switch (actor->params) {
                case 19: // Saved Bomb Shop lady and recovered big bomb bag, so a new item is in stock
                    return RC_CURIOSITY_SHOP_SPECIAL_ITEM;
                case 21: // Sakon stole the bomb bag, so the bomb shop check is in stock
                    return RC_BOMB_SHOP_ITEM_04_OR_CURIOSITY_SHOP_ITEM;
            }
            break;
    }

    return RC_UNKNOWN;
}

RandoCheckId IdentifyActiveShopItem() {
    RandoCheckId randoCheckId = RC_UNKNOWN;

    if (gPlayState->msgCtx.talkActor == nullptr) {
        return RC_UNKNOWN;
    }

    if (gPlayState->msgCtx.talkActor->id == ACTOR_EN_TRT) {
        EnTrt* enTrt = (EnTrt*)gPlayState->msgCtx.talkActor;
        if (enTrt->items[enTrt->cursorIndex] != nullptr) {
            randoCheckId = (RandoCheckId)enTrt->items[enTrt->cursorIndex]->actor.world.rot.z;
        }
    } else if (gPlayState->msgCtx.talkActor->id == ACTOR_EN_OSSAN) {
        if (gPlayState->msgCtx.talkActor->params == 0 || gPlayState->msgCtx.talkActor->params == 1) {
            EnOssan* enOssan = (EnOssan*)gPlayState->msgCtx.talkActor;
            if (enOssan->items[enOssan->cursorIndex] != nullptr) {
                randoCheckId = (RandoCheckId)enOssan->items[enOssan->cursorIndex]->actor.world.rot.z;
            }
        } else {
            EnSob1* enSob1 = (EnSob1*)gPlayState->msgCtx.talkActor;
            if (enSob1->items[enSob1->cursorIndex] != nullptr) {
                randoCheckId = (RandoCheckId)enSob1->items[enSob1->cursorIndex]->actor.world.rot.z;
            }
        }
    } else if (gPlayState->msgCtx.talkActor->id == ACTOR_EN_FSN) {
        EnFsn* enFsn = (EnFsn*)gPlayState->msgCtx.talkActor;
        if (enFsn->items[enFsn->cursorIndex] != nullptr) {
            randoCheckId = (RandoCheckId)enFsn->items[enFsn->cursorIndex]->actor.world.rot.z;
        }
    }

    return randoCheckId;
}

void Rando::ActorBehavior::InitEnGirlABehavior() {
    COND_ID_HOOK(OnActorInit, ACTOR_EN_GIRLA, IS_RANDO, [](Actor* actor) {
        EnGirlA* enGirlA = (EnGirlA*)actor;

        RandoCheckId randoCheckId = IdentifyShopItem(actor);
        if (randoCheckId != RC_UNKNOWN && RANDO_SAVE_CHECKS[randoCheckId].shuffled) {
            enGirlA->actor.world.rot.z = randoCheckId;
            enGirlA->mainActionFunc = EnGirlA_RandoInit;
        }
    });

    // Shop item description
    COND_ID_HOOK(OnOpenText, RANDO_DESC_TEXT_ID, IS_RANDO, [](u16* textId, bool* loadFromMessageTable) {
        RandoCheckId randoCheckId = IdentifyActiveShopItem();

        if (randoCheckId == RC_UNKNOWN) {
            return;
        }

        auto randoSaveCheck = RANDO_SAVE_CHECKS[randoCheckId];
        auto randoStaticItem = Rando::StaticData::Items[randoSaveCheck.randoItemId];

        auto entry = CustomMessage::LoadVanillaMessageTableEntry(*textId);
        entry.autoFormat = false;

        entry.msg =
            LOCALIZED("\x01{{itemName}}: {{rupees}} Rupees\x11\x00", "\x01{{itemName}}: {{rupees}} Rubis\x11\x00",
                      "\x01{{itemName}}: {{rupees}} Rubine\x11\x00", "\x01{{itemName}}: {{rupees}}ルピー\x11\x00",
                      "\x01{{itemName}}: {{rupees}} Rupias\x11\x00");
        entry.msg += '\x00';

        std::string itemName = LOCALIZED(randoStaticItem.nameEng, randoStaticItem.nameFre, randoStaticItem.nameGer,
                                         randoStaticItem.nameJpn, randoStaticItem.nameSpa);

        CustomMessage::Replace(&entry.msg, "{{itemName}}", itemName);
        CustomMessage::Replace(&entry.msg, "{{rupees}}", std::to_string(randoSaveCheck.price));

        if (!Rando::IsItemObtainable(randoSaveCheck.randoItemId, randoCheckId) && randoSaveCheck.obtained) {
            entry.msg += LOCALIZED("Out of Stock", "Rupture de stock", "Ausverkauft", "売り切れ", "Agotado");
        } else {
            // Use localized flavor texts
            const std::vector<std::string>* flavorTexts = nullptr;
            switch (gSaveContext.options.language) {
                case LANGUAGE_FRE:
                    flavorTexts = &flavorTextsFre;
                    break;
                case LANGUAGE_GER:
                    flavorTexts = &flavorTextsGer;
                    break;
                case LANGUAGE_JPN:
                    flavorTexts = &flavorTextsJpn;
                    break;
                case LANGUAGE_SPA:
                    flavorTexts = &flavorTextsSpa;
                    break;
                case LANGUAGE_ENG:
                default:
                    flavorTexts = &flavorTextsEng;
                    break;
            }
            entry.msg += (*flavorTexts)[rand() % flavorTexts->size()];
        }
        entry.msg += "\x1A\xBF";
        CustomMessage::ReplaceSpecialChars(&entry.msg);
        CustomMessage::LoadCustomMessageIntoFont(entry);
        *loadFromMessageTable = false;
    });

    // Magic Potion Shop Hag "I can't get the ingredients for this"
    COND_ID_HOOK(OnOpenText, 0x880, IS_RANDO, [](u16* textId, bool* loadFromMessageTable) {
        RandoCheckId randoCheckId = IdentifyActiveShopItem();

        if (randoCheckId == RC_UNKNOWN) {
            return;
        }

        auto randoSaveCheck = RANDO_SAVE_CHECKS[randoCheckId];
        auto randoStaticItem = Rando::StaticData::Items[randoSaveCheck.randoItemId];

        auto entry = CustomMessage::LoadVanillaMessageTableEntry(*textId);
        entry.autoFormat = false;
        entry.firstItemCost = randoSaveCheck.price;

        entry.msg =
            LOCALIZED("\x01{{itemName}}: {{itemPrice}} Rupees\x11\x00", "\x01{{itemName}}: {{itemPrice}} Rubis\x11\x00",
                      "\x01{{itemName}}: {{itemPrice}} Rubine\x11\x00", "\x01{{itemName}}: {{itemPrice}}ルピー\x11\x00",
                      "\x01{{itemName}}: {{itemPrice}} Rupias\x11\x00");
        entry.msg += '\x00';
        entry.msg += LOCALIZED("I need a mushroom to make this.\x1A", "J'ai besoin d'un champignon pour faire ça.\x1A",
                               "Ich brauche einen Pilz, um das zu machen.\x1A", "これを作るにはキノコが必要です。\x1A",
                               "Necesito una seta para hacer esto.\x1A");

        std::string itemName = LOCALIZED(randoStaticItem.nameEng, randoStaticItem.nameFre, randoStaticItem.nameGer,
                                         randoStaticItem.nameJpn, randoStaticItem.nameSpa);

        CustomMessage::Replace(&entry.msg, "{{itemName}}", itemName);
        CustomMessage::Replace(&entry.msg, "{{itemPrice}}", std::to_string(randoSaveCheck.price));
        CustomMessage::ReplaceSpecialChars(&entry.msg);
        CustomMessage::LoadCustomMessageIntoFont(entry);
        *loadFromMessageTable = false;
    });

    // Magic Potion Shop Hag "Well, I can use this to make something, come back later"
    COND_ID_HOOK(OnOpenText, 0x884, IS_RANDO, [](u16* textId, bool* loadFromMessageTable) {
        RandoCheckId randoCheckId = RC_HAGS_POTION_SHOP_ITEM_01;
        auto& randoSaveCheck = RANDO_SAVE_CHECKS[randoCheckId];

        if (!randoSaveCheck.shuffled || randoSaveCheck.eligible) {
            return;
        }

        auto entry = CustomMessage::LoadVanillaMessageTableEntry(*textId);
        entry.msg = LOCALIZED("I used this to make %r{{itemName}}%w, take it!\x19",
                              "J'ai utilisé ça pour faire %r{{itemName}}%w, prends-le!\x19",
                              "Ich habe das benutzt, um %r{{itemName}}%w zu machen, nimm es!\x19",
                              "これを使って%r{{itemName}}%wを作りました、どうぞ！\x19",
                              "Usé esto para hacer %r{{itemName}}%w, ¡tómalo!\x19");

        CustomMessage::Replace(&entry.msg, "{{itemName}}", Rando::StaticData::GetItemName(randoSaveCheck.randoItemId));

        // Mark the item as eligible for purchase
        randoSaveCheck.eligible = true;
        // Set flag that is normally set in the return experience that this skips
        SET_WEEKEVENTREG(WEEKEVENTREG_RECEIVED_FREE_BLUE_POTION);
        // Mark the item as out of stock
        EnTrt* enTrt =
            (EnTrt*)Actor_FindNearby(gPlayState, &GET_PLAYER(gPlayState)->actor, ACTOR_EN_TRT, ACTORCAT_NPC, 100.0f);
        if (enTrt != nullptr) {
            EnGirlA* enGirlA = enTrt->items[2];
            enGirlA->isOutOfStock = true;
            enGirlA->actor.draw = NULL;
        }

        CustomMessage::ReplaceSpecialChars(&entry.msg);
        CustomMessage::LoadCustomMessageIntoFont(entry);
        *loadFromMessageTable = false;
    });

    // Bomb Shop "We're expecting new stock" (hint)
    COND_ID_HOOK(OnOpenText, 0x648, IS_RANDO, [](u16* textId, bool* loadFromMessageTable) {
        auto randoSaveCheck = RANDO_SAVE_CHECKS[RC_BOMB_SHOP_ITEM_04_OR_CURIOSITY_SHOP_ITEM];
        auto entry = CustomMessage::LoadVanillaMessageTableEntry(*textId);

        entry.msg = LOCALIZED(
            "If nothing devastating happens to Mommy tonight, we should be able to sell %r{{itemName}}%w.\x19\xA8",
            "Si rien de dévastateur n'arrive à Maman ce soir, nous devrions pouvoir vendre %r{{itemName}}%w.\x19\xA8",
            "Wenn Mama heute Abend nichts Verheerendes passiert, sollten wir %r{{itemName}}%w verkaufen "
            "können.\x19\xA8",
            "今夜ママに何も破滅的なことが起こらなければ、%r{{itemName}}%wを売ることができるはずです。\x19\xA8",
            "Si no le pasa nada devastador a Mamá esta noche, deberíamos poder vender %r{{itemName}}%w.\x19\xA8");

        CustomMessage::Replace(&entry.msg, "{{itemName}}", Rando::StaticData::GetItemName(randoSaveCheck.randoItemId));
        CustomMessage::ReplaceSpecialChars(&entry.msg);
        CustomMessage::LoadCustomMessageIntoFont(entry);
        *loadFromMessageTable = false;
    });

    // Bomb Shop "We should have had..."
    COND_ID_HOOK(OnOpenText, 0x64A, IS_RANDO, [](u16* textId, bool* loadFromMessageTable) {
        auto randoSaveCheck = RANDO_SAVE_CHECKS[RC_BOMB_SHOP_ITEM_04_OR_CURIOSITY_SHOP_ITEM];
        auto randoStaticItem = Rando::StaticData::Items[randoSaveCheck.randoItemId];
        auto entry = CustomMessage::LoadVanillaMessageTableEntry(*textId);

        entry.msg = LOCALIZED(
            "Thanks to a mishap, we did not receive our %r{{itemName}}%w stock. Maybe next time...\x19\xA8",
            "À cause d'un accident, nous n'avons pas reçu notre stock de %r{{itemName}}%w. Peut-être la prochaine "
            "fois...\x19\xA8",
            "Aufgrund eines Missgeschicks haben wir unseren %r{{itemName}}%w Vorrat nicht erhalten. Vielleicht "
            "nächstes Mal...\x19\xA8",
            "事故のため、%r{{itemName}}%wの在庫を受け取れませんでした。また次回...\x19\xA8",
            "Debido a un percance, no recibimos nuestro stock de %r{{itemName}}%w. Tal vez la próxima vez...\x19\xA8");

        std::string itemName = LOCALIZED(randoStaticItem.nameEng, randoStaticItem.nameFre, randoStaticItem.nameGer,
                                         randoStaticItem.nameJpn, randoStaticItem.nameSpa);

        CustomMessage::Replace(&entry.msg, "{{itemName}}", itemName);

        CustomMessage::ReplaceSpecialChars(&entry.msg);
        CustomMessage::LoadCustomMessageIntoFont(entry);
        *loadFromMessageTable = false;
    });

    // Bomb Shop "I thought we could finally sell"
    COND_ID_HOOK(OnOpenText, 0x660, IS_RANDO, [](u16* textId, bool* loadFromMessageTable) {
        auto randoSaveCheck = RANDO_SAVE_CHECKS[RC_BOMB_SHOP_ITEM_04_OR_CURIOSITY_SHOP_ITEM];

        auto entry = CustomMessage::LoadVanillaMessageTableEntry(*textId);
        entry.msg = LOCALIZED("It's over... Now we'll never sell %r{{itemName}}%w...\x19\xA8",
                              "C'est fini... Maintenant nous ne vendrons plus jamais %r{{itemName}}%w...\x19\xA8",
                              "Es ist vorbei... Jetzt werden wir nie %r{{itemName}}%w verkaufen...\x19\xA8",
                              "終わりです...今度は%r{{itemName}}%wを売ることは絶対にありません...\x19\xA8",
                              "Se acabó... Ahora nunca venderemos %r{{itemName}}%w...\x19\xA8");

        CustomMessage::Replace(&entry.msg, "{{itemName}}", Rando::StaticData::GetItemName(randoSaveCheck.randoItemId));

        CustomMessage::ReplaceSpecialChars(&entry.msg);
        CustomMessage::LoadCustomMessageIntoFont(entry);
        *loadFromMessageTable = false;
    });

    // Bomb Shop "We just got a larger bomb bag in stock"
    COND_ID_HOOK(OnOpenText, 0x649, IS_RANDO, [](u16* textId, bool* loadFromMessageTable) {
        auto randoSaveCheck = RANDO_SAVE_CHECKS[RC_BOMB_SHOP_ITEM_04_OR_CURIOSITY_SHOP_ITEM];
        auto randoStaticItem = Rando::StaticData::Items[randoSaveCheck.randoItemId];

        auto entry = CustomMessage::LoadVanillaMessageTableEntry(*textId);
        entry.msg = LOCALIZED("We just got some new stock: %r{{itemName}}%w.\x19\xA8",
                              "Nous venons de recevoir de nouveaux stocks: %r{{itemName}}%w.\x19\xA8",
                              "Wir haben gerade neue Ware erhalten: %r{{itemName}}%w.\x19\xA8",
                              "新しい在庫が入りました: %r{{itemName}}%w。\x19\xA8",
                              "Acabamos de recibir nuevo stock: %r{{itemName}}%w.\x19\xA8");

        std::string itemName = LOCALIZED(randoStaticItem.nameEng, randoStaticItem.nameFre, randoStaticItem.nameGer,
                                         randoStaticItem.nameJpn, randoStaticItem.nameSpa);

        CustomMessage::Replace(&entry.msg, "{{itemName}}", itemName);

        CustomMessage::ReplaceSpecialChars(&entry.msg);
        CustomMessage::LoadCustomMessageIntoFont(entry);
        *loadFromMessageTable = false;
    });

    // Curiosity Shop "Tonight's special was stolen" (check that is shared with Bomb Shop)
    COND_ID_HOOK(OnOpenText, 0x29D3, IS_RANDO, renameStolenBombBag);
    COND_ID_HOOK(OnOpenText, 0x29D7, IS_RANDO, renameStolenBombBag);
    // Curiosity Shop "Tonight's bargain is" (special item check)
    COND_ID_HOOK(OnOpenText, 0x29D4, IS_RANDO, renameSpecialBargain);
    COND_ID_HOOK(OnOpenText, 0x29D8, IS_RANDO, renameSpecialBargain);

    // Magic Potion Shop Hag CANBUY_RESULT_CANNOT_GET_NOW (this text ID does not exist and just softlocks)
    COND_ID_HOOK(OnOpenText, 0x643, IS_RANDO, ReplaceCannotBuyMessage);
    // Goron Shop CANBUY_RESULT_CANNOT_GET_NOW
    COND_ID_HOOK(OnOpenText, 0xBD2, IS_RANDO, ReplaceCannotBuyMessage);
    // Bomb Shop CANBUY_RESULT_CANNOT_GET_NOW
    COND_ID_HOOK(OnOpenText, 0x645, IS_RANDO, ReplaceCannotBuyMessage);
    // Trading Post CANBUY_RESULT_CANNOT_GET_NOW
    COND_ID_HOOK(OnOpenText, 0x6BE, IS_RANDO, ReplaceCannotBuyMessage);
    COND_ID_HOOK(OnOpenText, 0x6DB, IS_RANDO, ReplaceCannotBuyMessage);
    // Zora Shop CANBUY_RESULT_CANNOT_GET_NOW (this text ID does not exist and just softlocks)
    COND_ID_HOOK(OnOpenText, 0x12E1, IS_RANDO, ReplaceCannotBuyMessage);
}
