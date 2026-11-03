#include <windows.h>
#include <Effekseer.h>
#include <Effekseer/Effekseer.EffectNode.h>
#include "collision_world.h"
#include "gl_state.h"
#include "effekseer_hgimg4.h"
#include <EffekseerRendererGL.h>
#include <EffekseerRendererGL/EffekseerRendererGL.GLExtension.h>
#include <cmath>
#include <cstring>
#include <climits>
#include <cstdlib>
#include <map>
#include <stdexcept>
#include <string>

namespace E = Effekseer;
namespace G = EffekseerRendererGL::GLExt;
using bridge_detail::GLState;
namespace {
E::ManagerRef manager;
EffekseerRendererGL::RendererRef renderer;
std::map<int, E::EffectRef> effects;
std::map<int, E::Handle> instances;
collision::World collisionWorld;
void* context = nullptr;
#ifdef EFK_ANGLE
void* currentContext() { return efk_angle_context(); }
#else
void* currentContext() { return wglGetCurrentContext(); }
#endif
DWORD owner = 0;
int nextId = 1;
double elapsed = 0;
thread_local char errorText[1024] = "";

void require(bool ok, const char* message) { if (!ok) throw std::runtime_error(message); }
float scalar(double n) {
    require(std::isfinite(n) && std::abs(n) <= 1.0e12, "Invalid numeric argument");
    return static_cast<float>(n);
}
void ready() {
    require(manager && renderer, "Call efk_init first");
    require(GetCurrentThreadId() == owner && currentContext() == context,
        "Use the initializing HGIMG4 OpenGL context and thread");
}
int newId() { require(nextId < INT_MAX, "ID space exhausted"); return nextId++; }
E::Handle handle(int id) {
    auto it = instances.find(id);
    require(it != instances.end() && manager->Exists(it->second), "Invalid or finished playback ID");
    return it->second;
}
template<class F> int guarded(F f) noexcept {
    try { errorText[0] = 0; return f(); }
    catch (const std::exception& e) { strncpy_s(errorText, e.what(), _TRUNCATE); }
    catch (...) { strcpy_s(errorText, "Unexpected native exception"); }
    return -1;
}

template<class F> int withReady(F f) noexcept {
    return guarded([&] { ready(); return f(); });
}
E::EffectRef effect(int id) {
    auto it = effects.find(id);
    require(it != effects.end(), "Invalid effect ID");
    return it->second;
}
void reset() {
    instances.clear(); effects.clear(); manager.Reset(); renderer.Reset();
    collisionWorld.clear();
    context = nullptr; owner = 0; elapsed = 0;
}
}

