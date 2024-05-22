#ifndef MOTION_BLUR_H
#define MOTION_BLUR_H

void MotionBlur_RenderMenuOptions();

#ifdef __cplusplus
#include <libultraship/libultraship.h>

extern "C" {
#endif

void MotionBlur_Override(u8* status, s32* alpha);

#ifdef __cplusplus
};
#endif

#endif // MOTION_BLUR_H
