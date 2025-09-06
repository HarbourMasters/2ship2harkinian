
#include "TimesplitsSettings.h"
#include "Timesplits.h"
#include <spdlog/fmt/fmt.h>
#include "public/bridge/consolevariablebridge.h"
#include "Context.h"
#include "Window.h"
#include "2s2h/BenGui/UIWidgets.hpp"

extern "C" {
#include "variables.h"
}

#include "interface/parameter_static/parameter_static.h"
#include "assets/archives/icon_item_24_static/icon_item_24_static_yar.h"
#include "assets/archives/icon_item_static/icon_item_static_yar.h"
#include "assets/interface/icon_item_dungeon_static/icon_item_dungeon_static.h"
#include "assets/interface/icon_item_field_static/icon_item_field_static.h"
#include "assets/objects/gameplay_keep/gameplay_keep.h"
#include "GameInteractor/GameInteractor.h"

std::map<uint32_t, ImVec4> songColorMap = {
    { ITEM_SONG_SONATA, ImVec4(0.588f, 1.0f, 0.392f, 1.0f) }, { ITEM_SONG_LULLABY, ImVec4(1.0f, 0.313f, 0.156f, 1.0f) },
    { ITEM_SONG_NOVA, ImVec4(0.392f, 0.588f, 1.0f, 1.0f) },   { ITEM_SONG_ELEGY, ImVec4(1.0f, 0.627f, 0.0f, 1.0f) },
    { ITEM_SONG_OATH, ImVec4(1.0f, 0.392f, 1.0f, 1.0f) },
};

