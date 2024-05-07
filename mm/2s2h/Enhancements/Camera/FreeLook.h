#ifndef FREE_LOOK_H
#define FREE_LOOK_H

void RegisterCameraFreeLook();

#ifdef __cplusplus
extern "C" {
#endif
#include <z64camera.h>
bool Camera_FreeLook(Camera* camera);
#ifdef __cplusplus
}
#endif

#endif // FREE_LOOK_H
