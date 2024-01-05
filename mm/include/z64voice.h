#ifndef Z64VOICE_H
#define Z64VOICE_H

#include "PR/ultratypes.h"
#include "unk.h"
#include "PR/os_voice.h"

typedef enum {
    /* 0  */ VOICE_WORD_ID_HOURS,
    /* 1  */ VOICE_WORD_ID_CHEESE,
    /* 2  */ VOICE_WORD_ID_WAKE_UP,
    /* 3  */ VOICE_WORD_ID_SIT,
    /* 4  */ VOICE_WORD_ID_MILK,
    /* 5  */ VOICE_WORD_ID_HIYA,
    /* 6  */ VOICE_WORD_ID_MAX,
    /* -1 */ VOICE_WORD_ID_NONE = 0xFFFF
} OSVoiceWordId;

void func_801A4EB0(void);
void func_801A4FD8(void);
void func_801A5080(u16 wordId);
u16 AudioVoice_GetWord(void);
void func_801A5118(void);
s32 func_801A51F0(s32 errorCode);
s32 func_801A5228(OSVoiceDictionary* dict);
OSVoiceData* func_801A5390(void);
void func_801A53E8(u16 distance, u16 answerNum, u16 warning, u16 voiceLevel, u16 voiceRelLevel);
u8* func_801A54C4(void);
s32 func_801A54D0(u16 wordId);
s32 func_801A5680(u16 wordId);
s32 func_801A5808(void);
void AudioVoice_ResetData(void);

#endif