std::vector<TimesplitObject> splitObjectList = {
    // clang-format off
    { ITEM_HEART_PIECE, "Piece of Heart", gQuestIconPieceOfHeartTex },

    // Inventory
    { ITEM_OCARINA_OF_TIME, 	"Ocarina of Time", 		gItemIconOcarinaOfTimeTex },
    { ITEM_BOW, 				"Bow", 					gItemIconBowTex },
    { ITEM_ARROW_FIRE, 			"Fire Arrow", 			gItemIconFireArrowTex },
    { ITEM_ARROW_ICE, 			"Ice Arrow", 			gItemIconIceArrowTex },
    { ITEM_ARROW_LIGHT, 		"Light Arrow", 			gItemIconLightArrowTex },
    { ITEM_MOONS_TEAR, 			"Moon's Tear", 			gItemIconMoonsTearTex },
    { ITEM_BOMB, 				"Bomb", 				gItemIconBombTex },
    { ITEM_BOMBCHU, 			"Bombchu", 				gItemIconBombchuTex },
    { ITEM_DEKU_STICK, 			"Deku Stick", 			gItemIconDekuStickTex },
    { ITEM_DEKU_NUT, 			"Deku Nut", 			gItemIconDekuNutTex },
    { ITEM_MAGIC_BEANS, 		"Magic Bean", 			gItemIconMagicBeansTex },
    { ITEM_ROOM_KEY, 			"Room Key", 			gItemIconRoomKeyTex },
    { ITEM_POWDER_KEG, 			"Powder Keg", 			gItemIconPowderKegTex },
    { ITEM_PICTOGRAPH_BOX, 		"Pictograph", 			gItemIconPictographBoxTex },
    { ITEM_LENS_OF_TRUTH, 		"Lens of Truth", 		gItemIconLensofTruthTex },
    { ITEM_HOOKSHOT, 			"Hookshot", 			gItemIconHookshotTex },
    { ITEM_SWORD_GREAT_FAIRY, 	"Great Fairy Sword", 	gItemIconGreatFairysSwordTex },
    { ITEM_LETTER_TO_KAFEI,     "Letter to Kafei", 	    gItemIconLetterToKafeiTex },
    { ITEM_BOTTLE,              "Empty Bottle", 	    gItemIconEmptyBottleTex },

    // Masks
    { ITEM_MASK_POSTMAN,        "Postman's Hat",         gItemIconPostmansHatTex },
    { ITEM_MASK_ALL_NIGHT,      "All-Night Mask",        gItemIconAllNightMaskTex },
    { ITEM_MASK_BLAST,          "Blast Mask",            gItemIconBlastMaskTex },
    { ITEM_MASK_STONE,          "Stone Mask",            gItemIconStoneMaskTex },
    { ITEM_MASK_GREAT_FAIRY,    "Great Fairy's Mask",    gItemIconGreatFairyMaskTex },
    { ITEM_MASK_DEKU,           "Deku Mask",             gItemIconDekuMaskTex },
    { ITEM_MASK_KEATON,         "Keaton Mask",           gItemIconKeatonMaskTex },
    { ITEM_MASK_BREMEN,         "Bremen Mask",           gItemIconBremenMaskTex },
    { ITEM_MASK_BUNNY,          "Bunny Hood",            gItemIconBunnyHoodTex },
    { ITEM_MASK_DON_GERO,       "Don Gero's Mask",       gItemIconDonGeroMaskTex },
    { ITEM_MASK_SCENTS,         "Mask of Scents",        gItemIconMaskOfScentsTex },
    { ITEM_MASK_GORON,          "Goron Mask",            gItemIconGoronMaskTex },
    { ITEM_MASK_ROMANI,         "Romani's Mask",         gItemIconRomaniMaskTex },
    { ITEM_MASK_CIRCUS_LEADER,  "Circus Leader's Mask",  gItemIconCircusLeaderMaskTex },
    { ITEM_MASK_KAFEIS_MASK,    "Kafei's Mask",          gItemIconKafeisMaskTex },
    { ITEM_MASK_COUPLE,         "Couple's Mask",         gItemIconCouplesMaskTex },
    { ITEM_MASK_TRUTH,          "Mask of Truth",         gItemIconMaskOfTruthTex },
    { ITEM_MASK_ZORA,           "Zora Mask",             gItemIconZoraMaskTex },
    { ITEM_MASK_KAMARO,         "Kamaro's Mask",         gItemIconKamaroMaskTex },
    { ITEM_MASK_GIBDO,          "Gibdo Mask",            gItemIconGibdoMaskTex },
    { ITEM_MASK_GARO,           "Garo's Mask",           gItemIconGaroMaskTex },
    { ITEM_MASK_CAPTAIN,        "Captain's Hat",         gItemIconCaptainsHatTex },
    { ITEM_MASK_GIANT,          "Giant's Mask",          gItemIconGiantsMaskTex },
    { ITEM_MASK_FIERCE_DEITY,   "Fierce Deity's Mask",   gItemIconFierceDeityMaskTex },

    // Songs
    { ITEM_SONG_TIME, 		    "Song of Time", 		gItemIconSongNoteTex },
    { ITEM_SONG_HEALING,	    "Song of Healing", 		gItemIconSongNoteTex },
    { ITEM_SONG_EPONA, 		    "Epona's Song",			gItemIconSongNoteTex },
    { ITEM_SONG_SOARING, 	    "Song of Soaring",		gItemIconSongNoteTex },
    { ITEM_SONG_STORMS, 	    "Song of Storms",		gItemIconSongNoteTex },
    { ITEM_SONG_SONATA, 	    "Sonata of Awakening", 	gItemIconSongNoteTex },
    { ITEM_SONG_LULLABY, 	    "Goron Lullaby", 		gItemIconSongNoteTex },
    { ITEM_SONG_NOVA, 		    "New Wave Bossa Nova", 	gItemIconSongNoteTex },
    { ITEM_SONG_ELEGY, 		    "Elegy of Emptiness", 	gItemIconSongNoteTex },
    { ITEM_SONG_OATH, 		    "Oath to Order",		gItemIconSongNoteTex },

    // Quest
    { ITEM_REMAINS_ODOLWA, 	 	"Odolwa's Remains", 	gItemIconOdolwasRemainsTex },
    { ITEM_REMAINS_GOHT, 	 	"Goht's Remains", 		gItemIconGohtsRemainsTex },
    { ITEM_REMAINS_GYORG, 	 	"Gyorg's Remains", 		gItemIconGyorgsRemainsTex },
    { ITEM_REMAINS_TWINMOLD, 	"Twinmold's Remains", 	gItemIconTwinmoldsRemainsTex },
    { ITEM_SWORD_KOKIRI, 	 	"Kokiri Sword",			gItemIconKokiriSwordTex },
    { ITEM_SHIELD_HERO, 	 	"Hero's Shield", 		gItemIconHerosShieldTex },
    { ITEM_WALLET_ADULT, 	 	"Adult Wallet", 		gItemIconAdultsWalletTex },
    { ITEM_BOMBERS_NOTEBOOK, 	"Bombers' Notebook", 	gItemIconBombersNotebookTex },

    // Dungeon


    // Upgrade Items
    { ITEM_QUIVER_30, 			"Quiver", 			    gItemIconQuiver30Tex},
    { ITEM_QUIVER_40, 			"Large Quiver", 		gItemIconQuiver40Tex},
    { ITEM_QUIVER_50, 			"Largest Quiver", 		gItemIconQuiver50Tex},
    { ITEM_BOMB_BAG_20, 		"Bomb Bag", 			gItemIconBombBag20Tex},
    { ITEM_BOMB_BAG_30, 		"Big Bomb Bag", 		gItemIconBombBag30Tex},
    { ITEM_BOMB_BAG_40, 		"Biggest Bomb Bag", 	gItemIconBombBag40Tex},
    { ITEM_SONG_LULLABY_INTRO, 	"Goron Lullaby Intro", 	gItemIconSongNoteTex },
    { ITEM_SWORD_RAZOR, 	    "Razor Sword", 	        gItemIconRazorSwordTex },
    { ITEM_SWORD_GILDED, 	    "Gilded Sword", 	    gItemIconGildedSwordTex },
    { ITEM_SHIELD_MIRROR, 	    "Mirror Shield", 	    gItemIconMirrorShieldTex },
    { ITEM_WALLET_GIANT, 	    "Giant Wallet", 	    gItemIconGiantsWalletTex },

    // Trade Items
    { ITEM_DEED_LAND, 			"Land Title Deed", 		gItemIconLandDeedTex },
    { ITEM_DEED_SWAMP, 			"Swamp Title Deed", 	gItemIconSwampDeedTex },
    { ITEM_DEED_MOUNTAIN, 		"Mountain Title Deed", 	gItemIconMountainDeedTex },
    { ITEM_DEED_OCEAN, 			"Ocean Title Deed", 	gItemIconOceanDeedTex },
    { ITEM_LETTER_MAMA, 		"Letter to Mama", 	    gItemIconLetterToMamaTex },
    { ITEM_PENDANT_OF_MEMORIES, "Pendant of Memories", 	gItemIconPendantOfMemoriesTex },

    // Bottled Items
    { ITEM_POTION_RED,      	"Red Potion",        		gItemIconRedPotionTex },
    { ITEM_POTION_GREEN,        "Green Potion",            	gItemIconGreenPotionTex },
    { ITEM_POTION_BLUE,         "Blue Potion",            	gItemIconBluePotionTex },
    { ITEM_FAIRY,    			"Bottled Fairy",    		gItemIconBottledFairyTex },
    { ITEM_DEKU_PRINCESS,		"Bottled Deku Princess",	gItemIconBottledDekuPrincessTex },
    { ITEM_MILK_BOTTLE,     	"Bottled Full Milk",        gItemIconBottledFullMilkTex },
    { ITEM_MILK_HALF,      		"Bottled Half Milk",        gItemIconBottledHalfMilkTex },
    { ITEM_FISH,          		"Bottled Fish",             gItemIconBottledFishTex },
    { ITEM_BUG,       			"Bottled Bug",       		gItemIconBottledBugTex },
    { ITEM_BLUE_FIRE,      		"Bottled Blue Fire",        gItemIconBottledBlueFireTex },
    { ITEM_POE,          		"Bottled Poe",            	gItemIconBottledPoeTex },
    { ITEM_BIG_POE,        		"Bottled Big Poe",         	gItemIconBottledBigPoeTex },
    { ITEM_SPRING_WATER,  		"Spring Water",  			gItemIconSpringWaterTex },
    { ITEM_HOT_SPRING_WATER,	"Hot Spring Water",         gItemIconHotSpringWaterTex },
    { ITEM_ZORA_EGG,       		"Bottled Zora Egg",         gItemIconBottledZoraEggTex },
    { ITEM_GOLD_DUST,      		"Bottled Gold Dust",        gItemIconBottledGoldDustTex },
    { ITEM_MUSHROOM,      		"Bottled Mushroom",         gItemIconBottledMushroomTex },
    { ITEM_SEAHORSE,      		"Bottled Seahorse",         gItemIconBottledSeahorseTex },
    { ITEM_CHATEAU,        		"Chateau Romani",           gItemIconChateauRomaniTex },

    // clang-format on
};

