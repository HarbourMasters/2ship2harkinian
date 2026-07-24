/* soh/.../CustomMessageManager.h shim → 2ship CustomMessage.
 * NOTE: SoH's CustomMessageManager (singleton + RegisterMessage) and 2ship's
 * CustomMessage (namespace funcs) differ; call sites that use the SoH API will
 * surface at build time and get adapted to the 2ship namespace functions. */
#ifndef NEI_SHIM_CUSTOMMESSAGEMANAGER_H
#define NEI_SHIM_CUSTOMMESSAGEMANAGER_H
#include "soh/_nei_compat_core.h"
#include "2s2h/CustomMessage/CustomMessage.h"
#endif