#define API extern "C" int __stdcall
API efk_init(int capacity) { return guarded([&] {
    require(!manager && !renderer, "Already initialized; call efk_shutdown first");
    require(capacity >= 1 && capacity <= 65536, "Capacity must be 1..65536");
#ifdef EFK_ANGLE
    require(currentContext() != nullptr, "No ANGLE EGL context: use hgimg4dx.as / hsp3gpdx.exe");
    const char* version = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    require(version && std::strstr(version, "OpenGL ES"), "Expected an OpenGL ES context");
    auto deviceType = EffekseerRendererGL::OpenGLDeviceType::OpenGLES2;
#else
    require(currentContext() != nullptr, "No OpenGL context: use hgimg4.as / hsp3gp.exe, not hgimg4dx.as");
    const char* version = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    require(version && std::atof(version) >= 3.1, "OpenGL 3.1 or later required");
    // HGIMG4 explicitly requests a 3.1 context. Use Effekseer's GLSL 1.20
    // renderer there; its OpenGL3 backend requires GLSL 3.30.
    auto deviceType = std::atof(version) >= 3.3 ? EffekseerRendererGL::OpenGLDeviceType::OpenGL3
                                              : EffekseerRendererGL::OpenGLDeviceType::OpenGL2;
#endif
    require(G::Initialize(deviceType, true), "OpenGL extension initialization failed");
    GLState state;
    try {
        renderer = EffekseerRendererGL::Renderer::Create(capacity, deviceType);
        require(renderer != nullptr, "Effekseer renderer creation failed");
        manager = E::Manager::Create(capacity);
        require(manager != nullptr, "Effekseer manager creation failed");
        manager->SetCoordinateSystem(E::CoordinateSystem::RH);
        manager->SetCollisionCallback([](const E::Vector3D& start, const E::Vector3D& end,
                                        E::Vector3D& point, E::Vector3D& normal) {
            const auto hit=collisionWorld.cast({start.X,start.Y,start.Z},{end.X,end.Y,end.Z});
            if(!hit.id) return false;
            point={float(hit.point.x),float(hit.point.y),float(hit.point.z)};
            normal={float(hit.normal.x),float(hit.normal.y),float(hit.normal.z)};
            collisionWorld.record(hit.id);
            return true;
        });
        manager->SetSpriteRenderer(renderer->CreateSpriteRenderer());
        manager->SetRibbonRenderer(renderer->CreateRibbonRenderer());
        manager->SetRingRenderer(renderer->CreateRingRenderer());
        manager->SetTrackRenderer(renderer->CreateTrackRenderer());
        manager->SetModelRenderer(renderer->CreateModelRenderer());
        manager->SetTextureLoader(renderer->CreateTextureLoader());
        manager->SetModelLoader(renderer->CreateModelLoader());
        manager->SetMaterialLoader(renderer->CreateMaterialLoader());
        manager->SetCurveLoader(E::MakeRefPtr<E::CurveLoader>());
        renderer->SetRestorationOfStatesFlag(true);
        E::Matrix44 view, projection;
        view.LookAtRH({0, 0, 20}, {0, 0, 0}, {0, 1, 0});
        projection.PerspectiveFovRH_OpenGL(0.785398163f, 4.0f / 3.0f, 0.1f, 1000.0f);
        renderer->SetCameraMatrix(view); renderer->SetProjectionMatrix(projection);
        E::Manager::LayerParameter layer; layer.ViewerPosition = {0, 0, 20};
        manager->SetLayerParameter(0, layer);
        context = currentContext(); owner = GetCurrentThreadId();
    } catch (...) { reset(); throw; }
    return 0;
}); }
API efk_shutdown() { return guarded([] {
    if (!manager && !renderer) return 0;
    ready(); GLState state; reset(); return 0;
}); }
API efk_load(const char* path, double scale) { return withReady([&] {
    require(path && *path, "Empty effect path");
    float s = scalar(scale); require(s > 0, "Scale must be positive");
    int size = MultiByteToWideChar(CP_ACP, MB_ERR_INVALID_CHARS, path, -1, nullptr, 0);
    require(size > 0, "Invalid ANSI effect path");
    std::wstring wide(size, 0);
    MultiByteToWideChar(CP_ACP, MB_ERR_INVALID_CHARS, path, -1, wide.data(), size);
    GLState state;
    auto effect = E::Effect::Create(manager, reinterpret_cast<const char16_t*>(wide.c_str()), s);
    require(effect != nullptr, "Could not load effect; check path and .efkefc/.efk format");
    int id = newId(); effects.emplace(id, effect); return id;
}); }
API efk_release(int id) { return withReady([&] {
    effect(id);
    GLState state; effects.erase(id); return 0;
}); }
API efk_play(int id, double x, double y, double z) { return withReady([&] {
    auto resource = effect(id);
    float fx = scalar(x), fy = scalar(y), fz = scalar(z);
    int result = newId();
    auto h = manager->Play(resource, fx, fy, fz);
    require(h >= 0, "Playback failed");
    try { instances.emplace(result, h); } catch (...) { manager->StopEffect(h); throw; }
    return result;
}); }
API efk_stop(int id) { return withReady([&] { manager->StopEffect(handle(id)); instances.erase(id); return 0; }); }
API efk_stop_all() { return withReady([] { manager->StopAllEffects(); instances.clear(); return 0; }); }
API efk_exists(int id) { return withReady([&] {
    auto it = instances.find(id);
    return it != instances.end() && manager->Exists(it->second) ? 1 : 0;
}); }
API efk_location(int id, double x, double y, double z) { return withReady([&] {
    manager->SetLocation(handle(id), scalar(x), scalar(y), scalar(z)); return 0;
}); }
API efk_rotation(int id, double x, double y, double z) { return withReady([&] {
    manager->SetRotation(handle(id), scalar(x), scalar(y), scalar(z)); return 0;
}); }
API efk_scale(int id, double x, double y, double z) { return withReady([&] {
    manager->SetScale(handle(id), scalar(x), scalar(y), scalar(z)); return 0;
}); }
API efk_speed(int id, double speed) { return withReady([&] {
    float s = scalar(speed); require(s >= 0, "Speed cannot be negative");
    manager->SetSpeed(handle(id), s); return 0;
}); }
API efk_pause(int id, int paused) { return withReady([&] { manager->SetPaused(handle(id), paused != 0); return 0; }); }
API efk_color(int id, int r, int g, int b, int a) { return withReady([&] {
    require(r >= 0 && r <= 255 && g >= 0 && g <= 255 && b >= 0 && b <= 255 && a >= 0 && a <= 255, "Color must be 0..255");
    manager->SetAllColor(handle(id), E::Color(uint8_t(r), uint8_t(g), uint8_t(b), uint8_t(a))); return 0;
}); }
API efk_update(double frames) { return withReady([&] {
    float delta = scalar(frames); require(delta >= 0 && delta <= 600, "Delta must be 0..600 frames (60 Hz)");
    GLState state; collisionWorld.beginUpdate(); manager->Update(delta); elapsed += frames / 60.0;
    renderer->SetTime(static_cast<float>(elapsed));
    for (auto it = instances.begin(); it != instances.end();) {
        if (!manager->Exists(it->second)) it = instances.erase(it); else ++it;
    }
    return 0;
}); }
API efk_camera(double x, double y, double z, double tx, double ty, double tz) { return withReady([&] {
    E::Vector3D eye(scalar(x), scalar(y), scalar(z)), target(scalar(tx), scalar(ty), scalar(tz));
    require(std::abs(x - tx) + std::abs(z - tz) > 1e-6, "Camera direction must not be parallel to Y up");
    E::Matrix44 view; view.LookAtRH(eye, target, {0, 1, 0}); renderer->SetCameraMatrix(view);
    E::Manager::LayerParameter layer; layer.ViewerPosition = eye; manager->SetLayerParameter(0, layer); return 0;
}); }
API efk_perspective(double degrees, double aspect, double nearZ, double farZ) { return withReady([&] {
    float f = scalar(degrees), a = scalar(aspect), n = scalar(nearZ), z = scalar(farZ);
    require(f > 0 && f < 179 && a > 0 && n > 0 && z > n, "Invalid perspective parameters");
    E::Matrix44 p; p.PerspectiveFovRH_OpenGL(f * 0.0174532925199433f, a, n, z); renderer->SetProjectionMatrix(p); return 0;
}); }
// HSP arrays contain doubles. Matrices use Effekseer's row-vector order, 16 elements each.
API efk_matrices(const double* view, const double* projection) { return withReady([&] {
    require(view && projection, "Null matrix pointer"); E::Matrix44 v, p;
    for (int i = 0; i < 16; ++i) { v.Values[i / 4][i % 4] = scalar(view[i]); p.Values[i / 4][i % 4] = scalar(projection[i]); }
    renderer->SetCameraMatrix(v); renderer->SetProjectionMatrix(p); return 0;
}); }
API efk_draw() { return withReady([] {
    GLState state;
    require(renderer->BeginRendering(), "BeginRendering failed");
    try {
        E::Manager::DrawParameter p; p.ViewProjectionMatrix = renderer->GetCameraProjectionMatrix();
        manager->Draw(p);
    } catch (...) { renderer->EndRendering(); throw; }
    require(renderer->EndRendering(), "EndRendering failed"); return 0;
}); }
API efk_last_error(char* buffer, int bytes) {
    if (!buffer || bytes < 1) return -1;
    strncpy_s(buffer, static_cast<size_t>(bytes), errorText, _TRUNCATE); return 0;
}
API efk_version() { return 1808; }

