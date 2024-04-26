#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void RegisterPreventActorUpdateHooks();
void RegisterPreventActorDrawHooks();
void RegisterPreventActorInitHooks();
void InitDeveloperTools();

#ifdef __cplusplus
}
#endif