#include "ActorViewer.h"
#include "2s2h/UIWidgets.hpp"

extern "C" {
    #include "global.h"
    extern PlayState* gPlayState;

}

typedef struct ActorInfo {
    u16 id;
    u16 params;
    Vec3f pos;
    Vec3s rot;
} ActorInfo;

typedef enum Method {
    LIST,
    TARGET,
    HOLD,
} Method;

std::array<const char*, 12> acMapping = { 
    "Switch", 
    "Background",
    "Player",
    "Explosive",
    "NPC",
    "Enemy",
    "Prop",
    "Item/Action",
    "Misc.",
    "Boss",
    "Door",
    "Chest"
};

// TODO: eventually move this out to a DB class like SoH did.
/*  
    (Empty) - Actor is defined but empty (nothing in init/update/destroy/draw actor functions)
    (Unknown) - Identity or purpose unknown, but defined.
    (OoT) - Left intact from Ocarina of Time and unused.
    (Broken) - At your own risk.
*/

std::unordered_map<s16, std::string> actorDescriptions = {
    { ACTOR_PLAYER, "Player" },
    { ACTOR_EN_TEST, "Crater Marks" },
    { ACTOR_EN_GIRLA, "Shop Items" },
    { ACTOR_EN_PART, "Enemy body parts" },
    { ACTOR_EN_LIGHT, "Deku Shrine Flames" },
    { ACTOR_EN_DOOR, "Wooden Door" },
    { ACTOR_EN_BOX, "Chest" },
    { ACTOR_EN_PAMETFROG, "Gekko (Miniboss)" },
    { ACTOR_EN_OKUTA, "Octorok" },
    { ACTOR_EN_BOM, "Bomb / Powder Keg" },
    { ACTOR_EN_WALLMAS, "Wallmaster" },
    { ACTOR_EN_DODONGO, "Dodongo" },
    { ACTOR_EN_FIREFLY, "Keese" },
    { ACTOR_EN_HORSE, "Epona" },
    { ACTOR_EN_ITEM00, "Collectibles" },
    { ACTOR_EN_ARROW, "Arrow / Deku Nut" },
    { ACTOR_EN_ELF, "Fairy" },
    { ACTOR_EN_NIW, "Cucco" },
    { ACTOR_EN_TITE, "Tektite" },
    { ACTOR_EN_PEEHAT, "Peehat" },
    { ACTOR_EN_BUTTE, "Butterfly" },
    { ACTOR_EN_INSECT, "Non-burrowing bug"},
    { ACTOR_EN_FISH, "Fish" },
    { ACTOR_EN_HOLL, "Loading Hall/Hole" },
    { ACTOR_EN_DINOFOS, "Dinolfos" },
    { ACTOR_EN_HATA, "Red Flag on Post" },
    { ACTOR_EN_ZL1, "(Empty)" },
    { ACTOR_EN_VIEWER, "Cutscene Actor(?)" },
    { ACTOR_EN_BUBBLE, "Shabom (OoT)" },
    { ACTOR_DOOR_SHUTTER, "Studded Lifting Door/Ikana Castle Rolling Door" },
    { ACTOR_EN_BOOM, "Zora Boomerang" },
    { ACTOR_EN_TORCH2, "Elegy of Emptiness Shell"},
    { ACTOR_EN_MINIFROG, "Frog Choir Frog" },
    { ACTOR_EN_ST, "Large Skulltula" },
    { ACTOR_EN_A_OBJ, "gameplay_keep item(?)" },
    { ACTOR_OBJ_WTURN, "Stone Tower Temple Inverter" },
    { ACTOR_EN_RIVER_SOUND, "Environmental noises"},
    { ACTOR_EN_OSSAN, "Trading Post Shop" },
    { ACTOR_EN_FAMOS, "Death Armos (Inv. Stone Tower)" },
    { ACTOR_EN_BOMBF, "Bomb Flower" },
    { ACTOR_EN_AM, "Armos" },
    { ACTOR_EN_DEKUBABA, "Deku Baba" },
    { ACTOR_EN_M_FIRE1, "Deku Nut Effect" },
    { ACTOR_EN_M_THUNDER, "Spin Attack/Sword Beam" },
    { ACTOR_BG_BREAKWALL, "Great Bay Temple Weather(?)" },
    { ACTOR_DOOR_WARP1, "Blue Warp portal/crystal / Majora's Mask boss warp platform" },
    { ACTOR_OBJ_SYOKUDAI, "Torch" },
    { ACTOR_ITEM_B_HEART, "Heart Container" },
    { ACTOR_EN_DEKUNUTS, "Mad Scrub" },
    { ACTOR_EN_BBFALL, "Red Bubble" },
    { ACTOR_ARMS_HOOK, "Hookshot Tip" },
    { ACTOR_EN_BB, "Blue Bubble" },
    { ACTOR_BG_KEIKOKU_SPR, "Termina Field Fountain Water" },
    { ACTOR_EN_WOOD02, "Tree/Shrub" },
    { ACTOR_EN_DEATH, "Gomess"} ,
    { ACTOR_EN_MINIDEATH, "Gomess's Bat" },
    { ACTOR_EN_VM, "Beamos" },
    { ACTOR_DEMO_EFFECT, "Cutscene Effect" },
    { ACTOR_DEMO_KANKYO, "BG Effect (Lost Woods/Giant's Chamber/Moon)" },
    { ACTOR_EN_FLOORMAS, "Floormaster" },
    { ACTOR_EN_RD, "Redead/Gibdo (can't talk to player)" },
    { ACTOR_BG_F40_FLIFT, "Grey Square Stone Elevator (Stone Tower Temple)" },
    { ACTOR_OBJ_MURE, "Bug/Insect/Butterfly spawner" },
    { ACTOR_EN_SW, "Skullwalltula" },
    { ACTOR_OBJECT_KANKYO, "Snow/Rain (SK backstory)/Bubble (Pinnacle Rock)" },
    { ACTOR_EN_HORSE_LINK_CHILD, "Child Epona (OoT) (Broken)" },
    { ACTOR_DOOR_ANA, "Grotto Hold Entrance" },
    { ACTOR_EN_ENCOUNT1, "Spawner (Dragonfly/Skullfish/Wallmaster)" },
    { ACTOR_DEMO_TRE_LGT, "Light from Treasure Chest" },
    { ACTOR_EN_ENCOUNT2, "Majora's Mask Balloon (Astral Observatory)" },
    { ACTOR_EN_FIRE_ROCK, "(Empty)" },
    { ACTOR_BG_CTOWER_ROT, "Twisting Path w/Stone Doors to Clock Tower" },
    { ACTOR_MIR_RAY, "Reflectable light ray (OoT) (Broken)" },
    { ACTOR_EN_SB, "Shellblade" },
    { ACTOR_EN_BIGSLIME, "Fused Jellies & Gekko" },
    { ACTOR_EN_KAREBABA, "Wilted Dekubaba/Mini Baba" },
    { ACTOR_EN_IN, "Gorman Brother" },
    { ACTOR_EN_RU, "Adult Ruto (OoT)" },
    { ACTOR_EN_BOM_CHU, "Bombchu" },
    { ACTOR_EN_HORSE_GAME_CHECK, "Gorman Race Track Dirt Patch" },
    { ACTOR_EN_RR, "Like Like" },
    { ACTOR_EN_FR, "(Unknown) - EnFr" },
    { ACTOR_EN_FISHING, "Fishing Pond Elements" },
    { ACTOR_OBJ_OSHIHIKI, "Pushable Block" },
    { ACTOR_EFF_DUST, "Dust Effect" },
    { ACTOR_BG_UMAJUMP, "Horse Jumping Fence" },
    { ACTOR_ARROW_FIRE, "Fire Arrow" },
    { ACTOR_ARROW_ICE, "Ice Arrow" },
    { ACTOR_ARROW_LIGHT, "Light Arrow" },
    { ACTOR_ITEM_ETCETERA, "Leftover Collectible Items (OoT)" },
    { ACTOR_OBJ_KIBAKO, "Small grabbable crate" },
    { ACTOR_OBJ_TSUBO, "Pot" },
    { ACTOR_EN_IK, "Iron Knuckle" },
    { ACTOR_DEMO_SHD, "(Unknown) - DemoShd" },
    { ACTOR_EN_DNS, "King's Chamber Deku Guard (Deku Palace)" },
    { ACTOR_ELF_MSG, "Tatl Hint (proximity C-Up?)" },
    { ACTOR_EN_HONOTRAP, "Fire-shooting Eye Switch" },
    { ACTOR_EN_TUBO_TRAP, "Flying Pot Trap" },
    { ACTOR_OBJ_ICE_POLY, "Large Ice Block (meltable)" },
    { ACTOR_EN_FZ, "Freezard" },
    { ACTOR_EN_KUSA, "Grass" },
    { ACTOR_OBJ_BEAN, "Floating Bean Plant/Soft Soil" },
    { ACTOR_OBJ_BOMBIWA, "Bombable Boulder" },
    { ACTOR_OBJ_SWITCH, "Floor/Eye Switch" },
    { ACTOR_OBJ_LIFT, "Brown Elevator (Dampe's Grave)" },
    { ACTOR_OBJ_HSBLOCK, "Hookshot Block" },
    { ACTOR_EN_OKARINA_TAG, "Ocarina Music Staff Spot" },
    { ACTOR_EN_GOROIWA, "Rolling Boulder" },
    { ACTOR_EN_DAIKU, "Carpenter" },
    { ACTOR_EN_NWC, "Cucco chick" },
    { ACTOR_ITEM_INBOX, "In-chest Item Draw (unused) "},
    { ACTOR_EN_GE1, "White-clad Gerudo Pirate" },
    { ACTOR_OBJ_BLOCKSTOP, "Push Block trigger (Snowhead) "},
    { ACTOR_EN_SDA, "Dynamic Player Shadow" },
    { ACTOR_EN_CLEAR_TAG, "Various Effects" },
    { ACTOR_EN_GM, "Gorman" },
    { ACTOR_EN_MS, "Bean Seller" },
    { ACTOR_EN_HS, "Grog" },
    { ACTOR_BG_INGATE, "Swamp Tour Boat" },
    { ACTOR_EN_KANBAN, "Square Signpost" },
    { ACTOR_EN_ATTACK_NIW, "Attacking Cucco" },
    { ACTOR_EN_MK, "Marine Researcher" },
    { ACTOR_EN_OWL, "Kaepora Gaebora" },
    { ACTOR_EN_ISHI, "Liftable Rocks/Silver Boulders" },
    { ACTOR_OBJ_HANA, "Orange Graveyard Flower" },
    { ACTOR_OBJ_MURE2, "Rock Circle Spawner" },
    { ACTOR_EN_FU, "Honey & Darling" },
    { ACTOR_EN_STREAM, "Water Vortex (OoT)" },
    { ACTOR_EN_MM, "Rock Sirloin" },
    { ACTOR_EN_WEATHER_TAG, "Local Weather Changes" },
    { ACTOR_EN_MAG, "Title Logo" },
    { ACTOR_ELF_MSG2, "Tatl Hint (Z-Target C-Up?)" },
    { ACTOR_BG_F40_SWLIFT, "Stone Tower vertically oscillating platform (unused)" },
    { ACTOR_EN_KAKASI, "Pierre the Scarecrow" },
    { ACTOR_OBJ_MAKEOSHIHIKI, "Pushable Block Switch Flag Handler" },
    { ACTOR_OCEFF_SPOT, "Sun's Song Ocarina Effect" },
    { ACTOR_EN_TORCH, "Grotto chest spawner" },
    { ACTOR_SHOT_SUN, "Sun hitbox (OoT)/Fairy Spawner(?)" },
    { ACTOR_OBJ_ROOMTIMER, "Room Timer" },
    { ACTOR_EN_SSH, "Cursed Man (Swamp Spider House)" },
    { ACTOR_OCEFF_WIPE, "Song of Time Ocarina Effect" },
    { ACTOR_OCEFF_STORM, "Song of Storms Ocarina Effect" },
    { ACTOR_OBJ_DEMO, "Proximity-based cutscene trigger" },
    { ACTOR_EN_MINISLIME, "Mini Jelly Droplet" },
    { ACTOR_EN_NUTSBALL, "Deku Nut projectile" },
    { ACTOR_OCEFF_WIPE2, "Epona's Song Ocarina Effect" },
    { ACTOR_OCEFF_WIPE3, "Saria's Song Ocarina Effect (OoT)" },
    { ACTOR_EN_DG, "Dog" },
    { ACTOR_EN_SI, "Gold Skulltula Token" },
    { ACTOR_OBJ_COMB, "Beehive" },
    { ACTOR_OBJ_KIBAKO2, "Large Wooden Crate" },
    { ACTOR_EN_HS2, "Targetable Nothing" },
    { ACTOR_OBJ_MURE3, "Group Rupee spawner" },
    { ACTOR_EN_TG, "Target Game (Honey & Darling)" },
    { ACTOR_EN_WF, "Wolfos/White Wolfos" },
    { ACTOR_EN_SKB, "Stalchild" },
    { ACTOR_EN_GS, "Gossip Stone" },
    { ACTOR_OBJ_SOUND, "Invisible Sound Emitter" },
    { ACTOR_EN_CROW, "Guay", },
    { ACTOR_EN_COW, "Cow" },
    { ACTOR_OCEFF_WIPE4, "Scarecrow's Song Ocarina Effect" },
    { ACTOR_EN_ZO, "Zora (Unused)" },
    { ACTOR_OBJ_MAKEKINSUTA, "Soft Soil w/Skulltula (Swamp Spider House)" },
    { ACTOR_EN_GE3, "Aviel (Gerudo Pirate Leader)" },
    { ACTOR_OBJ_HAMISHI, "Bronze Boulder" },
    { ACTOR_EN_ZL4, "Glitched Skull Kid T-pose" },
    { ACTOR_EN_MM2, "Postman's Letter to Himself" },
    { ACTOR_DOOR_SPIRAL, "Staircase" },
    { ACTOR_OBJ_PZLBLOCK, "Puzzle Block" },
    { ACTOR_OBJ_TOGE, "Blade Trap" },
    { ACTOR_OBJ_ARMOS, "Non-Hostile Armos" },
    { ACTOR_OBJ_BOYO, "(Unknown)" },
    { ACTOR_EN_GRASSHOPPER, "Dragonfly" },
    { ACTOR_OBJ_GRASS, "Optimized Manager for ObjGrassUnit grasses" },
    { ACTOR_OBJ_GRASS_CARRY, "Carried grass from ObjGrassUnit" },
    { ACTOR_OBJ_GRASS_UNIT, "Grass pattern initializer" },
    { ACTOR_BG_FIRE_WALL, "Wall of Fire from BgSpoutFire" },
    { ACTOR_EN_BU, "Dummied out Enemy" },
    { ACTOR_EN_ENCOUNT3, "Garo Spawner" },
    { ACTOR_EN_JSO, "Garo" },
    { ACTOR_OBJ_CHIKUWA, "Falling row of blocks (unused) "},
    { ACTOR_EN_KNIGHT, "Igos du Ikana/IdI Lackey" },
    { ACTOR_EN_WARP_TAG, "Warp to Moon Trial Entrance" },
    { ACTOR_EN_AOB_01, "Mamamu Yan" },
    { ACTOR_EN_BOJ_01, "(Empty)" },
    { ACTOR_EN_BOJ_02, "(Empty)" },
    { ACTOR_EN_BOJ_03, "(Empty)" },
    { ACTOR_EN_ENCOUNT4, "Stalchild/Fire Wall spawner (Keeta Chase)" },
    { ACTOR_EN_BOM_BOWL_MAN, "Bomber Line" },
    { ACTOR_EN_SYATEKI_MAN, "Shooting Gallery Guy" },
    { ACTOR_BG_ICICLE, "Icicle" },
    { ACTOR_EN_SYATEKI_CROW, "Shooting Gallery Guay" },
    { ACTOR_EN_BOJ_04, "(Empty)" },
    { ACTOR_EN_CNE_01, "Market NPC (Unused)" },
    { ACTOR_EN_BBA_01, "Bomb Shop Lady NPC (Unused)" },
    { ACTOR_EN_BJI_01, "Professor Shikashi (Astral Observatory)" },
    { ACTOR_BG_SPDWEB, "Spider web" },
    { ACTOR_EN_MT_TAG, "Goron Race Controls" },
    { ACTOR_BOSS_01, "Odolwa/Odolwa Bug/Odolwa Afterimage" },
    { ACTOR_BOSS_02, "Twinmold" },
    { ACTOR_BOSS_03, "Gyorg" },
    { ACTOR_BOSS_04, "Wart" },
    { ACTOR_BOSS_05, "Bio Deku Baba" },
    { ACTOR_BOSS_06, "Igos du Ikana window" },
    { ACTOR_BOSS_07, "Majora" },
    { ACTOR_BG_DY_YOSEIZO, "Great Fairy" },
    { ACTOR_EN_BOJ_05, "(Empty)"},
    { ACTOR_EN_SOB1, "Shop (Zora/Goron/Bomb) "},
    { ACTOR_EN_GO, "Goron" },
    { ACTOR_EN_RAF, "Carnivorous Lily Pad" },
    { ACTOR_OBJ_FUNEN, "Stone Tower Smoke" },
    { ACTOR_OBJ_RAILLIFT, "Moving Deku Flower Platform" },
    { ACTOR_BG_NUMA_HANA, "Big Wooden Flower (Woodfall Temple)" },
    { ACTOR_OBJ_FLOWERPOT, "Breakable Pot with Grass" },
    { ACTOR_OBJ_SPINYROLL, "Horizontal Spike-Covered Log" },
    { ACTOR_DM_HINA, "Boss Mask cutscene object" },
    { ACTOR_EN_SYATEKI_WF, "Shooting Gallery Wolfos" },
    { ACTOR_OBJ_SKATEBLOCK, "Ice-Sliding Pushable Block" },
    { ACTOR_OBJ_ICEBLOCK, "Ice Block Surrounding Frozen Enemy" },
    { ACTOR_EN_BIGPAMET, "Snapper" },
    { ACTOR_EN_SYATEKI_DEKUNUTS, "Shooting Gallery Scrub" },
    { ACTOR_ELF_MSG3, "Tatl Message (Proximity?)" },
    { ACTOR_EN_FG, "Enemy Frog (beta)" },
    { ACTOR_DM_RAVINE, "Tree Trunk (Lost Woods cutscene)" },
    { ACTOR_DM_SA, "Glitched Skull Kid T-Pose (cutscene)" },
    { ACTOR_EN_SLIME, "Chuchu" },
    { ACTOR_EN_PR, "Desbreko" },
    { ACTOR_OBJ_TOUDAI, "Clock Tower spotlight (unused)" },
    { ACTOR_OBJ_ENTOTU, "Clock Town smoking chimney" },
    { ACTOR_OBJ_BELL, "Stock Pot Inn bell" },
    { ACTOR_EN_SYATEKI_OKUTA, "Shooting Gallery Octorok" },
    { ACTOR_OBJ_SHUTTER, "West Clock Town bank closing shutter" },
    { ACTOR_DM_ZL, "Child Zelda" },
    { ACTOR_EN_ELFGRP, "Stray Fairy group manager" },
    { ACTOR_DM_TSG, "Mask effect handler (when Link falls in intro)" },
    { ACTOR_EN_BAGUO, "Nejiron" },
    { ACTOR_OBJ_VSPINYROLL, "Vertical spike rollers" },
    { ACTOR_OBJ_SMORK, "Romani Ranch Chimney Smoke" },
    { ACTOR_EN_TEST2, "Lens of Truth-affected object" },
    { ACTOR_EN_TEST3, "Kafei" },
    { ACTOR_EN_TEST4, "Three-day events" },
    { ACTOR_EN_BAT, "Bad Bat" },
    { ACTOR_EN_SEKIHI, "Mikau's Grave/Song Pedestal" },
    { ACTOR_EN_WIZ, "Wizrobe" },
    { ACTOR_EN_WIZ_BROCK, "Wizrobe Warp Platform" },
    { ACTOR_EN_WIZ_FIRE, "Wizrobe Fire/Ice Attack" },
    { ACTOR_EFF_CHANGE, "EoE Beam of Light" },
    { ACTOR_DM_STATUE, "Pillar of Water (Giant's Chamber)" },
    { ACTOR_OBJ_FIRESHIELD, "Ring of Fire" },
    { ACTOR_BG_LADDER, "Wooden Ladder" },
    { ACTOR_EN_MKK, "Black/White Boe" },
    { ACTOR_DEMO_GETITEM, "Cutscene Object for Great Fairy Mask/Sword" },
    { ACTOR_EN_DNB, "Exploding Snow Mountain? (unused)" },
    { ACTOR_EN_DNH, "Koume (Boat House)" },
    { ACTOR_EN_DNK, "Hallucinatory Mad Scrub" },
    { ACTOR_EN_DNQ, "Deku King" },
    { ACTOR_BG_KEIKOKU_SAKU, "Spiked Fence (Termina Field)" },
    { ACTOR_OBJ_HUGEBOMBIWA, "Milk Road/Goron Racetrack Boulder" },
    { ACTOR_EN_FIREFLY2, "(Empty)" },
    { ACTOR_EN_RAT, "Real Bombchu" },
    { ACTOR_EN_WATER_EFFECT, "Water/Rock Drop Spawner/Gyorg splashing effect" },
    { ACTOR_EN_KUSA2, "Keaton Grass" },
    { ACTOR_BG_SPOUT_FIRE, "Proximity-activated Fire Wall Spawner" },
    { ACTOR_BG_DBLUE_MOVEBG, "Great Bay Moving parts" },
    { ACTOR_EN_DY_EXTRA, "Great Fairy Beam" },
    { ACTOR_EN_BAL, "Tingle (w/Balloon)" },
    { ACTOR_EN_GINKO_MAN, "Bank Teller" },
    { ACTOR_EN_WARP_UZU, "Pirates' Fortress Telescope" },
    { ACTOR_OBJ_DRIFTICE, "Floating Ice Platform (Mountain Village)" },
    { ACTOR_EN_LOOK_NUTS, "Patrolling Deku Guard" },
    { ACTOR_EN_MUSHI2, "Bugs from Bottle" },
    { ACTOR_EN_FALL, "Moon/Moon effect/Moon Tear" },
    { ACTOR_EN_MM3, "Counting Game Postman" },
    { ACTOR_BG_CRACE_MOVEBG, "Sliding Doors (Deku Shrine)" },
    { ACTOR_EN_DNO, "Deku Butler" },
    { ACTOR_EN_PR2, "Skullfish" },
    { ACTOR_EN_PRZ, "Defeated Skullfish" },
    { ACTOR_EN_JSO2, "Garo Master" },
    { ACTOR_OBJ_ETCETERA, "Deku Flower" },
    { ACTOR_EN_EGOL, "Eyegore" },
    { ACTOR_OBJ_MINE, "Spike metal mine" },
    { ACTOR_OBJ_PURIFY, "Poisoned/Purified Water Element" },
    { ACTOR_EN_TRU, "Koume (Woods of Mystery)" },
    { ACTOR_EN_TRT, "Kotake" },
    { ACTOR_EN_TEST5, "Spring Water modifier" },
    { ACTOR_EN_TEST6, "Song of Time Effect" },
    { ACTOR_EN_AZ, "Beaver Bro" },
    { ACTOR_EN_ESTONE, "Rubble (Eyegore)" },
    { ACTOR_BG_HAKUGIN_POST, "Central Pillar (Snowhead Temple)" },
    { ACTOR_DM_OPSTAGE, "Lost Woods Cutscene Trees/Floor" },
    { ACTOR_DM_STK, "Skull Kid (Cutscene)/Majora's Mask (Cutscene)" },
    { ACTOR_DM_CHAR00, "Tatl/Tael (Cutscene)" },
    { ACTOR_DM_CHAR01, "Woodfall scene object" },
    { ACTOR_DM_CHAR02, "Ocarina of Time (Clock Tower rooftop cutscenes)" },
    { ACTOR_DM_CHAR03, "Deku Mask (Cutscene)" },
    { ACTOR_DM_CHAR04, "Tatl/Tael (unused)" },
    { ACTOR_DM_CHAR05, "Cutscene Mask object" },
    { ACTOR_DM_CHAR06, "Mountain Village Snowy landscape fadeout" },
    { ACTOR_DM_CHAR07, "Milk Bar Object" },
    { ACTOR_DM_CHAR08, "Large Great Bay Turtle" },
    { ACTOR_DM_CHAR09, "Pirates' Fortress CS character" },
    { ACTOR_OBJ_TOKEIDAI, "Clock Tower Component" },
    { ACTOR_EN_MNK, "Monkey" },
    { ACTOR_EN_EGBLOCK, "Pillar (weak to Eyegore, unused)" },
    { ACTOR_EN_GUARD_NUTS, "Deku Palace Entrace guard" },
    { ACTOR_BG_HAKUGIN_BOMBWALL, "Bombable Wall (Snowhead Temple)" },
    { ACTOR_OBJ_TOKEI_TOBIRA, "Clock Tower Swinging Doors" },
    { ACTOR_BG_HAKUGIN_ELVPOLE, "Raisable pillar (Snowhead Temple)" },
    { ACTOR_EN_MA4, "Romani" },
    { ACTOR_EN_TWIG, "Beaver Race Ring" },
    { ACTOR_EN_PO_FUSEN, "Poe Balloon (Romani Ranch)" },
    { ACTOR_EN_DOOR_ETC, "Wooden Door (copy)" },
    { ACTOR_EN_BIGOKUTA, "Big Octo" },
    { ACTOR_BG_ICEFLOE, "Ice Platform from Ice Arrow" },
    { ACTOR_OBJ_OCARINALIFT, "Triforce Elevator?" },
    { ACTOR_EN_TIME_TAG, "Event Trigger" },
    { ACTOR_BG_OPEN_SHUTTER, "Sliding doors" },
    { ACTOR_BG_OPEN_SPOT, "Spotlight (Human -> Deku cutscene)" },
    { ACTOR_BG_FU_KAITEN, "Rotating Platform (Honey & Darling)" },
    { ACTOR_OBJ_AQUA, "Bottle Water" },
    { ACTOR_EN_ELFORG, "Stray Fairy" },
    { ACTOR_EN_ELFBUB, "Stray Fairy (bubble)" },
    { ACTOR_EN_FU_MATO, "Target (Honey & Darling)" },
    { ACTOR_EN_FU_KAGO, "Bomb Basket (Honey & Darling)" },
    { ACTOR_EN_OSN, "Happy Mask Salesman" },
    { ACTOR_BG_CTOWER_GEAR, "Inside Clock Tower Cog/Organ" },
    { ACTOR_EN_TRT2, "Kotake (Southern Swamp/Woods of Mystery)" },
    { ACTOR_OBJ_TOKEI_STEP, "Door (to top of Clock Tower)" },
    { ACTOR_BG_LOTUS, "Lily Pad" },
    { ACTOR_EN_KAME, "Snapper" },
    { ACTOR_OBJ_TAKARAYA_WALL, "Treasure Chest Shop board manager" },
    { ACTOR_BG_FU_MIZU, "Water (Honey & Darling)" },
    { ACTOR_EN_SELLNUTS, "Business Scrub (carrying bags)" },
    { ACTOR_BG_DKJAIL_IVY, "Cuttable Ivy" },
    { ACTOR_OBJ_VISIBLOCK, "Lens of Truth Platform" },
    { ACTOR_EN_TAKARAYA, "Treasure Chest Shop Girl" },
    { ACTOR_EN_TSN, "Great Bay Fisherman" },
    { ACTOR_EN_DS2N, "Potion Shop Owner (OoT)" },
    { ACTOR_EN_FSN, "Curiosity Shop Man" },
    { ACTOR_EN_SHN, "Swamp Tourist Center Guide" },
    { ACTOR_EN_STOP_HEISHI, "Gate-Blocking Soldier" },
    { ACTOR_OBJ_BIGICICLE, "Large Icicle" },
    { ACTOR_EN_LIFT_NUTS, "Deku Scrub Playground Employee" },
    { ACTOR_EN_TK, "Dampe" },
    { ACTOR_BG_MARKET_STEP, "Scenery (West Clocktown)" },
    { ACTOR_OBJ_LUPYGAMELIFT, "Rupee Elevator (Deku Scrub Elevator)" },
    { ACTOR_EN_TEST7, "Song of Soaring effect" },
    { ACTOR_OBJ_LIGHTBLOCK, "Sun Block" },
    { ACTOR_MIR_RAY2, "Reflectable light ray" },
    { ACTOR_EN_WDHAND, "Dexihand" },
    { ACTOR_EN_GAMELUPY, "Deku Scrub Playground Rupee" },
    { ACTOR_BG_DANPEI_MOVEBG, "Floating Block (Deku Shrine/Snowhead Temple)" },
    { ACTOR_EN_SNOWWD, "Snow-Covered Tree" },
    { ACTOR_EN_PM, "Postman" },
    { ACTOR_EN_GAKUFU, "2D Song Button (Termina Field)" },
    { ACTOR_ELF_MSG4, "Tatl Hint (Proximity C-Up Copy?)" },
    { ACTOR_ELF_MSG5, "Tatl Message (Proximity copy?)" },
    { ACTOR_EN_COL_MAN, "Lab Heart Piece/Garo Master Falling rock/Garo Master Bomb" },
    { ACTOR_EN_TALK_GIBUD, "Talking Gibdo" },
    { ACTOR_EN_GIANT, "Giant" },
    { ACTOR_OBJ_SNOWBALL, "Large Snowball" },
    { ACTOR_BOSS_HAKUGIN, "Goht" },
    { ACTOR_EN_GB2, "Spirit House Owner" },
    { ACTOR_EN_ONPUMAN, "Monkey Instrument Prompt" },
    { ACTOR_BG_TOBIRA01, "Goron Shrine Gate" },
    { ACTOR_EN_TAG_OBJ, "Seahorse Spawner (unused)" },
    { ACTOR_OBJ_DHOUSE, "Stone Bridge" },
    { ACTOR_OBJ_HAKAISI, "Gravestone" },
    { ACTOR_BG_HAKUGIN_SWITCH, "Goron Link Switch" },
    { ACTOR_EN_SNOWMAN, "Eeno" },
    { ACTOR_TG_SW, "Skulltula bonk detector" },
    { ACTOR_EN_PO_SISTERS, "Poe Sister" },
    { ACTOR_EN_PP, "Hiploop" },
    { ACTOR_EN_HAKUROCK, "Goht Debris" },
    { ACTOR_EN_HANABI, "Fireworks" },
    { ACTOR_OBJ_DOWSING, "Switch/Chest/Collectible Detector" },
    { ACTOR_OBJ_WIND, "Updraft Current/Water Current" },
    { ACTOR_EN_RACEDOG, "Racetrack Dog" },
    { ACTOR_EN_KENDO_JS, "Swordsman" },
    { ACTOR_BG_BOTIHASIRA, "Keeta Race Gatepost" },
    { ACTOR_EN_FISH2, "Marine Research Lab Fish" },
    { ACTOR_EN_PST, "Postbox" },
    { ACTOR_EN_POH, "Poe" },
    { ACTOR_OBJ_SPIDERTENT, "Tent-Shaped spide web" },
    { ACTOR_EN_ZORAEGG, "Zora Egg" },
    { ACTOR_EN_KBT, "Zubora" },
    { ACTOR_EN_GG, "Darmani's Ghost" },
    { ACTOR_EN_MARUTA, "Practice Log" },
    { ACTOR_OBJ_SNOWBALL2, "Small Snowball" },
    { ACTOR_EN_GG2, "Darmani's Ghost (copy)" },
    { ACTOR_OBJ_GHAKA, "Darmani's Gravestone" },
    { ACTOR_EN_DNP, "Deku Princess" },
    { ACTOR_EN_DAI, "Biggoron" },
    { ACTOR_BG_GORON_OYU, "Goron Hot Spring Water" },
    { ACTOR_EN_KGY, "Gabora" },
    { ACTOR_EN_INVADEPOH, "Alien" },
    { ACTOR_EN_GK, "Goron Elder's Son" },
    { ACTOR_EN_AN, "Anju" },
    { ACTOR_EN_BEE, "Giant Bee" },
    { ACTOR_EN_OT, "Seahorse" },
    { ACTOR_EN_DRAGON, "Deep Python" },
    { ACTOR_OBJ_DORA, "Gong" },
    { ACTOR_EN_BIGPO, "Big Poe" },
    { ACTOR_OBJ_KENDO_KANBAN, "Cuttable Board (Swordsman's School)" },
    { ACTOR_EN_STH, "Guy looking at Moon/Uncursed Man (Swamp Spider House)" },
    { ACTOR_BG_SINKAI_KABE, "Deep Python manager" },
    { ACTOR_BG_HAKA_CURTAIN, "Flat's Tomb Curtain" },
    { ACTOR_BG_KIN2_BOMBWALL, "Bombable Wall (Ocean Spider House)" },
    { ACTOR_BG_KIN2_FENCE, "Fireplace Gate (Ocean Spider House)" },
    { ACTOR_BG_KIN2_PICTURE, "Skullkid Painting (Ocean Spider House)" },
    { ACTOR_BG_KIN2_SHELF, "Drawers (Ocean Spider House)" },
    { ACTOR_EN_RAIL_SKB, "Stalchildren Circle" },
    { ACTOR_EN_JG, "Goron Elder" },
    { ACTOR_EN_TRU_MT, "Koume on Broom" },
    { ACTOR_OBJ_UM, "Cremia's Cart" },
    { ACTOR_EN_NEO_REEBA, "(New!) Leevers" },
    { ACTOR_BG_MBAR_CHAIR, "Milk Bar Chair" },
    { ACTOR_BG_IKANA_BLOCK, "Rotating Room Pushblock (Stone Tower Temple)" },
    { ACTOR_BG_IKANA_MIRROR, "Mirror (Stone Tower Temple)" },
    { ACTOR_BG_IKANA_ROTARYROOM, "Rotating Room (Stone Tower Temple)" },
    { ACTOR_BG_DBLUE_BALANCE, "Seesaw/Waterwhell w/ platforms (Great Bay Temple)" },
    { ACTOR_BG_DBLUE_WATERFALL, "Waterfall (Great Bay Temple)" },
    { ACTOR_EN_KAIZOKU, "Fighter Pirate" },
    { ACTOR_EN_GE2, "Purple Gerudo Pirate" },
    { ACTOR_EN_MA_YTS, "Romani (paired)" },
    { ACTOR_EN_MA_YTO, "Cremia" },
    { ACTOR_OBJ_TOKEI_TURRET, "Flag/Carnival Platform (South Clock Town)" },
    { ACTOR_BG_DBLUE_ELEVATOR, "Elevator (Great Bay Temple)" },
    { ACTOR_OBJ_WARPSTONE, "Owl Statue" },
    { ACTOR_EN_ZOG, "Mikau" },
    { ACTOR_OBJ_ROTLIFT, "Spiked Rotating Platform" },
    { ACTOR_OBJ_JG_GAKKI, "Goron Elder's Drum" },
    { ACTOR_BG_INIBS_MOVEBG, "Twinmold Arena" },
    { ACTOR_EN_ZOT, "Zora with Directions/Pot Game Zora" },
    { ACTOR_OBJ_TREE, "Tree" },
    { ACTOR_OBJ_Y2LIFT, "Elevator Platform" },
    { ACTOR_OBJ_Y2SHUTTER, "Sliding grated shutters" },
    { ACTOR_OBJ_BOAT, "Pirate Boat" },
    { ACTOR_OBJ_TARU, "Wooden Barrel/Breakable Pirate Panel" },
    { ACTOR_OBJ_HUNSUI, "Switch-Activated Geyser" },
    { ACTOR_EN_JC_MATO, "Boat Cruise Target" },
    { ACTOR_MIR_RAY3, "Mirror shield reflection and glow" },
    { ACTOR_EN_ZOB, "Japas (Zora Bassist)" },
    { ACTOR_ELF_MSG6, "Tatl Hint (3rd proximity C-Up?)" },
    { ACTOR_OBJ_NOZOKI, "Sakon's Hideout Object" },
    { ACTOR_EN_TOTO, "Toto" },
    { ACTOR_EN_RAILGIBUD, "Patrolling Gibdos" },
    { ACTOR_EN_BABA, "Bomb Shop Lady (used)" },
    { ACTOR_EN_KUJIYA, "Lottery Shop" },
    { ACTOR_EN_GEG, "Goron with Don Gero's Mask" },
    { ACTOR_OBJ_KINOKO, "Mushroom" },
    { ACTOR_OBJ_YASI, "Palm Tree" },
    { ACTOR_EN_TANRON1, "Moth Swarm (Woodfall Temple)" },
    { ACTOR_EN_TANRON2, "Wart's Bubble" },
    { ACTOR_EN_TANRON3, "Small fish (Gyorg)" },
    { ACTOR_OBJ_CHAN, "Goron Shrine Chandelier" },
    { ACTOR_EN_ZOS, "Evan (Zora Synthesizer)" },
    { ACTOR_EN_S_GORO, "Goron Shrine Goron/Bomb Shop Goron" },
    { ACTOR_EN_NB, "Anju's Grandma" },
    { ACTOR_EN_JA, "Juggler" },
    { ACTOR_BG_F40_BLOCK, "Stone Tower Block" },
    { ACTOR_BG_F40_SWITCH, "Stone Tower Floor Switch" },
    { ACTOR_EN_PO_COMPOSER, "Flat/Sharp" },
    { ACTOR_EN_GURUGURU, "Guru Guru" },
    { ACTOR_OCEFF_WIPE5, "Ocarina Effect (Sonata/Lullaby/Bossa Nova/Elegy/Oath)" },
    { ACTOR_EN_STONE_HEISHI, "Shiro" },
    { ACTOR_OCEFF_WIPE6, "Song of Soaring Ocarina Effect" },
    { ACTOR_EN_SCOPENUTS, "Business Scrub (Heart Piece)" },
    { ACTOR_EN_SCOPECROW, "Guay (Astral Observatory Telescope)" },
    { ACTOR_OCEFF_WIPE7, "Song of Healing Ocarina Effect" },
    { ACTOR_EFF_KAMEJIMA_WAVE, "Turtle Awakening Wave" },
    { ACTOR_EN_HG, "Gibdo (Pamela's Father)" },
    { ACTOR_EN_HGO, "Pamela's Father" },
    { ACTOR_EN_ZOV, "Lulu (Zora Vocalist)" },
    { ACTOR_EN_AH, "Anju's Mother" },
    { ACTOR_OBJ_HGDOOR, "Closet Door (Music Box House)" },
    { ACTOR_BG_IKANA_BOMBWALL, "Bombable Tan Floor File (Stone Tower Temple)" },
    { ACTOR_BG_IKANA_RAY, "Large light ray (Stone Tower Temple)" },
    { ACTOR_BG_IKANA_SHUTTER, "Metal Shutter (Stone Tower Temple)" },
    { ACTOR_BG_HAKA_BOMBWALL, "Bombable Wall (Beneath the Well)" },
    { ACTOR_BG_HAKA_TOMB, "Flat's Tomb" },
    { ACTOR_EN_SC_RUPPE, "Giant Rupee" },
    { ACTOR_BG_IKNV_DOUKUTU, "Sharp's Cave" },
    { ACTOR_BG_IKNV_OBJ, "Waterwheel/Stone Tower Door/Sakon's Hideout Door" },
    { ACTOR_EN_PAMERA, "Pamela" },
    { ACTOR_OBJ_HSSTUMP, "Hookshottable Tree" },
    { ACTOR_EN_HIDDEN_NUTS, "Sleeping Deku Scrub" },
    { ACTOR_EN_ZOW, "Complaining Water" },
    { ACTOR_EN_TALK, "Green Target Spot" },
    { ACTOR_EN_AL, "Madame Aroma" },
    { ACTOR_EN_TAB, "Mr. Barten" },
    { ACTOR_EN_NIMOTSU, "Bomb Shop Bag (Stolen)" },
    { ACTOR_EN_HIT_TAG, "Invisible Rupee Hitbox" },
    { ACTOR_EN_RUPPECROW, "Guay (circling Clock Town)" },
    { ACTOR_EN_TANRON4, "Seagull" },
    { ACTOR_EN_TANRON5, "Destructible Item (Twinmold Arena)" },
    { ACTOR_EN_TANRON6, "Invisible Enemy (unused)" },
    { ACTOR_EN_DAIKU2, "Milk Road Carpenter" },
    { ACTOR_EN_MUTO, "Mutoh (carpenter boss)" },
    { ACTOR_EN_BAISEN, "Viscen (Clock Town Guard leader)" },
    { ACTOR_EN_HEISHI, "Soldier (Mayor's Office)" },
    { ACTOR_EN_DEMO_HEISHI, "Shiro (Unused?)" },
    { ACTOR_EN_DT, "Mayor Detour" },
    { ACTOR_EN_CHA, "Laundry Pool Bell" },
    { ACTOR_OBJ_DINNER, "Cremia & Romani's Dinner" },
    { ACTOR_EFF_LASTDAY, "Moon Crash CS Fire Wall" },
    { ACTOR_BG_IKANA_DHARMA, "Punchable Pillar Segments (Stone Tower Temple)" },
    { ACTOR_EN_AKINDONUTS, "Trade Quest business scrub" },
    { ACTOR_EFF_STK, "Skull Kid effect" },
    { ACTOR_EN_IG, "Link the Goron" },
    { ACTOR_EN_RG, "Racing Goron" },
    { ACTOR_EN_OSK, "Igos du Ikana's head/IdI lackey's head" },
    { ACTOR_EN_STH2, "Guy waving at telescope" },
    { ACTOR_EN_YB, "Kamaro" },
    { ACTOR_EN_RZ, "Judo (Red)/Marilla (Blue) Rosa" },
    { ACTOR_EN_SCOPECOIN, "Rupees (from telescope)" },
    { ACTOR_EN_BJT, "Hand in Toilet" },
    { ACTOR_EN_BOMJIMA, "Bomber Jim" },
    { ACTOR_EN_BOMJIMB, "Bomber (being chased)" },
    { ACTOR_EN_BOMBERS, "Blue-Hatted Bomber" },
    { ACTOR_EN_BOMBERS2, "Hideout Guard" },
    { ACTOR_EN_BOMBAL, "Majora's Mask Balloon (Clock Town)" },
    { ACTOR_OBJ_MOON_STONE, "Moon's Tear" },
    { ACTOR_OBJ_MU_PICT, "Dialogue Handler (Music Box House)" },
    { ACTOR_BG_IKNINSIDE, "Object (Ancient Castle of Ikana)" },
    { ACTOR_EFF_ZORABAND, "Indigo-Gos" },
    { ACTOR_OBJ_KEPN_KOYA, "Gorman Bros. Building" },
    { ACTOR_OBJ_USIYANE, "Cow Barn Roof" },
    { ACTOR_EN_NNH, "Deku Butler's Son's Corpse" },
    { ACTOR_OBJ_KZSAKU, "Underwater Grate" },
    { ACTOR_OBJ_MILK_BIN, "Milk Jar" },
    { ACTOR_EN_KITAN, "Keaton" },
    { ACTOR_BG_ASTR_BOMBWALL, "Bombable Wall (Astral Laboratory)" },
    { ACTOR_BG_IKNIN_SUSCEIL, "Hot Checkered Celing (Ikana Castle)" },
    { ACTOR_EN_BSB, "Captain Keeta" },
    { ACTOR_EN_RECEPGIRL, "Mayor's Receptionist" },
    { ACTOR_EN_THIEFBIRD, "Takkuri" },
    { ACTOR_EN_JGAME_TSN, "Jumping Game" },
    { ACTOR_OBJ_JGAME_LIGHT, "Jumping Game Torch" },
    { ACTOR_OBJ_YADO, "2nd Floor Window (Stockpot Inn)" },
    { ACTOR_DEMO_SYOTEN, "Ikana Canyon Cleansing CS Effect" },
    { ACTOR_DEMO_MOONEND, "Moon Disappearing CS" },
    { ACTOR_BG_LBFSHOT, "Rainbow Hookshot Pillar" },
    { ACTOR_BG_LAST_BWALL, "Bombable, Climbable Wall (Moon)" },
    { ACTOR_EN_AND, "Anju (Wedding Dress)" }, 
    { ACTOR_EN_INVADEPOH_DEMO, "Alien CS Actor" },
    { ACTOR_OBJ_DANPEILIFT, "Floating Block (Deku Shrine/Snowhead Temple)" },
    { ACTOR_EN_FALL2, "Warp Beam from Moon?" },
    { ACTOR_DM_AL, "Madame Aroma (Cutscene)" },
    { ACTOR_DM_AN, "Anju (Cutscene)" },
    { ACTOR_DM_AH, "Anju's Mother (Cutscene)" },
    { ACTOR_DM_NB, "Anju's Grandma (Credits)" },
    { ACTOR_EN_DRS, "Wedding Dress Mannequin" },
    { ACTOR_EN_ENDING_HERO, "Mayor Detour (credits)" },
    { ACTOR_DM_BAL, "Tingle (Cutscene)" },
    { ACTOR_EN_PAPER, "Tingle Confetti" },
    { ACTOR_EN_HINT_SKB, "Hinting Stalchild (Oceanside Spider House)" },
    { ACTOR_DM_TAG, "Cutscene?" },
    { ACTOR_EN_BH, "Brown Bird" },
    { ACTOR_EN_ENDING_HERO2, "Viscen watching moon (cutscene)" },
    { ACTOR_EN_ENDING_HERO3, "Mutoh watching moon (cutscene)" },
    { ACTOR_EN_ENDING_HERO4, "Soldier watching moon (cutscene)" },
    { ACTOR_EN_ENDING_HERO5, "Carpenter watching moon (cutscene)" },
    { ACTOR_EN_ENDING_HERO6, "Cutscene character (unused)"},
    { ACTOR_DM_GM, "Dm_An duplicate" },
    { ACTOR_OBJ_SWPRIZE, "Item Drop Spawner (soft soil)" },
    { ACTOR_EN_INVISIBLE_RUPPE, "Invisible Ruppe" },
    { ACTOR_OBJ_ENDING, "Stump/Lighting (credits end)" },
    { ACTOR_EN_RSN, "Bomb Shop Man (credits)" },
};

