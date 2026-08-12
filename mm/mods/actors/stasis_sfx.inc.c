/**
 * stasis_sfx.inc.c — the Stasis rune's own sound (Skijer's NEI). Included from stasis_rune.c.
 *
 * A self-contained one-voice PCM player. It exists because neither engine has a "play this buffer"
 * API a mod can reach: soh's MmDirectAudio is the MM-sound emulator (and its PCM entry point is
 * file-static), and 2ship has no equivalent at all. Both DO have the same audio-thread seam though
 * — the spot where Sm64Audio_MixInto and friends are mixed on top of the synth output — so this
 * hangs off that and stays identical in both repos.
 *
 * Threading: StasisSfx_Play/Stop run on the game thread and only ever write `sSfx.wantPlay` and the
 * parameters; the audio thread owns `pos` and reads the rest. One writer each, single voice, no
 * lock — the worst a race can do is start the cue one buffer early or late.
 */

#include "stasis_sfx_pcm.inc.c"

// Both engines render at 32 kHz stereo.
#define STASIS_SFX_OUT_RATE 32000.0f

typedef struct {
    volatile u8 wantPlay; // game thread raises, audio thread consumes
    volatile u8 playing;
    volatile f32 rate;   // 1.0 = as recorded; 2.0 = double speed (enemies)
    volatile f32 volume; // 0..1
    f32 pos;             // audio-thread only: sample cursor into sStasisSfxPcm
} StasisSfxState;

static StasisSfxState sSfx = { 0 };

// Start the cue. `rate` is a playback multiplier, so the enemy variant is literally 2.0f.
void StasisSfx_Play(f32 rate, f32 volume) {
    sSfx.rate = rate;
    sSfx.volume = volume;
    sSfx.wantPlay = 1;
}

void StasisSfx_Stop(void) {
    sSfx.wantPlay = 0;
    sSfx.playing = 0;
}

/**
 * Audio-thread mixer. `outBuf` is interleaved stereo s16, `numSamples` is the number of STEREO
 * FRAMES — the same contract Sm64Audio_MixInto is called with right next to this.
 */
void StasisSfx_MixInto(s16* outBuf, u32 numSamples) {
    f32 advance;
    f32 vol;
    u32 i;

    if (sSfx.wantPlay) {
        sSfx.wantPlay = 0;
        sSfx.playing = 1;
        sSfx.pos = 0.0f;
    }
    if (!sSfx.playing || (outBuf == NULL)) {
        return;
    }

    advance = (sSfx.rate * (f32)STASIS_SFX_RATE) / STASIS_SFX_OUT_RATE;
    vol = sSfx.volume;

    for (i = 0; i < numSamples; i++) {
        s32 idx = (s32)sSfx.pos;
        s32 mixL;
        s32 mixR;
        s32 s;

        if (idx >= (STASIS_SFX_SAMPLES - 1)) {
            sSfx.playing = 0;
            return;
        }

        // Linear interpolation between neighbouring samples: at 2x rate we skip every other one,
        // and without it the cue picks up an audible buzz.
        {
            f32 frac = sSfx.pos - (f32)idx;
            f32 a = (f32)sStasisSfxPcm[idx];
            f32 b = (f32)sStasisSfxPcm[idx + 1];

            s = (s32)((a + ((b - a) * frac)) * vol);
        }

        // Mixed on top of whatever the synth already wrote, clamped so a loud scene cannot wrap
        // around into noise.
        mixL = outBuf[(i * 2) + 0] + s;
        mixR = outBuf[(i * 2) + 1] + s;
        outBuf[(i * 2) + 0] = (s16)((mixL > 32767) ? 32767 : ((mixL < -32768) ? -32768 : mixL));
        outBuf[(i * 2) + 1] = (s16)((mixR > 32767) ? 32767 : ((mixR < -32768) ? -32768 : mixR));

        sSfx.pos += advance;
    }
}
