/**
 * PauseItemDescriptions.cpp - C-Up item descriptions in the pause menu
 *
 * Ported from soh's PauseItemDescriptions.cpp (Skijer's NEI). MM builds an item description text id
 * as `0x1700 + itemId`, which only resolves for vanilla inventory items — NEI's custom items live at
 * 0xB6+, so their id lands outside the message table. Worse, Message_FindMessageNES leaves
 * `font->messageStart` untouched on a miss and func_801514B0 dereferences it anyway, so hovering a
 * custom item and pressing C-Up read a stale pointer instead of showing anything.
 *
 * This file supplies the missing text (built through 2ship's CustomMessage font path, no message
 * table entry needed) plus the existence check the kaleido pages gate on.
 */

#include "PauseItemDescriptions.h"
#include "CustomMessage.h"

extern "C" {
#include "z64item.h"
#include "macros.h"
#include "functions.h"
#include "variables.h"
#include "message_data_static.h"
#include "mods/extended_inventory.h" // Sw97_* / Wand_* (Skijer's NEI)
}

// ---------------------------------------------------------------------------
// Description table
// ---------------------------------------------------------------------------

struct ItemDescEntry {
    u16 itemId;
    const char* desc;
};

// Skijer's NEI custom items (z64item.h, 0xB6-0xCF). Text mirrors mods/items/CONTROLS.md.
// "\n" is a hard line break; CustomMessage word-wraps whatever is left over.
static const ItemDescEntry sCustomItemDescs[] = {
    { ITEM_ROCS_FEATHER_SKIJER, "Jump in ground and small jump\nfrom water." },
    { ITEM_ROCS_CAPE, "Jump from ground or water. Press\nagain in the air for a double jump." },
    { ITEM_DESIRE_SENSOR, "Sense major items in this area.\nCosts 3 hearts. Randomizer only." },
    { ITEM_HYLIAS_GRACE, "Fairy flight for 10s. Ignores walls.\nA=up, B=down, L=sprint. 24 MP." }, // RETIRED item; row kept for old saves
    // 2026-08-06 page-2 additions (behaviour pending — the description says so honestly).
    { EXT_ITEM_SHEIKAH_SLATE, "Ancient Sheikah tablet. A cycles\nits runes. Their powers are dormant." },
    { EXT_ITEM_PHANTOM_HOURGLASS, "Sand of hours from another sea.\nIts powers are still dormant." },
    { EXT_ITEM_SHADOW_CRYSTAL, "Cursed twilight crystal.\nIts powers are still dormant." },
    { EXT_ITEM_ROD_OF_SEASONS, "Rod bearing the four seasons.\nIts powers are still dormant." },
    { ITEM_ZONAI_PERMAFROST, "Stop time for 10s. Enemies, NPCs\nand bosses freeze. Costs 12 magic." },
    { ITEM_DEMISE_DESTRUCTION, "Massive AoE explosion. Damages all\nenemies in range. Ground only. 12 MP." },
    { ITEM_DEKU_LEAF, "Ground: blow wind gust. Air: hold\nto glide. Drains magic while gliding." },
    { ITEM_SWITCH_HOOK, "Aim and fire to swap positions\nwith objects and enemies." },
    { ITEM_MOGMA_MITTS, "Toggle to climb any wall.\nDrains magic over time." },
    { ITEM_GUST_JAR, "Pull enemies toward you, then push\nthem away. Hold C for element select." },
    { ITEM_BALL_AND_CHAIN, "Heavy thrown weapon. Breaks ice walls\nand heavy objects. Hold C to charge.\nC-Up to aim." },
    { ITEM_WHIP, "Grapple from any bar surface. Swing\nwith joystick. Release for momentum\nlaunch." },
    { ITEM_SPINNER, "Toggle to ride. A for homing dash\nattack. Breaks rocks." },
    { ITEM_CANE_OF_SOMARIA, "Create statues (max 3) that press\nany switch. Hookable and throwable." },
    { ITEM_DOMINION_ROD, "Fire orb to possess Beamos, Armos\nor Anubis. Control them with analog+C." },
    { ITEM_TIME_GATE, "Travel through time. Swap between\nyoung and adult Link. Costs 48 magic." },
    { ITEM_BOMB_ARROWS, "Explosive arrows. Hold C to aim.\nConsumes 1 arrow and 1 bomb per shot." },
    { ITEM_ROD_FIRE, "Slash=3 fireballs. Stab=long shot.\nJump=flamethrower. Spin=fire AoE.\nC-Up to aim." },
    { ITEM_ROD_ICE, "Slash=3 iceballs. Stab=long shot.\nJump=ice wave. Spin=ice AoE.\nC-Up to aim." },
    { ITEM_ROD_LIGHT, "Slash=3 orbs. Stab=long shot.\nJump=beam. Spin=light AoE.\nC-Up to aim." },
    { ITEM_BEETLE, "Launch remote beetle. Steer with\njoystick. B=boost. Grabs items and\nhits enemies." },
    { ITEM_SHOVEL, "Dig to uncover grottos, Gold\nSkulltulas and buried rewards." },
    { ITEM_MINISH_CAP, "Fast travel to pod soil spots.\nKill Gold Skulltulas to unlock them." },
    { ITEM_LANTERN, "Swing near fire to catch it. 4 types.\nBlue=melts red ice. Green=HP regen.\nPoe/Green=free Lens. Swing=fire dmg." },
    { ITEM_CHATEAU_ROMANI, "Drink for infinite magic.\nOne-time consumable." },
    { ITEM_POKEBALL, "Transform into Pikachu.\nPress again to revert." },

    // (The twelve SW97 arrow/bullet rows are gone — the elemental shot has no item id any more.
    // sSw97ElemDescs below is keyed by SW97_ELEM_* instead, and the magic costs those rows
    // advertised are gone with them: medallion shots are free now.)

    // Bottle-side custom items
    { ITEM_NET, "Catch bugs, fish and fairies.\nSwing it like a sword." },
    { ITEM_BOTTOMLESS_BOTTLE, "Holds an unlimited supply of\nwhatever you last poured into it." },
    { ITEM_MAGIC_MUSHROOM, "A strange mushroom.\nBottle it before it spoils." },
    { ITEM_BOTTLE_WITH_MAGIC_MUSHROOM, "A bottled magic mushroom." },
};

