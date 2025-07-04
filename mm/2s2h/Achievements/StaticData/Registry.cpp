#include "Registry.h"

extern "C" {
#include "variables.h"
#include "z64item.h"
}

// Include bomber's notebook photo textures
#include "assets/archives/schedule_dma_static/schedule_dma_static_yar.h"

namespace Achievements {

namespace StaticData {

// Short macros for enum values
#define AI(id) AchievementId::id
#define AC(category) AchievementCategory::category
#define AE(event) AchievementEvent::event
#define TEX(path) (const char*)path

// Macro to reduce boilerplate in achievement definitions
#define ACHIEVEMENT_ENTRY(id, name, description, iconPath, secret, category, gamerscore, ...) \
    {                                                                                         \
        id, {                                                                                 \
            id, name, description, iconPath, secret, category, gamerscore, {                  \
                __VA_ARGS__                                                                   \
            }                                                                                 \
        }                                                                                     \
    }

// Macro to reduce boilerplate in event definitions
#define EVENT_ENTRY(id, name, description) \
    {                                      \
        id, {                              \
            id, name, description, {       \
            }                              \
        }                                  \
    }

// Static achievement data map
std::map<AchievementId, Achievement> Data;

// Static event data map
std::map<AchievementEvent, Event> EventData;

void Init() {
    // clang-format off
    Data = {
        // Bomber's Notebook achievements
        ACHIEVEMENT_ENTRY(AI(BN_HONORARY_MEMBER),
            "Honorary Member",
            "Fully complete the entry for the Bomber's Gang",
            TEX(gBombersNotebookPhotoBombersTex),
            false,
            AC(VANILLA),
            5,
            AE(EVENT_LEARNED_SECRET_CODE),
            AE(EVENT_RECEIVED_BOMBERS_NOTEBOOK)
        ),

        ACHIEVEMENT_ENTRY(AI(BN_FAITHFUL_BRIDE),
            "Faithful Bride",
            "Fully complete the entry for Anju",
            TEX(gBombersNotebookPhotoAnjuTex),
            false,
            AC(VANILLA),
            5,
            AE(EVENT_RECEIVED_ROOM_KEY),
            AE(EVENT_PROMISED_MIDNIGHT_MEETING),
            AE(EVENT_RECEIVED_LETTER_TO_KAFEI),
            AE(EVENT_DELIVERED_PENDANT_OF_MEMORIES),
            AE(EVENT_RECEIVED_COUPLES_MASK)
        ),

        ACHIEVEMENT_ENTRY(AI(BN_MINI_HE),
            "Mini-He",
            "Fully complete the entry for Kafei",
            TEX(gBombersNotebookPhotoKafeiTex),
            false,
            AC(VANILLA),
            5,
            AE(EVENT_PROMISED_TO_MEET_KAFEI),
            AE(EVENT_RECEIVED_PENDANT_OF_MEMORIES),
            AE(EVENT_ESCAPED_SAKONS_HIDEOUT),
            AE(EVENT_RECEIVED_COUPLES_MASK)
        ),

        ACHIEVEMENT_ENTRY(AI(BN_AMBIGUOUS_ALLEGIANCE),
            "Ambiguous Allegiance",
            "Fully complete the entry for the Curiosity Shop Owner",
            TEX(gBombersNotebookPhotoCuriosityShopManTex),
            false,
            AC(VANILLA),
            5,
            AE(EVENT_RECEIVED_KEATON_MASK),
            AE(EVENT_RECEIVED_PRIORITY_MAIL),
            AE(EVENT_RECEIVED_ALL_NIGHT_MASK)
        ),

        ACHIEVEMENT_ENTRY(AI(BN_FRAIL_DID),
            "Frail D.I.D.",
            "Fully complete the entry for the Bomb Shop Owner's Mother",
            TEX(gBombersNotebookPhotoBombShopLadyTex),
            false,
            AC(VANILLA),
            5,
            AE(EVENT_RECEIVED_BLAST_MASK)
        ),

        ACHIEVEMENT_ENTRY(AI(BN_BRAVE_LITTLE_ARCHER),
            "Brave Little Archer",
            "Fully complete the entry for Romani",
            TEX(gBombersNotebookPhotoRomaniTex),
            false,
            AC(VANILLA),
            5,
            AE(EVENT_PROMISED_TO_HELP_WITH_THEM),
            AE(EVENT_DEFENDED_AGAINST_THEM),
            AE(EVENT_RECEIVED_MILK_BOTTLE)
        ),

        ACHIEVEMENT_ENTRY(AI(BN_MATURE_RANCH_HAND),
            "Mature Ranch Hand",
            "Fully complete the entry for Cremia",
            TEX(gBombersNotebookPhotoCremiaTex),
            false,
            AC(VANILLA),
            5,
            AE(EVENT_ESCORTED_CREMIA),
            AE(EVENT_RECEIVED_ROMANIS_MASK)
        ),

        ACHIEVEMENT_ENTRY(AI(BN_DROWNING_IN_RESPONSIBILITIES),
            "Drowning in Responsibilities",
            "Fully complete the entry for Mayor Doutour",
            TEX(gBombersNotebookPhotoMayorDotourTex),
            false,
            AC(VANILLA),
            5,
            AE(EVENT_RECEIVED_MAYOR_HP)
        ),

        ACHIEVEMENT_ENTRY(AI(BN_CONCERNED_MOTHER),
            "Concerned Mother",
            "Fully complete the entry for Madame Aroma",
            TEX(gBombersNotebookPhotoMadameAromaTex),
            false,
            AC(VANILLA),
            5,
            AE(EVENT_RECEIVED_KAFEIS_MASK),
            AE(EVENT_DELIVERED_PRIORITY_MAIL)
        ),

        ACHIEVEMENT_ENTRY(AI(BN_LOOKING_FOR_VOLUNTEERS),
            "Looking for Volunteers",
            "Fully complete the entry for Toto",
            TEX(gBombersNotebookPhotoTotoTex),
            false,
            AC(VANILLA),
            5,
            AE(EVENT_RECEIVED_CIRCUS_LEADERS_MASK)
        ),

        ACHIEVEMENT_ENTRY(AI(BN_BREAKAWAY_BROTHER),
            "Breakaway Brother",
            "Fully complete the entry for Gorman",
            TEX(gBombersNotebookPhotoGormanTex),
            false,
            AC(VANILLA),
            5,
            AE(EVENT_MET_GORMAN)
        ),

        ACHIEVEMENT_ENTRY(AI(BN_ALWAYS_MAKES_HIS_APPOINTED_DELIVERIES),
            "Always Makes His Appointed Deliveries",
            "Fully complete the entry for the Postman",
            TEX(gBombersNotebookPhotoPostmanTex),
            false,
            AC(VANILLA),
            5,
            AE(EVENT_RECEIVED_POSTMAN_HP),
            AE(EVENT_RECEIVED_POSTMANS_HAT)
        ),

        ACHIEVEMENT_ENTRY(AI(BN_PREPARE_FOR_TROUBLE),
            "Prepare For Trouble, and Make it Double!",
            "Fully complete the entry for the Rosa Sisters",
            TEX(gBombersNotebookPhotoRosaSistersTex),
            false,
            AC(VANILLA),
            5,
            AE(EVENT_RECEIVED_ROSA_SISTERS_HP)
        ),

        ACHIEVEMENT_ENTRY(AI(BN_YOU_MISSED_A_SPOT),
            "You Missed a Spot",
            "Fully complete the entry for ???",
            TEX(gBombersNotebookPhotoToiletHandTex),
            false,
            AC(VANILLA),
            5,
            AE(EVENT_RECEIVED_TOILET_HAND_HP)
        ),

        ACHIEVEMENT_ENTRY(AI(BN_STORYTELLER),
            "Storyteller",
            "Fully complete the entry for Anju's Grandmother",
            TEX(gBombersNotebookPhotoAnjusGrandmotherTex),
            false,
            AC(VANILLA),
            5,
            AE(EVENT_RECEIVED_GRANDMA_SHORT_STORY_HP),
            AE(EVENT_RECEIVED_GRANDMA_LONG_STORY_HP)
        ),

        ACHIEVEMENT_ENTRY(AI(BN_PASSING_ON_THE_GROOVE),
            "Passing on the Groove",
            "Fully complete the entry for Kamaro",
            TEX(gBombersNotebookPhotoKamaroTex),
            false,
            AC(VANILLA),
            5,
            AE(EVENT_RECEIVED_KAMAROS_MASK)
        ),

        ACHIEVEMENT_ENTRY(AI(BN_CARETAKER_OF_THE_FOWL),
            "Caretaker of the Fowl",
            "Fully complete the entry for Grog",
            TEX(gBombersNotebookPhotoGrogTex),
            false,
            AC(VANILLA),
            5,
            AE(EVENT_RECEIVED_BUNNY_HOOD)
        ),

        ACHIEVEMENT_ENTRY(AI(BN_THERES_THREE_OF_YOU),
            "There's THREE of you?!",
            "Fully complete the entry for the Gorman Brothers",
            TEX(gBombersNotebookPhotoGormanBrothersTex),
            false,
            AC(VANILLA),
            5,
            AE(EVENT_RECEIVED_GAROS_MASK)
        ),

        ACHIEVEMENT_ENTRY(AI(BN_PERFECTING_THE_ART),
            "Perfecting the Art of Standing Perfectly Still",
            "Fully complete the entry for Shiro",
            TEX(gBombersNotebookPhotoShiroTex),
            false,
            AC(VANILLA),
            5,
            AE(EVENT_RECEIVED_STONE_MASK)
        ),

        ACHIEVEMENT_ENTRY(AI(BN_WINDMILL_MAN),
            "Windmill Man",
            "Fully complete the entry for Guru-Guru",
            TEX(gBombersNotebookPhotoGuruGuruTex),
            false,
            AC(VANILLA),
            5,
            AE(EVENT_RECEIVED_BREMEN_MASK)
        ),

        // Meta-achievement for all Bomber's Notebook completion
        ACHIEVEMENT_ENTRY(AI(BN_LOCAL_HERO),
            "Local Hero",
            "100% the Bomber's Notebook",
            TEX(gBombersNotebookEntryIconRibbonTex),
            true,
            AC(VANILLA),
            25,
            AE(EVENT_LEARNED_SECRET_CODE),
            AE(EVENT_RECEIVED_BOMBERS_NOTEBOOK),
            AE(EVENT_RECEIVED_COUPLES_MASK),
            AE(EVENT_RECEIVED_KEATON_MASK),
            AE(EVENT_RECEIVED_ALL_NIGHT_MASK),
            AE(EVENT_RECEIVED_BLAST_MASK),
            AE(EVENT_RECEIVED_MILK_BOTTLE),
            AE(EVENT_RECEIVED_ROMANIS_MASK),
            AE(EVENT_RECEIVED_MAYOR_HP),
            AE(EVENT_RECEIVED_KAFEIS_MASK),
            AE(EVENT_RECEIVED_CIRCUS_LEADERS_MASK),
            AE(EVENT_MET_GORMAN),
            AE(EVENT_RECEIVED_POSTMANS_HAT),
            AE(EVENT_RECEIVED_ROSA_SISTERS_HP),
            AE(EVENT_RECEIVED_TOILET_HAND_HP),
            AE(EVENT_RECEIVED_GRANDMA_LONG_STORY_HP),
            AE(EVENT_RECEIVED_KAMAROS_MASK),
            AE(EVENT_RECEIVED_BUNNY_HOOD),
            AE(EVENT_RECEIVED_GAROS_MASK),
            AE(EVENT_RECEIVED_STONE_MASK),
            AE(EVENT_RECEIVED_BREMEN_MASK)
        ),

        // General achievements
        ACHIEVEMENT_ENTRY(AI(TIMELORD),
            "Timelord",
            "Play the Song of Double Time and Reverse Song of Time.",
            TEX(gItemIcons[ITEM_OCARINA_OF_TIME]),
            false,
            AC(GENERAL),
            20,
            AE(EVENT_PLAYED_SONG_OF_DOUBLE_TIME),
            AE(EVENT_PLAYED_INVERTED_SONG_OF_TIME)
        )
    };
    // clang-format on

    // Event data initialization
    EventData = {
        // Bomber's Notebook events
        EVENT_ENTRY(AE(EVENT_LEARNED_SECRET_CODE), "Learned Secret Code",
                    "Player discovered the Bomber's hideout password"),
        EVENT_ENTRY(AE(EVENT_RECEIVED_BOMBERS_NOTEBOOK), "Received Bomber's Notebook",
                    "Player received the Bomber's Notebook from Jim"),

        // Faithful Bride events
        EVENT_ENTRY(AE(EVENT_RECEIVED_ROOM_KEY), "Received Room Key", "Player received the room key from Anju"),
        EVENT_ENTRY(AE(EVENT_PROMISED_MIDNIGHT_MEETING), "Promised Midnight Meeting",
                    "Player promised to meet Anju at midnight"),
        EVENT_ENTRY(AE(EVENT_RECEIVED_LETTER_TO_KAFEI), "Received Letter to Kafei",
                    "Player received Anju's letter to Kafei"),
        EVENT_ENTRY(AE(EVENT_DELIVERED_PENDANT_OF_MEMORIES), "Delivered Pendant of Memories",
                    "Player delivered the Pendant of Memories"),
        EVENT_ENTRY(AE(EVENT_RECEIVED_COUPLES_MASK), "Received Couple's Mask",
                    "Player received the Couple's Mask from the reunited couple"),

        // Mini-He events
        EVENT_ENTRY(AE(EVENT_PROMISED_TO_MEET_KAFEI), "Promised to Meet Kafei", "Player promised to meet with Kafei"),
        EVENT_ENTRY(AE(EVENT_ESCAPED_SAKONS_HIDEOUT), "Escaped Sakon's Hideout",
                    "Player successfully escaped from Sakon's hideout"),

        // Ambiguous Allegiance events
        EVENT_ENTRY(AE(EVENT_RECEIVED_KEATON_MASK), "Received Keaton Mask",
                    "Player received the Keaton Mask from the Curiosity Shop Owner"),
        EVENT_ENTRY(AE(EVENT_RECEIVED_PRIORITY_MAIL), "Received Priority Mail", "Player received the Priority Mail"),
        EVENT_ENTRY(AE(EVENT_RECEIVED_ALL_NIGHT_MASK), "Received All-Night Mask", "Player received the All-Night Mask"),

        // Frail D.I.D. event
        EVENT_ENTRY(AE(EVENT_RECEIVED_BLAST_MASK), "Received Blast Mask",
                    "Player received the Blast Mask from the Bomb Shop Owner's Mother"),

        // Brave Little Archer events
        EVENT_ENTRY(AE(EVENT_PROMISED_TO_HELP_WITH_THEM), "Promised to Help with Them",
                    "Player promised Romani to help defend against 'them'"),
        EVENT_ENTRY(AE(EVENT_DEFENDED_AGAINST_THEM), "Defended Against Them",
                    "Player successfully defended the ranch from the alien invasion"),
        EVENT_ENTRY(AE(EVENT_RECEIVED_MILK_BOTTLE), "Received Milk Bottle",
                    "Player received a bottle of Romani's milk"),

        // Mature Ranch Hand events
        EVENT_ENTRY(AE(EVENT_ESCORTED_CREMIA), "Escorted Cremia", "Player escorted Cremia on the milk run"),
        EVENT_ENTRY(AE(EVENT_RECEIVED_ROMANIS_MASK), "Received Romani's Mask",
                    "Player received Romani's Mask from Cremia"),

        // Single-event achievements
        EVENT_ENTRY(AE(EVENT_RECEIVED_MAYOR_HP), "Received Mayor Heart Piece",
                    "Player received a Heart Piece from Mayor Dotour"),
        EVENT_ENTRY(AE(EVENT_DELIVERED_PRIORITY_MAIL), "Delivered Priority Mail",
                    "Player delivered the Priority Mail to Madame Aroma"),
        EVENT_ENTRY(AE(EVENT_RECEIVED_CIRCUS_LEADERS_MASK), "Received Circus Leader's Mask",
                    "Player received the Circus Leader's Mask from Toto"),
        EVENT_ENTRY(AE(EVENT_MET_GORMAN), "Met Gorman", "Player met Gorman in the Milk Bar"),
        EVENT_ENTRY(AE(EVENT_RECEIVED_POSTMAN_HP), "Received Postman Heart Piece",
                    "Player received a Heart Piece from the Postman"),
        EVENT_ENTRY(AE(EVENT_RECEIVED_POSTMANS_HAT), "Received Postman's Hat", "Player received the Postman's Hat"),
        EVENT_ENTRY(AE(EVENT_RECEIVED_ROSA_SISTERS_HP), "Received Rosa Sisters Heart Piece",
                    "Player received a Heart Piece from the Rosa Sisters"),
        EVENT_ENTRY(AE(EVENT_RECEIVED_TOILET_HAND_HP), "Received Toilet Hand Heart Piece",
                    "Player received a Heart Piece from the toilet hand"),
        EVENT_ENTRY(AE(EVENT_RECEIVED_GRANDMA_SHORT_STORY_HP), "Received Grandma Short Story Heart Piece",
                    "Player received a Heart Piece for listening to grandma's short story"),
        EVENT_ENTRY(AE(EVENT_RECEIVED_GRANDMA_LONG_STORY_HP), "Received Grandma Long Story Heart Piece",
                    "Player received a Heart Piece for listening to grandma's long story"),
        EVENT_ENTRY(AE(EVENT_RECEIVED_KAMAROS_MASK), "Received Kamaro's Mask",
                    "Player received Kamaro's Mask from the dancing ghost"),
        EVENT_ENTRY(AE(EVENT_RECEIVED_BUNNY_HOOD), "Received Bunny Hood", "Player received the Bunny Hood from Grog"),
        EVENT_ENTRY(AE(EVENT_RECEIVED_GAROS_MASK), "Received Garo's Mask",
                    "Player received the Garo's Mask from the Gorman Brothers"),
        EVENT_ENTRY(AE(EVENT_RECEIVED_STONE_MASK), "Received Stone Mask", "Player received the Stone Mask from Shiro"),
        EVENT_ENTRY(AE(EVENT_RECEIVED_BREMEN_MASK), "Received Bremen Mask",
                    "Player received the Bremen Mask from Guru-Guru"),
        EVENT_ENTRY(AE(EVENT_RECEIVED_KAFEIS_MASK), "Received Kafei's Mask",
                    "Player received Kafei's Mask from Madame Aroma"),

        // Time manipulation events
        EVENT_ENTRY(AE(EVENT_PLAYED_SONG_OF_DOUBLE_TIME), "Played Song of Double Time",
                    "Player used the Song of Double Time to speed up time"),
        EVENT_ENTRY(AE(EVENT_PLAYED_INVERTED_SONG_OF_TIME), "Played Inverted Song of Time",
                    "Player used the Inverted Song of Time to slow down time")
    };

    // Populate reverse relationships
    for (auto& [eventId, event] : EventData) {
        for (const auto& [achievementId, achievement] : Data) {
            for (AchievementEvent requiredEvent : achievement.requiredEvents) {
                if (requiredEvent == eventId) {
                    event.dependentAchievements.push_back(achievement.id);
                    break; // Achievement can only depend on an event once
                }
            }
        }
    }
}

const Achievement* GetAchievement(AchievementId id) {
    auto it = Data.find(id);
    return (it != Data.end()) ? &it->second : nullptr;
}

const Event* GetEvent(AchievementEvent eventId) {
    auto it = EventData.find(eventId);
    return (it != EventData.end()) ? &it->second : nullptr;
}

#undef AI
#undef AC
#undef AE
#undef TEX
#undef ACHIEVEMENT_ENTRY
#undef EVENT_ENTRY

} // namespace StaticData

} // namespace Achievements