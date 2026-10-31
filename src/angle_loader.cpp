#include "angle_gl.h"
#include <cstring>
#include <stdexcept>

void* efk_angle_context() {
    auto module = GetModuleHandleW(L"libEGL.dll");
    if (!module) return nullptr;
    auto get = reinterpret_cast<void* (WINAPI*)()>(GetProcAddress(module, "eglGetCurrentContext"));
    return get ? get() : nullptr;
}

FARPROC efk_angle_proc(const char* name) {
    // Never load another copy of ANGLE: use the instance already owned by HGIMG4DX.
    auto gles = GetModuleHandleW(L"libGLESv2.dll");
    auto egl = GetModuleHandleW(L"libEGL.dll");
    if (!gles || !egl) return nullptr;
    // These entry points may be exported even though an ES2 context cannot use
    // them. The renderer must not infer ES3 support from GetProcAddress alone.
    const char* unsupported[] = {
        "glGenVertexArrays", "glDeleteVertexArrays", "glBindVertexArray",
        "glGenSamplers", "glDeleteSamplers", "glSamplerParameteri", "glBindSampler",
        "glMapBuffer", "glMapBufferRange", "glUnmapBuffer", "glDrawElementsInstanced",
        "glDrawBuffers", "glTexImage3D", "glCopyTexSubImage3D", "glGetStringi"
    };
    for (const auto* entry : unsupported) if (std::strcmp(name, entry) == 0) return nullptr;
    auto result = GetProcAddress(gles, name);
    if (result) return result;
    auto get = reinterpret_cast<FARPROC (WINAPI*)(const char*)>(GetProcAddress(egl, "eglGetProcAddress"));
    return get ? get(name) : nullptr;
}
