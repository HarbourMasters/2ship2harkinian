#include "Enhancements.h"

void InitEnhancements() {
    // Cycle & Saving
    RegisterEndOfCycleSaveHooks();
    RegisterSavingEnhancements();
    RegisterAutosave();
    RegisterKeepExpressMail();
}
