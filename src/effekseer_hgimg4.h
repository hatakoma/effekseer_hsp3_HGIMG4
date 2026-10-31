#pragma once
// Windows x86 C ABI. Link with the CMake-generated import library.
#ifdef __cplusplus
extern "C" {
#endif
int __stdcall efk_init(int capacity);
int __stdcall efk_shutdown(void);
int __stdcall efk_load(const char* ansi_path, double magnification);
int __stdcall efk_release(int effect_id);
int __stdcall efk_play(int effect_id, double x, double y, double z);
int __stdcall efk_stop(int playback_id);
int __stdcall efk_stop_all(void);
int __stdcall efk_exists(int playback_id);
int __stdcall efk_location(int playback_id, double x, double y, double z);
int __stdcall efk_rotation(int playback_id, double x, double y, double z);
int __stdcall efk_scale(int playback_id, double x, double y, double z);
int __stdcall efk_speed(int playback_id, double speed);
int __stdcall efk_pause(int playback_id, int paused);
int __stdcall efk_color(int playback_id, int r, int g, int b, int a);
int __stdcall efk_update(double frames);
int __stdcall efk_camera(double x, double y, double z, double tx, double ty, double tz);
int __stdcall efk_perspective(double vertical_fov_degrees, double aspect, double near_z, double far_z);
int __stdcall efk_matrices(const double* view16, const double* projection16);
int __stdcall efk_draw(void);
int __stdcall efk_last_error(char* buffer, int buffer_bytes);
int __stdcall efk_version(void);
int __stdcall efk_collider_floor(int id, double height);
int __stdcall efk_collider_sphere(int id, double x, double y, double z, double radius);
int __stdcall efk_collider_box(int id, double x, double y, double z, double sx, double sy, double sz);
int __stdcall efk_collider_enable(int id, int enabled);
int __stdcall efk_collider_remove(int id);
int __stdcall efk_collider_clear(void);
int __stdcall efk_collider_hits(int id);
int __stdcall efk_collision_count(void);
int __stdcall efk_collision_cast(double sx, double sy, double sz, double ex, double ey, double ez, double* output7);
int __stdcall efk_collision_configure(int effect, int enabled, double bounce, double friction);
#ifdef __cplusplus
}
#endif