// ---------------------------------------------------------------------------
// Lookup
// ---------------------------------------------------------------------------

// Keyed by SW97_ELEM_*, NOT by item id — the elemental shot is a flag on the bow/slingshot now.
static const ItemDescEntry sSw97ElemDescs[] = {
    { SW97_ELEM_FIRE, "Fire elemental shot. Costs no magic." },
    { SW97_ELEM_ICE, "Ice elemental shot. Costs no magic." },
    { SW97_ELEM_LIGHT, "Light elemental shot. Costs no magic." },
    { SW97_ELEM_DARK, "Dark elemental shot. Costs no magic." },
    { SW97_ELEM_SOUL, "Soul elemental shot. Costs no magic." },
    { SW97_ELEM_WIND, "Wind elemental shot. Costs no magic." },
    // BOMB is the one element whose wording depends on the weapon, so it is handled separately in
    // PauseItemDesc_Get rather than living in this element-keyed table.
};

static const char* kBombArrowsDesc = "Explosive arrows. Hold C to aim.\nConsumes 1 arrow and 1 bomb per shot.";
static const char* kBombBulletsDesc = "Explosive bullets. Hold C to aim.\nConsumes 1 seed and 1 bomb per shot.";

// The six rods share one item id, so their descriptions key off the active mode.
static const ItemDescEntry sWandModeDescs[] = {
    { WAND_MODE_SAND, "Sand Rod. Unlocked by the Spirit\nMedallion." },
    { WAND_MODE_TORNADO, "Tornado Rod. Unlocked by the Forest\nMedallion." },
    { WAND_MODE_WATER, "Water Rod. Unlocked by the Water\nMedallion." },
    { WAND_MODE_METEOR, "Meteor Rod. Unlocked by the Fire\nMedallion." },
    { WAND_MODE_STORM, "Storm Rod. Unlocked by the Light\nMedallion." },
    { WAND_MODE_SCEPTER, "Shadow Scepter. Unlocked by the\nShadow Medallion." },
};