std::map<uint32_t, std::vector<uint32_t>> itemSubMenuList = {
    // clang-format off
    { ITEM_BOW,             { ITEM_QUIVER_30, ITEM_QUIVER_40, ITEM_QUIVER_50 } },
    { ITEM_BOMB,            { ITEM_BOMB_BAG_20, ITEM_BOMB_BAG_30, ITEM_BOMB_BAG_40 } },
    { ITEM_MOONS_TEAR,      { ITEM_MOONS_TEAR, ITEM_DEED_LAND, ITEM_DEED_SWAMP, ITEM_DEED_MOUNTAIN, ITEM_DEED_OCEAN } },
    { ITEM_ROOM_KEY,        { ITEM_ROOM_KEY, ITEM_LETTER_MAMA } },
    { ITEM_LETTER_TO_KAFEI, { ITEM_LETTER_TO_KAFEI, ITEM_PENDANT_OF_MEMORIES } },
    { ITEM_BOTTLE,          { ITEM_POTION_RED, ITEM_POTION_GREEN, ITEM_POTION_BLUE, ITEM_FAIRY, ITEM_DEKU_PRINCESS, 
                              ITEM_MILK_BOTTLE, ITEM_MILK_HALF, ITEM_FISH, ITEM_BUG, ITEM_BLUE_FIRE, ITEM_POE, 
                              ITEM_BIG_POE, ITEM_SPRING_WATER, ITEM_HOT_SPRING_WATER, ITEM_ZORA_EGG, ITEM_GOLD_DUST, 
                              ITEM_MUSHROOM, ITEM_SEAHORSE, ITEM_CHATEAU } },
    { ITEM_SONG_LULLABY,    { ITEM_SONG_LULLABY_INTRO, ITEM_SONG_LULLABY } },
    { ITEM_SWORD_KOKIRI,    { ITEM_SWORD_KOKIRI, ITEM_SWORD_RAZOR, ITEM_SWORD_GILDED } },
    { ITEM_SHIELD_HERO,     { ITEM_SHIELD_HERO, ITEM_SHIELD_MIRROR } },
    { ITEM_WALLET_ADULT,    { ITEM_WALLET_ADULT, ITEM_WALLET_GIANT } },
    // clang-format on
};

