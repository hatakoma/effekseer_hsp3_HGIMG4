#pragma once
#include <windows.h>
#include <EffekseerRendererGL/EffekseerRendererGL.GLExtension.h>
#include <stdexcept>
#include <vector>

namespace bridge_detail {
namespace G = EffekseerRendererGL::GLExt;
// A full guard also covers loading/initialization, where the renderer's draw-only
// restoration does not apply. It preserves HGIMG4's cached OpenGL bindings.
struct GLState {
    GLint active, program, vao, array, element, src, dst, srcA, dstA, eq, eqA, depthFunc, cull;
    GLint unpack;
    GLboolean blend, depth, face, depthWrite;
    struct Unit { GLint tex; GLint sampler; };
    std::vector<Unit> units;
    using BindSampler = void (APIENTRY*)(GLuint, GLuint);
    BindSampler bindSampler = nullptr;
#ifdef EFK_ANGLE
    struct Attribute { GLint enabled, size, type, normalized, stride, buffer; void* pointer; };
    std::vector<Attribute> attributes;
    using GetAttrib = void (APIENTRY*)(GLuint, GLenum, GLint*);
    using GetPointer = void (APIENTRY*)(GLuint, GLenum, void**);
#endif
    GLState() {
        glGetIntegerv(0x84E0, &active); // GL_ACTIVE_TEXTURE
        glGetIntegerv(GL_CURRENT_PROGRAM, &program);
        vao = 0;
        if (G::IsSupportedVertexArray()) glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &vao);
        glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &array);
        glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &element);
        glGetIntegerv(GL_BLEND_SRC_RGB, &src); glGetIntegerv(GL_BLEND_DST_RGB, &dst);
        glGetIntegerv(GL_BLEND_SRC_ALPHA, &srcA); glGetIntegerv(GL_BLEND_DST_ALPHA, &dstA);
        glGetIntegerv(GL_BLEND_EQUATION, &eq); glGetIntegerv(0x883D, &eqA);
        glGetIntegerv(GL_DEPTH_FUNC, &depthFunc); glGetIntegerv(GL_CULL_FACE_MODE, &cull);
        glGetIntegerv(GL_UNPACK_ALIGNMENT, &unpack);
        blend = glIsEnabled(GL_BLEND); depth = glIsEnabled(GL_DEPTH_TEST); face = glIsEnabled(GL_CULL_FACE);
        glGetBooleanv(GL_DEPTH_WRITEMASK, &depthWrite);
#ifndef EFK_ANGLE
        GLint major = 0, minor = 0;
        glGetIntegerv(0x821B, &major); glGetIntegerv(0x821C, &minor);
        if (major > 3 || (major == 3 && minor >= 3))
            bindSampler = reinterpret_cast<BindSampler>(wglGetProcAddress("glBindSampler"));
#else
        // ES2 has no core VAO. Save every attribute touched by Effekseer so
        // HGIMG4's cached bindings remain valid after drawing/loading.
        auto getAttrib = reinterpret_cast<GetAttrib>(efk_angle_proc("glGetVertexAttribiv"));
        auto getPointer = reinterpret_cast<GetPointer>(efk_angle_proc("glGetVertexAttribPointerv"));
        if (!getAttrib || !getPointer) throw std::runtime_error("ANGLE vertex attribute queries unavailable");
        GLint n = 0; glGetIntegerv(0x8869, &n); // MAX_VERTEX_ATTRIBS
        attributes.resize(n);
        for (GLuint i = 0; i < attributes.size(); ++i) {
            auto& a = attributes[i];
            getAttrib(i, 0x8622, &a.enabled); getAttrib(i, 0x8623, &a.size);
            getAttrib(i, 0x8625, &a.type); getAttrib(i, 0x886A, &a.normalized);
            getAttrib(i, 0x8624, &a.stride); getAttrib(i, 0x889F, &a.buffer);
            getPointer(i, 0x8645, &a.pointer);
        }
#endif
        GLint count = 0; glGetIntegerv(0x8872, &count); // fragment texture units
        units.resize(count);
        for (GLuint i = 0; i < units.size(); ++i) {
            G::glActiveTexture(GL_TEXTURE0 + i);
            glGetIntegerv(GL_TEXTURE_BINDING_2D, &units[i].tex);
            if (bindSampler) glGetIntegerv(0x8919, &units[i].sampler);
        }
        G::glActiveTexture(active);
    }
    ~GLState() {
        if (G::IsSupportedVertexArray()) G::glBindVertexArray(vao);
#ifdef EFK_ANGLE
        for (GLuint i = 0; i < attributes.size(); ++i) {
            const auto& a = attributes[i];
            G::glBindBuffer(GL_ARRAY_BUFFER, a.buffer);
            G::glVertexAttribPointer(i, a.size, a.type, static_cast<GLboolean>(a.normalized), a.stride, a.pointer);
            if (a.enabled) G::glEnableVertexAttribArray(i); else G::glDisableVertexAttribArray(i);
        }
#endif
        G::glBindBuffer(GL_ARRAY_BUFFER, array); G::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element);
        G::glUseProgram(program);
        for (GLuint i = 0; i < units.size(); ++i) {
            G::glActiveTexture(GL_TEXTURE0 + i); glBindTexture(GL_TEXTURE_2D, units[i].tex);
            if (bindSampler) bindSampler(i, units[i].sampler);
        }
        G::glActiveTexture(active);
        G::glBlendFuncSeparate(src, dst, srcA, dstA); G::glBlendEquationSeparate(eq, eqA);
        (blend ? glEnable : glDisable)(GL_BLEND);
        (depth ? glEnable : glDisable)(GL_DEPTH_TEST);
        (face ? glEnable : glDisable)(GL_CULL_FACE);
        glDepthFunc(depthFunc); glDepthMask(depthWrite); glCullFace(cull);
        glPixelStorei(GL_UNPACK_ALIGNMENT, unpack);
    }
};
}
