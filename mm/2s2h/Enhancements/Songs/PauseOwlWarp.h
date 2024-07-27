#ifndef PAUSE_OWL_WARP_H
#define PAUSE_OWL_WARP_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void RegisterPauseOwlWarp(void);
bool PauseOwlWarp_IsOwlWarpEnabled(void);

#ifdef __cplusplus
}
#endif

#endif // PAUSE_OWL_WARP_H