IndexRangeObject GetIndexRange(uint32_t start, uint32_t end) {
    IndexRangeObject setRange = { 0, 0 };

    for (size_t i = 0; i < splitObjectList.size(); i++) {
        if (splitObjectList[i].splitId == start) {
            setRange.startIndex = static_cast<int>(i);
        }
        if (splitObjectList[i].splitId == end) {
            setRange.endIndex = static_cast<int>(i);
        }
    }

    return setRange;
}

bool shouldPopUpOpen = false;
uint32_t popupItem = 0;
const char* popupTooltip = "";
IndexRangeObject range = GetIndexRange((uint32_t)ITEM_OCARINA_OF_TIME, (uint32_t)ITEM_BOTTLE);
const char* listName = "Inventory";
uint32_t listColumns = 6;

void DrawActionButtons() {
    if (ImGui::BeginTable("Action Buttons", 2)) {
        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);

        ImGui::TableNextColumn();
        if (UIWidgets::Button("New Attempt", {
                                                 .color = UIWidgets::Colors(CVarGetInteger("gSettings.Menu.Theme", 5)),
                                             })) {
            if (splitList.size() == 0) {
                return;
            }

            for (auto& splits : splitList) {
                splits.splitStatus = SPLIT_INACTIVE;
            }
            splitList[0].splitStatus = SPLIT_ACTIVE;
        }

        ImGui::TableNextColumn();
        if (UIWidgets::Button("Update Splits",
                              {
                                  .color = UIWidgets::Colors(CVarGetInteger("gSettings.Menu.Theme", 5)),
                              })) {
            UpdateSplitBests();
        }
        ImGui::EndTable();
    }
}