namespace {
double coordinate(double x) {
    require(std::isfinite(x) && std::abs(x)<=1000000.0, "Collision coordinates must be finite and within +/-1000000");
    return x;
}
collision::Collider& collider(int id) {
    auto it=collisionWorld.objects.find(id);
    require(it!=collisionWorld.objects.end(), "Invalid collider ID");
    return it->second;
}
int putCollider(int id, collision::Collider c) {
    if(id==0) {
        require(collisionWorld.objects.size()<256, "Maximum 256 colliders");
        id=newId();
        collisionWorld.objects.emplace(id,c);
    } else {
        auto& existing = collider(id);
        require(existing.kind==c.kind, "Collider type cannot be changed");
        c.enabled=existing.enabled; c.hits=existing.hits;
        existing=c;
    }
    return id;
}

}
API efk_collider_floor(int id, double height) { return withReady([&] {
    collision::Collider c; c.center.y=coordinate(height); return putCollider(id,c);
}); }
API efk_collider_sphere(int id,double x,double y,double z,double radius) { return withReady([&] {
    collision::Collider c; c.kind=collision::Kind::Sphere;
    c.center={coordinate(x),coordinate(y),coordinate(z)}; c.radius=coordinate(radius);
    require(c.radius>=0.0001,"Sphere radius must be >= 0.0001"); return putCollider(id,c);
}); }
API efk_collider_box(int id,double x,double y,double z,double sx,double sy,double sz) { return withReady([&] {
    collision::Collider c; c.kind=collision::Kind::Box;
    c.center={coordinate(x),coordinate(y),coordinate(z)};
    c.halfSize={coordinate(sx)*0.5,coordinate(sy)*0.5,coordinate(sz)*0.5};
    require(sx>=0.0001 && sy>=0.0001 && sz>=0.0001,"Box dimensions must be >= 0.0001"); return putCollider(id,c);
}); }
API efk_collider_enable(int id,int enabled) { return withReady([&] {
    collider(id).enabled=enabled!=0; return 0;
}); }
API efk_collider_remove(int id) { return withReady([&] {
    collider(id); collisionWorld.objects.erase(id); return 0;
}); }
API efk_collider_clear() { return withReady([] { collisionWorld.clear(); return 0; }); }
API efk_collider_hits(int id) { return withReady([&] { return collider(id).hits; }); }
API efk_collision_count() { return withReady([] { return collisionWorld.hits; }); }
API efk_collision_cast(double sx,double sy,double sz,double ex,double ey,double ez,double* output) { return withReady([&] {
    require(output!=nullptr,"Output must contain at least 7 doubles");
    collision::Vec start{coordinate(sx),coordinate(sy),coordinate(sz)},end{coordinate(ex),coordinate(ey),coordinate(ez)};
    auto hit=collisionWorld.cast(start,end);
    output[0]=hit.point.x; output[1]=hit.point.y; output[2]=hit.point.z;
    output[3]=hit.normal.x; output[4]=hit.normal.y; output[5]=hit.normal.z; output[6]=hit.id ? hit.t : 0;
    return hit.id;
}); }
API efk_collision_configure(int id,int enabled,double bounce,double friction) { return withReady([&] {
    auto resource = effect(id);
    float b=scalar(bounce),f=scalar(friction);
    require(b>=0 && b<=1 && f>=0 && f<=1,"Bounce and friction must be 0..1");
    for(const auto& instance:instances)
        require(!manager->Exists(instance.second) || manager->GetEffect(instance.second)!=resource,
                "Configure collision before playing the effect (stop active playbacks first)");
    // This intentionally uses a version-pinned internal node type: the public
    // API exposes the callback but does not expose collision node parameters.
    static_cast<E::EffectNodeImplemented*>(resource->GetRoot())->Traverse([&](E::EffectNodeImplemented* node) {
        node->Collisions.IsSceneCollisionWithExternal=enabled!=0;
        node->Collisions.IsGroundCollisionEnabled=false;
        node->Collisions.WorldCoordinateSyatem=E::WorldCoordinateSyatemType::Global;
        node->Collisions.Bounce={b,b}; node->Collisions.Friction={f,f};
        node->Collisions.LifetimeReductionPerCollision={0,0};
        return true;
    });
    return 0;
}); }
