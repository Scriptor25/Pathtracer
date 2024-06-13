#version 450 core
#include "common.incl"

bool Scene_Hit(in Scene self, in Ray ray, in Interval ray_t, inout Record rec) {
    Record tmp_rec;
    bool hit = false;

    for (int i = 0; i < self.sphere_count; ++i) {
        if (Sphere_Hit(self.spheres[i], ray, ray_t, tmp_rec)) {
            hit = true;
            ray_t.max = tmp_rec.t;
            rec = tmp_rec;
        }
    }

    for (int i = 0; i < self.triangle_count; ++i) {
        if (Triangle_Hit(self.triangles[i], ray, ray_t, tmp_rec)) {
            hit = true;
            ray_t.max = tmp_rec.t;
            rec = tmp_rec;
        }
    }

    return hit;
}
