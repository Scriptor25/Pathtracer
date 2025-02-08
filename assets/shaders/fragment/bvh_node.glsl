#version 450 core
#include "common.incl"

layout (binding = 3, std430) readonly buffer BVHBuffer {
    BVHNode nodes[];
};

bool BVHNode_Hit(in uint index, in Ray ray, in float ray_t_min, in float ray_t_max) {

    BVHNode self = nodes[index];

    vec3 origin = ray.origin;
    vec3 direction = ray.direction;

    for (uint a = 0u; a < 3; ++a) {
        float min = self.min[a];
        float max = self.max[a];
        float invd = 1.0 / direction[a];

        float t0 = (min - origin[a]) * invd;
        float t1 = (max - origin[a]) * invd;

        if (t0 < t1) {
            if (t0 > ray_t_min) ray_t_min = t0;
            if (t1 < ray_t_max) ray_t_max = t1;
        }
        else {
            if (t1 > ray_t_min) ray_t_min = t1;
            if (t0 < ray_t_max) ray_t_max = t0;
        }

        if (ray_t_max <= ray_t_min) return false;
    }

    return true;
}
