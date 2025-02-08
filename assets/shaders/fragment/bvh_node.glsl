#version 450 core

#include "common.incl"

layout (binding = 3, std430) readonly buffer BVHBuffer {
    BVHNode nodes[];
};

bool BVHNode_Hit(in uint index, in Ray ray, in Interval ray_t) {

    BVHNode self = nodes[index];

    vec3 origin = ray.origin;
    vec3 direction = ray.direction;

    bool ok = true;
    for (uint a = 0u; a < 3u && ok; ++a) {
        float a_min = self.min[a];
        float a_max = self.max[a];

        float o = origin[a];
        float d_inv = 1.0 / direction[a];

        float t0 = (a_min - o) * d_inv;
        float t1 = (a_max - o) * d_inv;

        if (t0 >= t1) {
            float t = t0;
            t0 = t1;
            t1 = t;
        }

        ray_t.min = max(ray_t.min, t0);
        ray_t.max = min(ray_t.max, t1);

        ok = ray_t.max > ray_t.min;
    }

    return ok;
}
