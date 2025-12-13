// Local includes
#include "Registry.h"

// Standard library
#include <map>
#include <vector>

// Assets
#include "assets/archives/schedule_dma_static/schedule_dma_static_yar.h"

extern "C" {
#include "variables.h"
#include "z64item.h"
}

namespace Achievements {

namespace StaticData {

// ============================================================================
// Macro Helpers
// ============================================================================

#define ACH_AI(id) AchievementId::id
#define ACH_AC(category) AchievementCategory::category
#define ACH_AE(event) AchievementEvent::event
#define ACH_TEX(path) (const char*)path

#define ACH_ACHIEVEMENT_ENTRY(id, name, description, iconPath, secret, category, harbourMastery, ...) \
    {                                                                                                 \
        id, {                                                                                         \
            id, name, description, iconPath, secret, category, harbourMastery, {                      \
                __VA_ARGS__                                                                           \
            }                                                                                         \
        }                                                                                             \
    }

#define ACH_EVENT_ENTRY(id, name, description) \
    {                                          \
        id, {                                  \
            id, name, description, {           \
            }                                  \
        }                                      \
    }

std::map<AchievementId, Achievement> Data;
std::map<AchievementEvent, Event> EventData;

void Init() {
    // clang-format off
    Data = {
        ACH_ACHIEVEMENT_ENTRY(ACH_AI(BN_HONORARY_MEMBER),
            "Honorary Member",
            "Fully complete the entry for the Bomber's Gang",
            ACH_TEX(gBombersNotebookPhotoBombersTex),
            false,
            ACH_AC(VANILLA),
            5,
            ACH_AE(EVENT_LEARNED_SECRET_CODE),
            ACH_AE(EVENT_RECEIVED_BOMBERS_NOTEBOOK)
        ),

        ACH_ACHIEVEMENT_ENTRY(ACH_AI(BN_FAITHFUL_BRIDE),
            "Faithful Bride",
            "Fully complete the entry for Anju",
            ACH_TEX(gBombersNotebookPhotoAnjuTex),
            false,
            ACH_AC(VANILLA),
            5,
            ACH_AE(EVENT_RECEIVED_ROOM_KEY),
            ACH_AE(EVENT_PROMISED_MIDNIGHT_MEETING),
            ACH_AE(EVENT_RECEIVED_LETTER_TO_KAFEI),
            ACH_AE(EVENT_PROMISED_TO_MEET_KAFEI),
            ACH_AE(EVENT_DELIVERED_PENDANT_OF_MEMORIES),
            ACH_AE(EVENT_RECEIVED_COUPLES_MASK)
        ),

        ACH_ACHIEVEMENT_ENTRY(ACH_AI(BN_MINI_HE),
            "Mini-He",
            "Fully complete the entry for Kafei",
            ACH_TEX(gBombersNotebookPhotoKafeiTex),
            false,
            ACH_AC(VANILLA),
            5,
            ACH_AE(EVENT_RECEIVED_PENDANT_OF_MEMORIES),
            ACH_AE(EVENT_ESCAPED_SAKONS_HIDEOUT),
            ACH_AE(EVENT_RECEIVED_COUPLES_MASK)
        ),

        ACH_ACHIEVEMENT_ENTRY(ACH_AI(BN_AMBIGUOUS_ALLEGIANCE),
            "Ambiguous Allegiance",
            "Fully complete the entry for the Curiosity Shop Owner",
            ACH_TEX(gBombersNotebookPhotoCuriosityShopManTex),
            false,
            ACH_AC(VANILLA),
            5,
            ACH_AE(EVENT_RECEIVED_KEATON_MASK),
            ACH_AE(EVENT_RECEIVED_PRIORITY_MAIL),
            ACH_AE(EVENT_RECEIVED_ALL_NIGHT_MASK)
        ),

        ACH_ACHIEVEMENT_ENTRY(ACH_AI(BN_FRAIL_DID),
            "Frail D.I.D.",
            "Fully complete the entry for the Bomb Shop Owner's Mother",
            ACH_TEX(gBombersNotebookPhotoBombShopLadyTex),
            false,
            ACH_AC(VANILLA),
            5,
            ACH_AE(EVENT_RECEIVED_BLAST_MASK)
        ),

        ACH_ACHIEVEMENT_ENTRY(ACH_AI(BN_BRAVE_LITTLE_ARCHER),
            "Brave Little Archer",
            "Fully complete the entry for Romani",
            ACH_TEX(gBombersNotebookPhotoRomaniTex),
            false,
            ACH_AC(VANILLA),
            5,
            ACH_AE(EVENT_PROMISED_TO_HELP_WITH_THEM),
            ACH_AE(EVENT_DEFENDED_AGAINST_THEM),
            ACH_AE(EVENT_RECEIVED_MILK_BOTTLE)
        ),

        ACH_ACHIEVEMENT_ENTRY(ACH_AI(BN_MATURE_RANCH_HAND),
            "Mature Ranch Hand",
            "Fully complete the entry for Cremia",
            ACH_TEX(gBombersNotebookPhotoCremiaTex),
            false,
            ACH_AC(VANILLA),
            5,
            ACH_AE(EVENT_ESCORTED_CREMIA),
            ACH_AE(EVENT_RECEIVED_ROMANIS_MASK)
        ),

        ACH_ACHIEVEMENT_ENTRY(ACH_AI(BN_DROWNING_IN_RESPONSIBILITIES),
            "Drowning in Responsibilities",
            "Fully complete the entry for Mayor Doutour",
            ACH_TEX(gBombersNotebookPhotoMayorDotourTex),
            false,
            ACH_AC(VANILLA),
            5,
            ACH_AE(EVENT_RECEIVED_MAYOR_HP)
        ),

        ACH_ACHIEVEMENT_ENTRY(ACH_AI(BN_CONCERNED_MOTHER),
            "Concerned Mother",
            "Fully complete the entry for Madame Aroma",
            ACH_TEX(gBombersNotebookPhotoMadameAromaTex),
            false,
            ACH_AC(VANILLA),
            5,
            ACH_AE(EVENT_RECEIVED_KAFEIS_MASK),
            ACH_AE(EVENT_DELIVERED_PRIORITY_MAIL)
        ),

        ACH_ACHIEVEMENT_ENTRY(ACH_AI(BN_LOOKING_FOR_VOLUNTEERS),
            "Looking for Volunteers",
            "Fully complete the entry for Toto",
            ACH_TEX(gBombersNotebookPhotoTotoTex),
            false,
            ACH_AC(VANILLA),
            5,
            ACH_AE(EVENT_RECEIVED_CIRCUS_LEADERS_MASK)
        ),

        ACH_ACHIEVEMENT_ENTRY(ACH_AI(BN_BREAKAWAY_BROTHER),
            "Breakaway Brother",
            "Fully complete the entry for Gorman",
            ACH_TEX(gBombersNotebookPhotoGormanTex),
            false,
            ACH_AC(VANILLA),
            5,
            ACH_AE(EVENT_RECEIVED_CIRCUS_LEADERS_MASK)
        ),

        ACH_ACHIEVEMENT_ENTRY(ACH_AI(BN_ALWAYS_MAKES_HIS_APPOINTED_DELIVERIES),
            "Always Makes His Appointed Deliveries",
            "Fully complete the entry for the Postman",
            ACH_TEX(gBombersNotebookPhotoPostmanTex),
            false,
            ACH_AC(VANILLA),
            5,
            ACH_AE(EVENT_RECEIVED_POSTMAN_HP),
            ACH_AE(EVENT_DEPOSITED_LETTER_TO_KAFEI),
            ACH_AE(EVENT_RECEIVED_POSTMANS_HAT)
        ),

        ACH_ACHIEVEMENT_ENTRY(ACH_AI(BN_PREPARE_FOR_TROUBLE),
            "Prepare For Trouble, and Make it Double!",
            "Fully complete the entry for the Rosa Sisters",
            ACH_TEX(gBombersNotebookPhotoRosaSistersTex),
            false,
            ACH_AC(VANILLA),
            5,
            ACH_AE(EVENT_RECEIVED_ROSA_SISTERS_HP)
        ),

        ACH_ACHIEVEMENT_ENTRY(ACH_AI(BN_YOU_MISSED_A_SPOT),
            "You Missed a Spot",
            "Fully complete the entry for ???",
            ACH_TEX(gBombersNotebookPhotoToiletHandTex),
            false,
            ACH_AC(VANILLA),
            5,
            ACH_AE(EVENT_RECEIVED_TOILET_HAND_HP)
        ),

        ACH_ACHIEVEMENT_ENTRY(ACH_AI(BN_STORYTELLER),
            "Storyteller",
            "Fully complete the entry for Anju's Grandmother",
            ACH_TEX(gBombersNotebookPhotoAnjusGrandmotherTex),
            false,
            ACH_AC(VANILLA),
            5,
            ACH_AE(EVENT_RECEIVED_GRANDMA_SHORT_STORY_HP),
            ACH_AE(EVENT_RECEIVED_GRANDMA_LONG_STORY_HP)
        ),

        ACH_ACHIEVEMENT_ENTRY(ACH_AI(BN_PASSING_ON_THE_GROOVE),
            "Passing on the Groove",
            "Fully complete the entry for Kamaro",
            ACH_TEX(gBombersNotebookPhotoKamaroTex),
            false,
            ACH_AC(VANILLA),
            5,
            ACH_AE(EVENT_RECEIVED_KAMAROS_MASK)
        ),

        ACH_ACHIEVEMENT_ENTRY(ACH_AI(BN_CARETAKER_OF_THE_FOWL),
            "Caretaker of the Fowl",
            "Fully complete the entry for Grog",
            ACH_TEX(gBombersNotebookPhotoGrogTex),
            false,
            ACH_AC(VANILLA),
            5,
            ACH_AE(EVENT_RECEIVED_BUNNY_HOOD)
        ),

        ACH_ACHIEVEMENT_ENTRY(ACH_AI(BN_THERES_THREE_OF_YOU),
            "There's THREE of you?!",
            "Fully complete the entry for the Gorman Brothers",
            ACH_TEX(gBombersNotebookPhotoGormanBrothersTex),
            false,
            ACH_AC(VANILLA),
            5,
            ACH_AE(EVENT_RECEIVED_GAROS_MASK),
            ACH_AE(EVENT_ESCORTED_CREMIA)
        ),

        ACH_ACHIEVEMENT_ENTRY(ACH_AI(BN_PERFECTING_THE_ART),
            "Perfecting the Art of Standing Perfectly Still",
            "Fully complete the entry for Shiro",
            ACH_TEX(gBombersNotebookPhotoShiroTex),
            false,
            ACH_AC(VANILLA),
            5,
            ACH_AE(EVENT_RECEIVED_STONE_MASK)
        ),

        ACH_ACHIEVEMENT_ENTRY(ACH_AI(BN_WINDMILL_MAN),
            "Windmill Man",
            "Fully complete the entry for Guru-Guru",
            ACH_TEX(gBombersNotebookPhotoGuruGuruTex),
            false,
            ACH_AC(VANILLA),
            5,
            ACH_AE(EVENT_RECEIVED_BREMEN_MASK)
        ),

        ACH_ACHIEVEMENT_ENTRY(ACH_AI(BN_LOCAL_HERO),
            "Local Hero",
            "100% the Bomber's Notebook",
            ACH_TEX(gBombersNotebookEntryIconRibbonTex),
            true,
            ACH_AC(VANILLA),
            25,
            ACH_AE(EVENT_LEARNED_SECRET_CODE),
            ACH_AE(EVENT_RECEIVED_BOMBERS_NOTEBOOK),
            ACH_AE(EVENT_RECEIVED_COUPLES_MASK),
            ACH_AE(EVENT_RECEIVED_KEATON_MASK),
            ACH_AE(EVENT_RECEIVED_ALL_NIGHT_MASK),
            ACH_AE(EVENT_RECEIVED_BLAST_MASK),
            ACH_AE(EVENT_RECEIVED_MILK_BOTTLE),
            ACH_AE(EVENT_RECEIVED_ROMANIS_MASK),
            ACH_AE(EVENT_RECEIVED_MAYOR_HP),
            ACH_AE(EVENT_RECEIVED_KAFEIS_MASK),
            ACH_AE(EVENT_RECEIVED_CIRCUS_LEADERS_MASK),
            ACH_AE(EVENT_RECEIVED_POSTMAN_HP),
            ACH_AE(EVENT_DEPOSITED_LETTER_TO_KAFEI),
            ACH_AE(EVENT_RECEIVED_POSTMANS_HAT),
            ACH_AE(EVENT_RECEIVED_ROSA_SISTERS_HP),
            ACH_AE(EVENT_RECEIVED_TOILET_HAND_HP),
            ACH_AE(EVENT_RECEIVED_GRANDMA_LONG_STORY_HP),
            ACH_AE(EVENT_RECEIVED_KAMAROS_MASK),
            ACH_AE(EVENT_RECEIVED_BUNNY_HOOD),
            ACH_AE(EVENT_RECEIVED_GAROS_MASK),
            ACH_AE(EVENT_ESCORTED_CREMIA),
            ACH_AE(EVENT_RECEIVED_STONE_MASK),
            ACH_AE(EVENT_RECEIVED_BREMEN_MASK)
        ),

        ACH_ACHIEVEMENT_ENTRY(ACH_AI(TIMELORD),
            "Timelord",
            "Play the Song of Double Time and Reverse Song of Time.",
            ACH_TEX(gItemIcons[ITEM_OCARINA_OF_TIME]),
            false,
            ACH_AC(GENERAL),
            20,
            ACH_AE(EVENT_PLAYED_SONG_OF_DOUBLE_TIME),
            ACH_AE(EVENT_PLAYED_INVERTED_SONG_OF_TIME)
        )
    };
    // clang-format on

    EventData = {
        ACH_EVENT_ENTRY(ACH_AE(EVENT_LEARNED_SECRET_CODE), "Learned Secret Code",
                        "Player discovered the Bomber's hideout password"),
        ACH_EVENT_ENTRY(ACH_AE(EVENT_RECEIVED_BOMBERS_NOTEBOOK), "Received Bomber's Notebook",
                        "Player received the Bomber's Notebook from Jim"),
        ACH_EVENT_ENTRY(ACH_AE(EVENT_RECEIVED_ROOM_KEY), "Received Room Key", "Player received the room key from Anju"),
        ACH_EVENT_ENTRY(ACH_AE(EVENT_PROMISED_MIDNIGHT_MEETING), "Promised Midnight Meeting",
                        "Player promised to meet Anju at midnight"),
        ACH_EVENT_ENTRY(ACH_AE(EVENT_RECEIVED_LETTER_TO_KAFEI), "Received Letter to Kafei",
                        "Player received Anju's letter to Kafei"),
        ACH_EVENT_ENTRY(ACH_AE(EVENT_DEPOSITED_LETTER_TO_KAFEI), "Deposited Letter to Kafei",
                        "Player mailed Anju's letter to Kafei"),
        ACH_EVENT_ENTRY(ACH_AE(EVENT_DELIVERED_PENDANT_OF_MEMORIES), "Delivered Pendant of Memories",
                        "Player delivered the Pendant of Memories"),
        ACH_EVENT_ENTRY(ACH_AE(EVENT_RECEIVED_COUPLES_MASK), "Received Couple's Mask",
                        "Player received the Couple's Mask from the reunited couple"),
        ACH_EVENT_ENTRY(ACH_AE(EVENT_PROMISED_TO_MEET_KAFEI), "Promised to Meet Kafei",
                        "Player promised to meet with Kafei"),
        ACH_EVENT_ENTRY(ACH_AE(EVENT_ESCAPED_SAKONS_HIDEOUT), "Escaped Sakon's Hideout",
                        "Player successfully escaped from Sakon's hideout"),
        ACH_EVENT_ENTRY(ACH_AE(EVENT_RECEIVED_KEATON_MASK), "Received Keaton Mask",
                        "Player received the Keaton Mask from the Curiosity Shop Owner"),
        ACH_EVENT_ENTRY(ACH_AE(EVENT_RECEIVED_PRIORITY_MAIL), "Received Priority Mail",
                        "Player received the Priority Mail"),
        ACH_EVENT_ENTRY(ACH_AE(EVENT_RECEIVED_ALL_NIGHT_MASK), "Received All-Night Mask",
                        "Player received the All-Night Mask"),
        ACH_EVENT_ENTRY(ACH_AE(EVENT_RECEIVED_BLAST_MASK), "Received Blast Mask",
                        "Player received the Blast Mask from the Bomb Shop Owner's Mother"),
        ACH_EVENT_ENTRY(ACH_AE(EVENT_PROMISED_TO_HELP_WITH_THEM), "Promised to Help with Them",
                        "Player promised Romani to help defend against 'them'"),
        ACH_EVENT_ENTRY(ACH_AE(EVENT_DEFENDED_AGAINST_THEM), "Defended Against Them",
                        "Player successfully defended the ranch from the alien invasion"),
        ACH_EVENT_ENTRY(ACH_AE(EVENT_RECEIVED_MILK_BOTTLE), "Received Milk Bottle",
                        "Player received a bottle of Romani's milk"),
        ACH_EVENT_ENTRY(ACH_AE(EVENT_ESCORTED_CREMIA), "Escorted Cremia", "Player escorted Cremia on the milk run"),
        ACH_EVENT_ENTRY(ACH_AE(EVENT_RECEIVED_ROMANIS_MASK), "Received Romani's Mask",
                        "Player received Romani's Mask from Cremia"),
        ACH_EVENT_ENTRY(ACH_AE(EVENT_RECEIVED_MAYOR_HP), "Received Mayor Heart Piece",
                        "Player received a Heart Piece from Mayor Dotour"),
        ACH_EVENT_ENTRY(ACH_AE(EVENT_DELIVERED_PRIORITY_MAIL), "Delivered Priority Mail",
                        "Player delivered the Priority Mail to Madame Aroma"),
        ACH_EVENT_ENTRY(ACH_AE(EVENT_RECEIVED_CIRCUS_LEADERS_MASK), "Received Circus Leader's Mask",
                        "Player received the Circus Leader's Mask from Toto"),
        ACH_EVENT_ENTRY(ACH_AE(EVENT_MET_GORMAN), "Met Gorman", "Player met Gorman in the Milk Bar"),
        ACH_EVENT_ENTRY(ACH_AE(EVENT_RECEIVED_POSTMAN_HP), "Received Postman Heart Piece",
                        "Player received a Heart Piece from the Postman"),
        ACH_EVENT_ENTRY(ACH_AE(EVENT_RECEIVED_POSTMANS_HAT), "Received Postman's Hat",
                        "Player received the Postman's Hat"),
        ACH_EVENT_ENTRY(ACH_AE(EVENT_RECEIVED_ROSA_SISTERS_HP), "Received Rosa Sisters Heart Piece",
                        "Player received a Heart Piece from the Rosa Sisters"),
        ACH_EVENT_ENTRY(ACH_AE(EVENT_RECEIVED_TOILET_HAND_HP), "Received Toilet Hand Heart Piece",
                        "Player received a Heart Piece from the toilet hand"),
        ACH_EVENT_ENTRY(ACH_AE(EVENT_RECEIVED_GRANDMA_SHORT_STORY_HP), "Received Grandma Short Story Heart Piece",
                        "Player received a Heart Piece for listening to grandma's short story"),
        ACH_EVENT_ENTRY(ACH_AE(EVENT_RECEIVED_GRANDMA_LONG_STORY_HP), "Received Grandma Long Story Heart Piece",
                        "Player received a Heart Piece for listening to grandma's long story"),
        ACH_EVENT_ENTRY(ACH_AE(EVENT_RECEIVED_KAMAROS_MASK), "Received Kamaro's Mask",
                        "Player received Kamaro's Mask from the dancing ghost"),
        ACH_EVENT_ENTRY(ACH_AE(EVENT_RECEIVED_BUNNY_HOOD), "Received Bunny Hood",
                        "Player received the Bunny Hood from Grog"),
        ACH_EVENT_ENTRY(ACH_AE(EVENT_RECEIVED_GAROS_MASK), "Received Garo's Mask",
                        "Player received the Garo's Mask from the Gorman Brothers"),
        ACH_EVENT_ENTRY(ACH_AE(EVENT_RECEIVED_STONE_MASK), "Received Stone Mask",
                        "Player received the Stone Mask from Shiro"),
        ACH_EVENT_ENTRY(ACH_AE(EVENT_RECEIVED_BREMEN_MASK), "Received Bremen Mask",
                        "Player received the Bremen Mask from Guru-Guru"),
        ACH_EVENT_ENTRY(ACH_AE(EVENT_RECEIVED_KAFEIS_MASK), "Received Kafei's Mask",
                        "Player received Kafei's Mask from Madame Aroma"),
        ACH_EVENT_ENTRY(ACH_AE(EVENT_PLAYED_SONG_OF_DOUBLE_TIME), "Played Song of Double Time",
                        "Player used the Song of Double Time to speed up time"),
        ACH_EVENT_ENTRY(ACH_AE(EVENT_PLAYED_INVERTED_SONG_OF_TIME), "Played Inverted Song of Time",
                        "Player used the Inverted Song of Time to slow down time"),
        ACH_EVENT_ENTRY(ACH_AE(EVENT_TINGLE_MAP_CLOCK_TOWN), "Bought Clock Town Map",
                        "Player bought the Clock Town map from Tingle"),
        ACH_EVENT_ENTRY(ACH_AE(EVENT_TINGLE_MAP_WOODFALL), "Bought Woodfall Map",
                        "Player bought the Woodfall map from Tingle"),
        ACH_EVENT_ENTRY(ACH_AE(EVENT_TINGLE_MAP_SNOWHEAD), "Bought Snowhead Map",
                        "Player bought the Snowhead map from Tingle"),
        ACH_EVENT_ENTRY(ACH_AE(EVENT_TINGLE_MAP_ROMANI_RANCH), "Bought Romani Ranch Map",
                        "Player bought the Romani Ranch map from Tingle"),
        ACH_EVENT_ENTRY(ACH_AE(EVENT_RECEIVED_LAND_TITLE_DEED), "Received Land Title Deed",
                        "Player obtained the Land Title Deed"),
        ACH_EVENT_ENTRY(ACH_AE(EVENT_BANK_REWARD_3), "Bank Reward 5000 Rupees", "Player earned the final bank reward"),
        ACH_EVENT_ENTRY(ACH_AE(EVENT_BANK_REWARD_2), "Bank Reward 200 Rupees", "Player earned the second bank reward"),
        ACH_EVENT_ENTRY(ACH_AE(EVENT_BANK_REWARD_1), "Bank Reward 200 Rupees", "Player earned the first bank reward"),
        ACH_EVENT_ENTRY(ACH_AE(EVENT_LEARNED_SONATA_OF_AWAKENING), "Learned Sonata of Awakening",
                        "Player learned the Sonata of Awakening"),
        ACH_EVENT_ENTRY(ACH_AE(EVENT_DEKU_PLAYGROUND_REWARD_ALL_DAYS), "Deku Playground Champion",
                        "Player cleared all three days at the Deku Playground"),
        ACH_EVENT_ENTRY(ACH_AE(EVENT_DEKU_PLAYGROUND_REWARD_ANY_DAY), "Deku Playground Clear",
                        "Player cleared a day at the Deku Playground"),
        ACH_EVENT_ENTRY(ACH_AE(EVENT_RECEIVED_MASK_OF_SCENTS), "Received Mask of Scents",
                        "Player received the Mask of Scents"),
        ACH_EVENT_ENTRY(ACH_AE(EVENT_RECEIVED_DOGGY_RACETRACK_HP), "Doggy Racetrack Prize",
                        "Player won the Doggy Racetrack Heart Piece"),
        ACH_EVENT_ENTRY(ACH_AE(EVENT_RECEIVED_GREAT_SPIN_ATTACK), "Great Spin Attack",
                        "Player obtained the Great Spin Attack"),
        ACH_EVENT_ENTRY(ACH_AE(EVENT_GORON_RACE_REWARD), "Goron Race Reward", "Player won the Goron Race"),
        ACH_EVENT_ENTRY(ACH_AE(EVENT_RECEIVED_BOMB_BAG_FROM_GORON_SCRUB), "Bought Bomb Bag",
                        "Player bought the Bomb Bag from the Goron Scrub"),
        ACH_EVENT_ENTRY(ACH_AE(EVENT_RECEIVED_MOUNTAIN_TITLE_DEED), "Received Mountain Title Deed",
                        "Player obtained the Mountain Title Deed"),
        ACH_EVENT_ENTRY(ACH_AE(EVENT_RECEIVED_POWDER_KEG), "Received Powder Keg", "Player received the Powder Keg"),
        ACH_EVENT_ENTRY(ACH_AE(EVENT_HONEY_DARLING_REWARD_ALL_DAYS), "Honey & Darling Champion",
                        "Player cleared all three Honey & Darling games"),
        ACH_EVENT_ENTRY(ACH_AE(EVENT_HONEY_DARLING_REWARD_ANY_DAY), "Honey & Darling Clear",
                        "Player cleared a Honey & Darling game"),
        ACH_EVENT_ENTRY(ACH_AE(EVENT_RECEIVED_LAB_FISH_HP), "Lab Fish Reward",
                        "Player fed the lab fish and got a Heart Piece"),
        ACH_EVENT_ENTRY(ACH_AE(EVENT_LEARNED_ZORA_SONG), "Learned Zora Song",
                        "Player learned the Zora Hall rehearsal song"),
        ACH_EVENT_ENTRY(ACH_AE(EVENT_CLOCK_TOWN_STRAY_FAIRY), "Clock Town Stray Fairy",
                        "Player collected the Clock Town Stray Fairy"),
        ACH_EVENT_ENTRY(ACH_AE(EVENT_RECEIVED_DON_GERO_MASK), "Received Don Gero Mask",
                        "Player received the Don Gero Mask"),
        ACH_EVENT_ENTRY(ACH_AE(EVENT_RECEIVED_MOON_TEAR), "Received Moon's Tear", "Player obtained the Moon's Tear"),
        ACH_EVENT_ENTRY(ACH_AE(EVENT_RECEIVED_PINNACLE_ROCK_HP), "Pinnacle Rock Prize",
                        "Player rescued the seahorses at Pinnacle Rock"),
        ACH_EVENT_ENTRY(ACH_AE(EVENT_TOWN_ARCHERY_REWARD_1), "Town Archery Reward",
                        "Player cleared the Town Shooting Gallery"),
        ACH_EVENT_ENTRY(ACH_AE(EVENT_SWAMP_ARCHERY_REWARD_1), "Swamp Archery First Prize",
                        "Player cleared the Swamp Shooting Gallery"),
        ACH_EVENT_ENTRY(ACH_AE(EVENT_SWAMP_ARCHERY_REWARD_2), "Swamp Archery Second Prize",
                        "Player set a new record at the Swamp Shooting Gallery"),
        ACH_EVENT_ENTRY(ACH_AE(EVENT_RECEIVED_SWAMP_TITLE_DEED), "Received Swamp Title Deed",
                        "Player obtained the Swamp Title Deed"),
        ACH_EVENT_ENTRY(ACH_AE(EVENT_LEARNED_SONG_OF_SOARING), "Learned Song of Soaring",
                        "Player learned the Song of Soaring"),
        ACH_EVENT_ENTRY(ACH_AE(EVENT_SWORDSMAN_SCHOOL_HP), "Swordsman School Prize",
                        "Player earned the Swordsman's School Heart Piece"),
        ACH_EVENT_ENTRY(ACH_AE(EVENT_BOAT_ARCHERY_HEART_PIECE), "Boat Archery Prize",
                        "Player won the Boat Archery Heart Piece"),
        ACH_EVENT_ENTRY(ACH_AE(EVENT_RECEIVED_PICTOGRAPH_BOX), "Received Pictograph Box",
                        "Player received the Pictograph Box"),
        ACH_EVENT_ENTRY(ACH_AE(EVENT_TINGLE_PICTURE_HEART_PIECE), "Tingle's Picture Prize",
                        "Player showed Tingle his picture"),
        ACH_EVENT_ENTRY(ACH_AE(EVENT_LEARNED_GORON_LULLABY_INTRO), "Learned Goron Lullaby Intro",
                        "Player learned the first half of Goron Lullaby"),
        ACH_EVENT_ENTRY(ACH_AE(EVENT_BEAVER_RACE_2), "Beaver Race Second Prize", "Player won the second Beaver Race"),
        ACH_EVENT_ENTRY(ACH_AE(EVENT_BEAVER_RACE_1), "Beaver Race First Prize", "Player won the first Beaver Race"),
        ACH_EVENT_ENTRY(ACH_AE(EVENT_RECEIVED_RED_POTION), "Received Red Potion",
                        "Player got a Red Potion from Kotake"),
        ACH_EVENT_ENTRY(ACH_AE(EVENT_RECEIVED_OCEAN_TITLE_DEED), "Received Ocean Title Deed",
                        "Player obtained the Ocean Title Deed"),
        ACH_EVENT_ENTRY(ACH_AE(EVENT_ZORA_HALL_EVAN_HP), "Zora Hall Evan Reward",
                        "Player received a Heart Piece from Evan")
    };

    for (auto& [achievementEventId, event] : EventData) {
        for (const auto& [achievementId, achievement] : Data) {
            for (const AchievementEvent requiredEvent : achievement.requiredEvents) {
                if (requiredEvent == achievementEventId) {
                    event.dependentAchievements.push_back(achievement.id);
                    break;
                }
            }
        }
    }
}

const Achievement* GetAchievement(AchievementId achievementId) {
    const auto it = Data.find(achievementId);
    return (it != Data.end()) ? &it->second : nullptr;
}

const Event* GetEvent(AchievementEvent achievementEventId) {
    const auto it = EventData.find(achievementEventId);
    return (it != EventData.end()) ? &it->second : nullptr;
}

// ============================================================================
// Cleanup
// ============================================================================
#undef ACH_AI
#undef ACH_AC
#undef ACH_AE
#undef ACH_TEX
#undef ACH_ACHIEVEMENT_ENTRY
#undef ACH_EVENT_ENTRY

} // namespace StaticData

} // namespace Achievements