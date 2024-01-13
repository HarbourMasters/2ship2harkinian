#ifndef BenGui_hpp
#define BenGui_hpp

#include <stdio.h>
#include "BenMenuBar.h"
#include "DeveloperTools/SaveEditor.h"

namespace BenGui {
    void SetupHooks();
    void SetupGuiElements();
    void Draw();
    void Destroy();
}

#endif /* BenGui_hpp */