extern "C" const char* PauseItemDesc_Get(u16 itemId, s32 pageIndex) {
    // Custom items only live on the item page (all extended-inventory sub-pages route through it).
    if (pageIndex != PAUSE_ITEM) {
        return NULL;
    }

    // SW97 elemental shot: the cursor is on a plain bow/slingshot and the element rides a flag, so
    // describe whatever is primed on THAT weapon rather than looking the cursor item up.
    if (Sw97_IsBowItem(itemId) || Sw97_IsSlingItem(itemId)) {
        u8 isSling = Sw97_IsSlingItem(itemId);
        u8 elem = Sw97_EffectiveElement(isSling);
        if (elem == SW97_ELEM_BOMB) {
            return isSling ? kBombBulletsDesc : kBombArrowsDesc;
        }
        if (elem != SW97_ELEM_NONE) {
            for (size_t i = 0; i < ARRAY_COUNT(sSw97ElemDescs); i++) {
                if (sSw97ElemDescs[i].itemId == elem) {
                    return sSw97ElemDescs[i].desc;
                }
            }
        }
    }

    // Elemental Wand: one id, six descriptions — follow the active mode.
    if (itemId == ITEM_ELEMENTAL_WAND) {
        u8 mode = Wand_GetMode();
        for (size_t i = 0; i < ARRAY_COUNT(sWandModeDescs); i++) {
            if (sWandModeDescs[i].itemId == mode) {
                return sWandModeDescs[i].desc;
            }
        }
    }

    for (size_t i = 0; i < ARRAY_COUNT(sCustomItemDescs); i++) {
        if (sCustomItemDescs[i].itemId == itemId) {
            return sCustomItemDescs[i].desc;
        }
    }

    return NULL;
}

extern "C" u8 PauseItemDesc_VanillaTextExists(u16 textId) {
    if (gPlayState == NULL) {
        return false;
    }

    MessageTableEntry* entry = gPlayState->msgCtx.messageTableNES;
    if (entry == NULL) {
        return false;
    }

    while (entry->textId != 0xFFFF) {
        if (entry->textId == textId) {
            return true;
        }
        entry++;
    }

    return false;
}

// ---------------------------------------------------------------------------
// Display
// ---------------------------------------------------------------------------

/**
 * A description text id that is known to be in the table, used as the template for custom text.
 * func_801514B0 has no OnOpenText hook of its own (unlike Message_OpenText), so the only way to run
 * it with custom text is to let it set up all the pause-message state from a real entry and then
 * swap the font buffer underneath. Resolved once, since the item range differs per ROM revision.
 */
static u16 PauseItemDesc_GetTemplateTextId() {
    static u16 sTemplateTextId = 0;

    if (sTemplateTextId == 0) {
        for (u16 textId = 0x1700; textId < 0x1740; textId++) {
            if (PauseItemDesc_VanillaTextExists(textId)) {
                sTemplateTextId = textId;
                break;
            }
        }
    }

    return sTemplateTextId;
}

extern "C" void PauseItemDesc_Show(PlayState* play, const char* desc, u8 textBoxPos) {
    u16 templateTextId = PauseItemDesc_GetTemplateTextId();
    if (templateTextId == 0) {
        return;
    }

    func_801514B0(play, templateTextId, textBoxPos);

    MessageContext* msgCtx = &play->msgCtx;
    Font* font = &msgCtx->font;

    // Keep the template's header bytes: func_801514B0 already derived unk11F08/unk11F18/unk11F0C
    // from the first word of the buffer, so only the body may change.
    CustomMessage::Entry entry;
    entry.textboxType = font->msgBuf.schar[0];
    entry.textboxYPos = font->msgBuf.schar[1];
    entry.icon = font->msgBuf.schar[2];
    entry.nextMessageID = 0xFFFF;
    entry.firstItemCost = 0xFFFF;
    entry.secondItemCost = 0xFFFF;
    entry.msg = desc;

    CustomMessage::LoadCustomMessageIntoFont(entry);

    msgCtx->msgBufPos = 0;
    msgCtx->textDrawPos = 0;
    msgCtx->decodedTextLen = 0;
}