std::string GetActorDescription(u16 actorNum) {
    return actorDescriptions.contains(actorNum) ? actorDescriptions[actorNum] : "???";
}

std::vector<Actor*> GetCurrentSceneActors() {
    if (!gPlayState) return {};

    std::vector<Actor*> sceneActors;
    for (size_t category = ACTORCAT_SWITCH; category < ACTORCAT_MAX; category++) {
        Actor* currentActor = gPlayState->actorCtx.actorLists[category].first;
        if (currentActor != nullptr) {
            while (currentActor != nullptr) {
                sceneActors.push_back(currentActor);
                currentActor = currentActor->next;
            }
        }
    }
    return sceneActors;

}

static bool needs_reset = false;

static Actor* display{};
static Actor* fetch;
static ActorInfo newActor;
static std::vector<Actor*> list;

static s16 lastSceneId = 0;
static s16 newActorId = 0;
static u8 category = 0;
static s8 method = -1;
static std::string filler = "Please select";

void ResetVariables() {
    display = fetch = {};
    newActor = {};
    filler = "Please select";
}

void ActorViewerWindow::DrawElement() {
    ImGui::SetNextWindowSize(ImVec2(520, 600), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Actor Viewer", &mIsVisible, ImGuiWindowFlags_NoFocusOnAppearing)) {
        ImGui::End();
        return;
    }

    if (gPlayState != nullptr) {
        needs_reset = lastSceneId != gPlayState->sceneId;
        if (needs_reset) {
            ResetVariables();
            lastSceneId = gPlayState->sceneId;
            needs_reset = false;
        }

        if (ImGui::BeginCombo("Actor", filler.c_str())) {
            if (gPlayState != nullptr && lastSceneId != gPlayState->sceneId) {
                list = GetCurrentSceneActors();
                lastSceneId = gPlayState->sceneId;
            }
            if (!list.empty()) {
                u8 count = 0;
                u8 prev_category = 0xff;
                for (size_t i = 0; i < list.size(); i++) {
                    std::string label = acMapping[list[i]->category];
                    if (list[i]->category != prev_category) {
                        prev_category = list[i]->category;
                        count = 1;
                    } else {
                        count++;
                    }
                    label += " " + std::to_string(count) + ": " + GetActorDescription(list[i]->id);
                    if (ImGui::Selectable(label.c_str())) {
                        method = LIST;
                        display = list[i];
                        newActorId = i;
                        filler = label;
                        break;
                    }
                }
            }
            ImGui::EndCombo();
        }

        if (ImGui::TreeNode("Selected Actor")) {
            if (display != nullptr) {
                ImGui::BeginGroup();
                ImGui::Text("ID: %d", display->id);
                ImGui::Text("Description: %s", GetActorDescription(display->id).c_str());
                ImGui::Text("Category: %s", acMapping[display->category]);
                ImGui::Text("Params: %d", &display->params);
                ImGui::EndGroup();

                ImGui::PushItemWidth(ImGui::GetFontSize() * 10);

                ImGui::BeginGroup();
                f32 pos[3] = {display->world.pos.x, display->world.pos.y, display->world.pos.z};
                ImGui::Text("Actor Position");
                ImGui::InputScalar("x", ImGuiDataType_Float, &display->world.pos.x);
                ImGui::SameLine();
                ImGui::InputScalar("y", ImGuiDataType_Float, &display->world.pos.y);
                ImGui::SameLine();
                ImGui::InputScalar("z", ImGuiDataType_Float, &display->world.pos.z);
                ImGui::EndGroup();

                ImGui::BeginGroup();
                s16 rot[3] = {display->world.rot.x, display->world.rot.y, display->world.rot.z};
                ImGui::Text("Actor Rotation");
                ImGui::InputScalar("rx", ImGuiDataType_S16, &display->world.rot.x);
                ImGui::SameLine();
                ImGui::InputScalar("ry", ImGuiDataType_S16, &display->world.rot.y);
                ImGui::SameLine();
                ImGui::InputScalar("rz", ImGuiDataType_S16, &display->world.rot.z);
                ImGui::EndGroup();

                ImGui::PopItemWidth();
                ImGui::PushItemWidth(ImGui::GetFontSize() * 5);

                if ((display->category == ACTORCAT_BOSS || display->category == ACTORCAT_ENEMY) &&
                    display->colChkInfo.health > 0) {
                    ImGui::InputScalar("Enemy Health", ImGuiDataType_U8, &display->colChkInfo.health);
                }
                ImGui::PopItemWidth();

                ImGui::BeginGroup();
                ImGui::Text("Flags");
                UIWidgets::DrawFlagArray32("flags", display->flags);
                ImGui::EndGroup();

                ImGui::BeginGroup();
                ImGui::Text("BG Check");
                UIWidgets::DrawFlagArray16("bgCheckFlags", display->bgCheckFlags);
                ImGui::EndGroup();
            }

            if (UIWidgets::Button("Refresh")) {
                list = GetCurrentSceneActors();
                display = list[newActorId];
            }

            if (UIWidgets::Button("Go to Actor")) {
                Player* player = GET_PLAYER(gPlayState);
                Math_Vec3f_Copy(&player->actor.world.pos, &display->world.pos);
                Math_Vec3f_Copy(&player->actor.home.pos, &display->world.pos);
            }

            if (UIWidgets::Button("Fetch: Target", {.tooltip = "Grabs actor with target arrow above it."})) {
                Player* player = GET_PLAYER(gPlayState);
                fetch = player->lockOnActor;
                if (fetch != nullptr) {
                    display = fetch;
                    category = fetch->category;
                    list = GetCurrentSceneActors();
                    method = TARGET;
                }
            }
            if (UIWidgets::Button("Fetch: Held", {.tooltip = "Grabs actor Link is currently holding."})) {
                Player* player = GET_PLAYER(gPlayState);
                fetch = player->heldActor;
                if (fetch != nullptr) {
                    display = fetch;
                    category = fetch->category;
                    list = GetCurrentSceneActors();
                    method = HOLD;
                }
            }

            
            if (UIWidgets::Button("Kill", {.color = UIWidgets::Colors::Red}) && display != nullptr &&
                display->id != ACTOR_PLAYER) {
                Actor_Kill(display);
            }
            ImGui::TreePop();

        }

        if (ImGui::TreeNode("New...")) {
            ImGui::PushItemWidth(ImGui::GetFontSize() * 10);
            ImU16 one = 1;

            ImGui::Text("%s", GetActorDescription(newActor.id).c_str());
            ImGui::InputScalar("ID", ImGuiDataType_S16, &newActor.id, &one);
            ImGui::InputScalar("Params", ImGuiDataType_S16, &newActor.params, &one);

            ImGui::BeginGroup();
            ImGui::Text("New Actor Position");
            ImGui::InputScalar("x", ImGuiDataType_Float, &newActor.pos.x);
            ImGui::SameLine();
            ImGui::InputScalar("y", ImGuiDataType_Float, &newActor.pos.y);
            ImGui::SameLine();
            ImGui::InputScalar("z", ImGuiDataType_Float, &newActor.pos.z);
            ImGui::EndGroup();

            ImGui::BeginGroup();
            ImGui::Text("New Actor Rotation");
            ImGui::InputScalar("rX", ImGuiDataType_S16, &newActor.rot.x);
            ImGui::SameLine();
            ImGui::InputScalar("rY", ImGuiDataType_S16, &newActor.rot.y);
            ImGui::SameLine();
            ImGui::InputScalar("rZ", ImGuiDataType_S16, &newActor.rot.z);
            ImGui::EndGroup();

            if (UIWidgets::Button("Fetch from Link")) {
                Player* player = GET_PLAYER(gPlayState);
                newActor.pos = player->actor.world.pos;
                newActor.rot = player->actor.world.rot;
            }

            if (UIWidgets::Button("Spawn", {.color = UIWidgets::Colors::Green})) {
                Actor_Spawn(&gPlayState->actorCtx, gPlayState, newActor.id, newActor.pos.x, newActor.pos.y,
                    newActor.pos.z, newActor.rot.x, newActor.rot.y, newActor.rot.z, newActor.params);
            }

            if (UIWidgets::Button("Spawn as Child", {.color = UIWidgets::Colors::Green})) {
                Actor* parent = display;
                if (parent != nullptr) {
                    Actor_SpawnAsChild(&gPlayState->actorCtx, parent, gPlayState, newActor.id, newActor.pos.x,
                        newActor.pos.y, newActor.pos.z, newActor.rot.x, newActor.rot.y, newActor.rot.z, newActor.params);
                } else {
                    Audio_PlaySfx(NA_SE_SY_ERROR);
                }
            }

            if (UIWidgets::Button("Reset")) {
                newActor = {};
            }

            ImGui::TreePop();
        }
    } else {
        ImGui::Text("Playstate needed for actors!");
    }
    ImGui::End();    
}

void ActorViewerWindow::InitElement() {

}

