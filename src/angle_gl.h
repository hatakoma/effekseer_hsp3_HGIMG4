#pragma once
// Force-included only into the ANGLE renderer/bridge. Reuse the Windows GL
// ABI declarations, but provide ES2 dispatch functions instead of opengl32 imports.
#include <windows.h>
#undef WINGDIAPI
#define WINGDIAPI
#include <GL/gl.h>
FARPROC efk_angle_proc(const char* name);
void* efk_angle_context();
