set(CVAR_VSYNC_ENABLED "${CVAR_PREFIX_SETTING}.VsyncEnabled" CACHE STRING "")
set(CVAR_Z_FIGHTING_MODE "${CVAR_PREFIX_SETTING}.ZFightingMode" CACHE STRING "")
set(CVAR_DISABLE_CLOSE_COLOR_WRAP "${CVAR_PREFIX_SETTING}.DisableCloseColorWrap" CACHE STRING "")
set(CVAR_ALWAYS_EXECUTE_DL_BRANCHES "${CVAR_PREFIX_SETTING}.AlwaysExecuteDLBranches" CACHE STRING "")
set(CVAR_NEW_FILE_DROPPED "${CVAR_PREFIX_GENERAL}.NewFileDropped" CACHE STRING "")
set(CVAR_DROPPED_FILE "${CVAR_PREFIX_GENERAL}.DroppedFile" CACHE STRING "")
set(CVAR_INTERNAL_RESOLUTION "${CVAR_PREFIX_SETTING}.InternalResolution" CACHE STRING "")
set(CVAR_MSAA_VALUE "${CVAR_PREFIX_SETTING}.MSAAValue" CACHE STRING "")
set(CVAR_SDL_WINDOWED_FULLSCREEN "${CVAR_PREFIX_SETTING}.SdlWindowedFullscreen" CACHE STRING "")
set(CVAR_TEXTURE_FILTER "${CVAR_PREFIX_SETTING}.TextureFilter" CACHE STRING "")
set(CVAR_IMGUI_CONTROLLER_NAV "${CVAR_PREFIX_SETTING}.ControlNav" CACHE STRING "")
set(CVAR_CONSOLE_WINDOW_OPEN "${CVAR_PREFIX_WINDOW}.Console" CACHE STRING "")
set(CVAR_CONTROLLER_CONFIGURATION_WINDOW_OPEN "${CVAR_PREFIX_WINDOW}.ControllerConfiguration" CACHE STRING "")
set(CVAR_CONTROLLER_DISCONNECTED_WINDOW_OPEN "${CVAR_PREFIX_WINDOW}.ControllerDisconnected" CACHE STRING "")
set(CVAR_CONTROLLER_REORDERING_WINDOW_OPEN "${CVAR_PREFIX_WINDOW}.ControllerReordering" CACHE STRING "")
set(CVAR_GFX_DEBUGGER_WINDOW_OPEN "${CVAR_PREFIX_WINDOW}.GfxDebugger" CACHE STRING "")
set(CVAR_STATS_WINDOW_OPEN "${CVAR_PREFIX_WINDOW}.Stats" CACHE STRING "")
set(CVAR_ENABLE_MULTI_VIEWPORTS "${CVAR_PREFIX_SETTING}.EnableMultiViewports" CACHE STRING "")
set(CVAR_LOW_RES_MODE "${CVAR_PREFIX_SETTING}.LowResMode" CACHE STRING "")
set(CVAR_SIMULATED_INPUT_LAG "${CVAR_PREFIX_SETTING}.SimulatedInputLag" CACHE STRING "")
set(CVAR_ALT_ASSETS "${CVAR_PREFIX_ENHANCEMENT}.AltAssets" CACHE STRING "")
set(CVAR_GAME_OVERLAY_FONT "${CVAR_PREFIX_SETTING}.OverlayFont" CACHE STRING "")
set(CVAR_MENU_BAR_OPEN "${CVAR_PREFIX_SETTING}.OpenMenuBar" CACHE STRING "")
set(CVAR_PREFIX_CONTROLLERS "${CVAR_PREFIX_SETTING}.Controllers" CACHE STRING "")
set(CVAR_PREFIX_ADVANCED_RESOLUTION "${CVAR_PREFIX_SETTING}.AdvancedResolution" CACHE STRING "")
include("libultraship/cmake/cvars.cmake")
