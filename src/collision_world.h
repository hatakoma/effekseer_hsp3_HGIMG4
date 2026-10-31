#pragma once
#include <algorithm>
#include <cmath>
#include <limits>
#include <map>

namespace collision {
struct Vec {
    double x=0, y=0, z=0;
    double operator[](int i) const { return i==0 ? x : i==1 ? y : z; }
    double& operator[](int i) { return i==0 ? x : i==1 ? y : z; }
    Vec operator+(Vec b) const { return {x+b.x,y+b.y,z+b.z}; }
    Vec operator-(Vec b) const { return {x-b.x,y-b.y,z-b.z}; }
    Vec operator*(double a) const { return {x*a,y*a,z*a}; }
    double dot(Vec b) const { return x*b.x+y*b.y+z*b.z; }
};
enum class Kind { Floor, Sphere, Box };
struct Collider {
    Kind kind=Kind::Floor;
    Vec center, halfSize;
    double radius=0;
    bool enabled=true;
    int hits=0;
};
struct Hit { int id=0; double t=1; Vec point,normal; };
class World {
public:
    std::map<int,Collider> objects;
    int hits=0;
    void beginUpdate() { hits=0; for(auto& item:objects) item.second.hits=0; }
    void record(int id) {
        if(hits<std::numeric_limits<int>::max()) ++hits;
        auto& count=objects.at(id).hits;
        if(count<std::numeric_limits<int>::max()) ++count;
    }
    void clear() { objects.clear(); hits=0; }
    // Swept POINT against static solids. Only entering contacts are reported;
    // particles starting strictly inside a solid are allowed to escape.
    Hit cast(Vec start, Vec end) const {
        constexpr double eps=1e-9;
        Vec delta=end-start;
        Hit best;
        if(delta.dot(delta)<eps*eps) return best;
        for(const auto& item:objects) {
            const auto& c=item.second;
            if(!c.enabled) continue;
            double t=-1;
            Vec normal;
            if(c.kind==Kind::Floor) {
                if(start.y<c.center.y-eps || delta.y>=-eps) continue;
                t=(c.center.y-start.y)/delta.y;
                normal={0,1,0};
            } else if(c.kind==Kind::Sphere) {
                Vec m=start-c.center;
                double a=delta.dot(delta), b=m.dot(delta), q=m.dot(m)-c.radius*c.radius;
                if(q < -eps || b>=0) continue;
                double discriminant=b*b-a*q;
                if(discriminant<=0) continue; // a tangent is not an entering hit
                // Stable near-root formula, avoiding cancellation near the surface.
                t=q/( -b+std::sqrt(discriminant));
                Vec n=start+delta*t-c.center;
                double length=std::sqrt(n.dot(n));
                if(length<=eps) continue;
                normal=n*(1.0/length);
            } else {
                Vec min=c.center-c.halfSize,max=c.center+c.halfSize;
                bool inside=true;
                for(int k=0;k<3;++k) inside=inside && start[k]>min[k]+eps && start[k]<max[k]-eps;
                if(inside) continue;
                double entry=-std::numeric_limits<double>::infinity();
                double leave=std::numeric_limits<double>::infinity();
                bool miss=false;
                for(int k=0;k<3;++k) {
                    if(std::abs(delta[k])<=eps) {
                        if(start[k]<min[k] || start[k]>max[k]) { miss=true; break; }
                        continue;
                    }
                    double nearT=(min[k]-start[k])/delta[k],farT=(max[k]-start[k])/delta[k];
                    double sign=-1;
                    if(nearT>farT) { std::swap(nearT,farT); sign=1; }
                    if(nearT>entry) { entry=nearT; normal={}; normal[k]=sign; }
                    leave=std::min(leave,farT);
                    if(entry>leave) { miss=true; break; }
                }
                if(miss || leave<0) continue;
                t=entry;
            }
            if(t < -eps || t>1+eps || delta.dot(normal)>=-eps) continue;
            t=std::clamp(t,0.0,1.0);
            if(best.id && t>=best.t-eps) continue;
            best={item.first,t,start+delta*t,normal};
        }
        return best;
    }
};
}
