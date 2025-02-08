#version 450 core

#include "common.incl"

layout (binding = 3, std430) readonly buffer BVHBuffer {
    BVHNode nodes[];
};

bool BVHNode_Hit(in uint index, in Ray ray, in Interval ray_t) {

    vec3 origin = ray.origin;
    vec3 direction = ray.direction;

    for (uint a = 0u; a < 3; ++a) {
        float min = nodes[index].min[a];
        float max = nodes[index].max[a];
        float invd = 1.0 / direction[a];

        float t0 = (min - origin[a]) * invd;
        float t1 = (max - origin[a]) * invd;

        if (t0 < t1) {
            if (t0 > ray_t.min) ray_t.min = t0;
            if (t1 < ray_t.max) ray_t.max = t1;
        } else {
            if (t1 > ray_t.min) ray_t.min = t1;
            if (t0 < ray_t.max) ray_t.max = t0;
        }

        if (ray_t.max <= ray_t.min) {
            return false;
        }
    }

    return true;
}
