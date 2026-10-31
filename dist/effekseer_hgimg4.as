#ifndef __effekseer_hgimg4__
#define __effekseer_hgimg4__
; Windows x86, HSP 3.7 ANSI, hgimg4.as (desktop OpenGL 3.1+).
; All commands return stat >= 0 on success, -1 on error.
#ifdef EFK_ANGLE
#uselib "effekseer_hgimg4dx.dll"
#else
#uselib "effekseer_hgimg4.dll"
#endif
#func global efk_init "efk_init" int
#func global efk_shutdown "efk_shutdown"
#func global efk_load "efk_load" str, double
#func global efk_release "efk_release" int
#func global efk_play "efk_play" int, double, double, double
#func global efk_stop "efk_stop" int
#func global efk_stop_all "efk_stop_all"
#func global efk_exists "efk_exists" int
#func global efk_location "efk_location" int, double, double, double
#func global efk_rotation "efk_rotation" int, double, double, double
#func global efk_scale "efk_scale" int, double, double, double
#func global efk_speed "efk_speed" int, double
#func global efk_pause "efk_pause" int, int
#func global efk_color "efk_color" int, int, int, int, int
#func global efk_update "efk_update" double
#func global efk_camera "efk_camera" double, double, double, double, double, double
#func global efk_perspective "efk_perspective" double, double, double, double
#func global efk_matrices "efk_matrices" var, var
#func global efk_draw "efk_draw"
#func global efk_last_error "efk_last_error" var, int
#func global efk_version "efk_version"
#func global efk_collider_floor "efk_collider_floor" int,double
#func global efk_collider_sphere "efk_collider_sphere" int,double,double,double,double
#func global efk_collider_box "efk_collider_box" int,double,double,double,double,double,double
#func global efk_collider_enable "efk_collider_enable" int,int
#func global efk_collider_remove "efk_collider_remove" int
#func global efk_collider_clear "efk_collider_clear"
#func global efk_collider_hits "efk_collider_hits" int
#func global efk_collision_count "efk_collision_count"
#func global efk_collision_cast "efk_collision_cast" double,double,double,double,double,double,var
#func global efk_collision_configure "efk_collision_configure" int,int,double,double
#endif
