#version 450 core

#include "common.incl"

layout (binding = 0, std430) readonly buffer TriangleBuffer {
    Triangle triangles[];
};

bool Triangle_Hit(in uint index, in Ray ray, in Interval ray_t, inout Record rec) {

    Triangle self = triangles[index];

    vec3 edge1 = self.p1 - self.p0;
    vec3 edge2 = self.p2 - self.p0;
    vec3 cross_dir_e2 = cross(ray.direction, edge2);
    float det = dot(edge1, cross_dir_e2);

    if (abs(det) < 1e-5) {
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

    if (!Interval_Surrounds(ray_t, t)) {
        return false;
    }

    float w = 1.0 - u - v;

    rec.t = t;
    rec.p = Ray_At(ray, t);
    rec.uv = self.uv0 * w + self.uv1 * u + self.uv2 * v;
    vec3 outward_normal = self.n0 * w + self.n1 * u + self.n2 * v;
    Record_SetNormal(rec, ray, outward_normal);
    rec.material = self.material;

    return true;
}
