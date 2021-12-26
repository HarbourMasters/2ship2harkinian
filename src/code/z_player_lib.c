#include "global.h"

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_801226E0.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_80122744.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_80122760.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_80122868.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_801229A0.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_801229EC.s")

typedef struct {
    /* 0x00 */ Vec3f unk_00;
    /* 0x0C */ Vec3f unk_0C;
    /* 0x18 */ s16 unk_18;
    /* 0x1A */ s16 unk_1A;
} struct_801F58B0; // size = 0x1C

extern s16 D_801BFDA0[];
extern struct_801F58B0 D_801F58B0[3][3];
extern s32 D_801F59C8[2];

void func_80127B64(struct_801F58B0 arg0[], UNK_TYPE arg1, Vec3f* arg2);

void func_801229FC(Player* player) {
    if (player->maskObjectLoading == 1) {
        // TODO: check if player->maskId is unsigned
        s16 temp_s1 = D_801BFDA0[(u8)player->maskId - 1];

        osCreateMesgQueue(&player->maskObjectLoadQueue, &player->maskObjectLoadMsg, 1);
        DmaMgr_SendRequestImpl(&player->maskDmaRequest, player->maskObjectSegment, objectFileTable[temp_s1].vromStart,
                               objectFileTable[temp_s1].vromEnd - objectFileTable[temp_s1].vromStart, 0,
                               &player->maskObjectLoadQueue, NULL);
        player->maskObjectLoading += 1;
    } else if (player->maskObjectLoading == 2) {
        if (osRecvMesg(&player->maskObjectLoadQueue, NULL, 0) == 0) {
            player->maskObjectLoading = 0;

            if (player->currentMask == PLAYER_MASK_GREAT_FAIRY) {
                s32 i;

                for (i = 0; i < ARRAY_COUNT(D_801F58B0); i++) {
                    func_80127B64(D_801F58B0[i], ARRAY_COUNT(D_801F58B0[i]), &player->bodyPartsPos[7]);
                }
            }
        }
    } else if ((player->currentMask != 0) && (player->currentMask != (u8)player->maskId)) {
        player->maskObjectLoading = 1;
        player->maskId = player->currentMask;
    } else if (player->currentMask == 8) {
        s32 i;

        for (i = 0; i < ARRAY_COUNT(D_801F59C8); i++) {
            D_801F59C8[i] += Rand_S16Offset(4, 0x17) + (s32)(fabsf(player->linearVelocity) * 50.0f);
        }
    }
}

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_80122BA4.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_80122C20.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_80122D44.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_80122ED8.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_80122EEC.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_80122F28.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_80122F9C.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_80122FCC.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_8012300C.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_8012301C.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_80123140.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_80123358.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_801233E4.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_80123420.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_80123434.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_80123448.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_801234B0.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_801234D4.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_80123590.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_801235DC.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_8012364C.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_80123810.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_80123960.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_801239AC.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_80123AA4.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_80123BD4.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_80123C58.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_80123C90.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_80123D50.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_80123DA4.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_80123DC0.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_80123E90.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_80123F14.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_80123F2C.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_80123F48.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_80124020.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/Player_GetMask.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/Player_RemoveMask.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_8012405C.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_80124088.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_801240C8.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_801240DC.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_80124110.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_80124148.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_80124168.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_80124190.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_801241B4.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_801241E0.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_8012420C.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_8012422C.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_80124258.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_80124278.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_801242B4.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_801242DC.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_80124420.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_80124618.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_801246F4.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_80124870.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_80124CC4.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_80124F18.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_80124FF0.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_801251C4.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_80125318.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_80125340.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_8012536C.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_801253A4.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_80125500.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_80125580.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_80125CE0.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_80125D4C.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_801262C8.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_801263FC.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_80126440.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_801265C8.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_8012669C.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_80126808.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_8012697C.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_80126AB4.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_80126B8C.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_80126BD0.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_801271B0.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_80127438.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_80127488.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_8012754C.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_80127594.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_801278F8.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_80127A60.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_80127B64.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_80127BE8.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_80127DA4.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_80128388.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_801284A0.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_80128640.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_80128B74.s")

#pragma GLOBAL_ASM("asm/non_matchings/code/z_player_lib/func_80128BD0.s")