void DrawOptions() {
    if (ImGui::BeginTable("Action Buttons", 3)) {
        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);

        ImGui::TableNextColumn();
        UIWidgets::CVarCheckbox("Enable Time Splits", "gWindows.Timesplits",
                            {
                                .color = UIWidgets::Colors(CVarGetInteger("gSettings.Menu.Theme", 5)),
                            });

        ImGui::TableNextColumn();
        if (UIWidgets::CVarCheckbox("Show Headers", "gSettings.TimeSplits.ShowHeaders",
            {
                .color = UIWidgets::Colors(CVarGetInteger("gSettings.Menu.Theme", 5)),
            })) {
            UpdateSplitSettings(SPLIT_HEADERS);
        };

        ImGui::TableNextColumn();
        if (UIWidgets::CVarCheckbox("Hide Background", "gSettings.TimeSplits.Opacity",
                                    {
                                        .color = UIWidgets::Colors(CVarGetInteger("gSettings.Menu.Theme", 5)),
                                    })) {
            UpdateSplitSettings(SPLIT_OPACITY);
        };

        ImGui::EndTable();
    }
}

void DrawItemList(const char* tableName, IndexRangeObject range, uint32_t tableSize) {
    if (ImGui::BeginTable(tableName, tableSize)) {
        for (int i = range.startIndex; i <= range.endIndex; i++) {
            auto item = splitObjectList[i].splitId;

            ImGui::TableNextColumn();
            SplitsPushImageButtonStyle();
            if (ImGui::ImageButton(std::to_string(item).c_str(),
                                   Ship::Context::GetInstance()->GetWindow()->GetGui()->GetTextureByName(
                                       (const char*)gItemIcons[item]),
                ImVec2(32.0f, 32.0f), ImVec2(0, 0), ImVec2(1, 1), ImVec4(0, 0, 0, 0), GetColorTint(item))) {
                if (itemSubMenuList.contains(item)) {
                    shouldPopUpOpen = true;
                    popupItem = item;
                    ImGui::OpenPopup("ItemSubMenu");
                } else {
                    AddSplitEntry(item);
                }
            }
            UIWidgets::Tooltip(splitObjectList[i].splitName.c_str());

            SplitsPopImageButtonStyle();
        }
        HandlePopUpContext(popupItem);
        ImGui::EndTable();
    }
}

