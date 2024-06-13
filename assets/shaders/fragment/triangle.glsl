#version 450 core
#include "common.incl"

bool Triangle_Hit(in Triangle self, in Ray ray, in Interval ray_t, inout Record rec) {

    vec3 edge1 = self.p1 - self.p0;
    vec3 edge2 = self.p2 - self.p0;
    vec3 cross_dir_e2 = cross(ray.direction, edge2);
    float det = dot(edge1, cross_dir_e2);

    if (abs(det) < 1e-8) {
        return false;
    }

    float inv_det = 1.0 / det;
    vec3 s = ray.origin - self.p0;
    float u = inv_det * dot(s, cross_dir_e2);

    if (u < 0.0 || u > 1.0) {
        return false;
    }

    vec3 cross_s_e1 = cross(s, edge1);
    float v = inv_det * dot(ray.direction, cross_s_e1);

    if (v < 0.0 || (u + v) > 1.0) {
        return false;
    }

    float t = inv_det * dot(edge2, cross_s_e1);

    if (!Interval_Contains(ray_t, t)) {
        return false;
    }

    rec.t = t;
    rec.p = Ray_At(ray, t);
    vec3 outward_normal = cross(normalize(edge1), normalize(edge2));
    Record_SetNormal(rec, ray, outward_normal);
    rec.material = self.material;

    return true;
}
