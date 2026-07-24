/* soh/resource/type/Text.h shim → 2ship's MM text resource (2s2h/resource/type/TextMM.h).
 * 2ship does not ship a generic LUS "Text" resource type; its message resource is
 * SOH::TextMM (entries are SOH::MessageEntryMM). NEI message code that included
 * SoH's soh/resource/type/Text.h now lands here. */
#ifndef NEI_SHIM_RES_TEXT_H
#define NEI_SHIM_RES_TEXT_H
#include "soh/_nei_compat_core.h"
#include "2s2h/resource/type/TextMM.h"
#endif