void TimesplitsSettingsWindow::DrawElement() {
    bool shouldRemoveEntry = false;
    uint32_t entryId = 0, entryIndex = 0;
        
    DrawOptions();
    DrawActionButtons();

    if (ImGui::BeginTable("Split Settings", 2)) {
        ImGui::TableSetupColumn("Preview", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHeaderLabel,
                                85.0f);
        ImGui::TableSetupColumn("Item Lists", ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_NoHeaderLabel);

        ImGui::TableNextColumn();
        ImGui::BeginDisabled();
        UIWidgets::Button("Preview", {
                                         .color = UIWidgets::Colors(CVarGetInteger("gSettings.Menu.Theme", 5)),
                                     });
        ImGui::EndDisabled();
        ImGui::BeginChild("Preview List");
        for (size_t i = 0; i < splitList.size(); i++) {
            auto& splits = splitList[i];

            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ((ImGui::GetContentRegionAvail().x - 32.0f) * 0.5f));

            SplitsPushImageButtonStyle();
            if (ImGui::ImageButton(
                    std::to_string(splits.splitId).c_str(),
                    Ship::Context::GetInstance()->GetWindow()->GetGui()->GetTextureByName(splits.splitImage),
                    ImVec2(32.0f, 32.0f), ImVec2(0, 0), ImVec2(1, 1), ImVec4(0, 0, 0, 0),
                    GetColorTint(splits.splitId))) {
                shouldRemoveEntry = true;
                entryId = splits.splitId;
                entryIndex = i;
            };

            HandleDragAndDrop(splitList, i);
            SplitsPopImageButtonStyle();
        }
        ImGui::EndChild();

        ImGui::TableNextColumn();
        if (ImGui::BeginTable("Item Lists", 4)) {
            ImGui::TableSetupColumn("",
                                    ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_NoHeaderLabel);
            ImGui::TableSetupColumn("",
                                    ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_NoHeaderLabel);
            ImGui::TableSetupColumn("",
                                    ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_NoHeaderLabel);
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_NoHeaderLabel);
            
            ImGui::TableNextColumn();
            if (UIWidgets::Button("Inventory",
                                  {
                                      .color = UIWidgets::Colors(CVarGetInteger("gSettings.Menu.Theme", 5)),
                                  })) {
                range = GetIndexRange((uint32_t)ITEM_HEART_PIECE, (uint32_t)ITEM_BOTTLE);
                listName = "Inventory";
                listColumns = 6;
            }
            ImGui::TableNextColumn();
            if (UIWidgets::Button("Masks", {
                                               .color = UIWidgets::Colors(CVarGetInteger("gSettings.Menu.Theme", 5)),
                                           })) {
                range = GetIndexRange((uint32_t)ITEM_MASK_POSTMAN, (uint32_t)ITEM_MASK_FIERCE_DEITY);
                listName = "Masks";
                listColumns = 6;
            }
            ImGui::TableNextColumn();
            if (UIWidgets::Button("Songs", {
                                               .color = UIWidgets::Colors(CVarGetInteger("gSettings.Menu.Theme", 5)),
                                           })) {
                range = GetIndexRange((uint32_t)ITEM_SONG_TIME, (uint32_t)ITEM_SONG_OATH);
                listName = "Songs";
                listColumns = 5;
            }
            ImGui::TableNextColumn();
            if (UIWidgets::Button("Quest", {
                                               .color = UIWidgets::Colors(CVarGetInteger("gSettings.Menu.Theme", 5)),
                                           })) {
                range = GetIndexRange((uint32_t)ITEM_REMAINS_ODOLWA, (uint32_t)ITEM_BOMBERS_NOTEBOOK);
                listName = "Quest";
                listColumns = 4;
            }
            ImGui::EndTable();
        }
        DrawItemList(listName, range, listColumns);

        ImGui::EndTable();
    }
    
    if (shouldRemoveEntry) {
        RemoveSplitEntry(entryId, entryIndex);
        shouldRemoveEntry = false;
    }
}

void TimesplitsSettingsWindow::InitElement() {

}
