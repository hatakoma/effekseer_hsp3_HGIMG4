#include "collision_world.h"
#include <Effekseer/Parameter/Effekseer.Collisions.h>
#include <iostream>
#include <stdexcept>
using namespace collision;
int checks=0;
void check(bool condition) { ++checks; if(!condition) throw std::runtime_error("Failed check "+std::to_string(checks)); }
void close(double a,double b) { if(std::abs(a-b)>=1e-6) std::cerr<<"actual="<<a<<" expected="<<b<<'\n'; check(std::abs(a-b)<1e-6); }
int main() { try {
    World w;
    w.objects[1]={Kind::Floor,{0,0,0}};
    auto h=w.cast({0,10,0},{0,-10,0}); check(h.id==1); close(h.t,0.5); close(h.normal.y,1);
    check(w.cast({0,-1,0},{0,1,0}).id==0);
    check(w.cast({0,1,0},{0,1,0}).id==0);
    check(w.cast({0,0,0},{0,-1,0}).id==1);
    check(w.cast({0,0,0},{0,1,0}).id==0);
    w.objects[1].enabled=false; check(w.cast({0,1,0},{0,-1,0}).id==0);
    w.clear(); w.objects[2]={Kind::Sphere,{0,0,0},{},1};
    h=w.cast({-10,0,0},{10,0,0}); check(h.id==2); close(h.point.x,-1); close(h.normal.x,-1); close(h.t,0.45);
    check(w.cast({-10,1,0},{10,1,0}).id==0);
    check(w.cast({-10,2,0},{10,2,0}).id==0);
    check(w.cast({0,0,0},{10,0,0}).id==0);
    check(w.cast({-1,0,0},{0,0,0}).id==2);
    check(w.cast({-1,0,0},{-2,0,0}).id==0);
    w.objects[3]={Kind::Box,{-4,0,0},{1,1,1}};
    h=w.cast({-10,0,0},{10,0,0}); check(h.id==3); close(h.point.x,-5); close(h.normal.x,-1);
    h=w.cast({-4,10,0},{-4,-10,0}); check(h.id==3); close(h.point.y,1); close(h.normal.y,1);
    h=w.cast({-4,0,-10},{-4,0,10}); check(h.id==3); close(h.point.z,-1); close(h.normal.z,-1);
    h=w.cast({-4,0,10},{-4,0,-10}); check(h.id==3); close(h.point.z,1); close(h.normal.z,1);
    check(w.cast({-4,0,0},{-4,10,0}).id==0);
    check(w.cast({-10,2,0},{-3,2,0}).id==0);
    check(w.cast({-4,1,0},{-4,0,0}).id==3);
    check(w.cast({-4,1,0},{-4,2,0}).id==0);
    // Nearest hit must be independent of insertion order, including fast sweeps.
    for(int i=1;i<=1000;++i) {
        double speed=10.0+i*100.0;
        h=w.cast({-speed,0,0},{speed,0,0}); check(h.id==3); close(h.point.x,-5);
    }
    w.record(3); w.record(3); w.record(2); check(w.hits==3); check(w.objects[3].hits==2);
    w.beginUpdate(); check(w.hits==0 && w.objects[3].hits==0);
    // Exercise the actual Effekseer reaction, not a duplicate reflection formula.
    w.clear(); w.objects[1]={Kind::Floor,{0,0,0}};
    Effekseer::CollisionsState state; state.Bounce=0.5f; state.Friction=0.25f;
    Effekseer::CollisionsParameter parameter; parameter.IsSceneCollisionWithExternal=true;
    auto result=Effekseer::CollisionsFunctions::Update(state,parameter,{2,-2,0},{0,1,0},{2,-3,0},{0,0,0},1,
        [&](const Effekseer::Vector3D& a,const Effekseer::Vector3D& b,Effekseer::Vector3D& p,Effekseer::Vector3D& n) {
            auto hit=w.cast({a.X,a.Y,a.Z},{b.X,b.Y,b.Z});
            if(!hit.id) return false;
            p={float(hit.point.x),float(hit.point.y),float(hit.point.z)};
            n={float(hit.normal.x),float(hit.normal.y),float(hit.normal.z)}; return true;
        });
    check(state.CollidedThisFrame);
    close(2+result.VelocityChange.GetX(),1.5);
    // Effekseer's SIMD normal normalization uses an approximate reciprocal sqrt.
    check(std::abs(-3+result.VelocityChange.GetY()-1.5)<0.005);
    close(result.PositionChange.GetY(),-1);
    std::cout<<"PASS: "<<checks<<" collision geometry and Effekseer bounce/friction checks\n";
    return 0;
} catch(const std::exception& e) { std::cerr<<e.what()<<'\n'; return 1; } }
